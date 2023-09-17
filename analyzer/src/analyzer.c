#include <stdlib.h>

#include "analyzer.h"

typedef struct {
    size_t non_idle;
    size_t total;
} Aggregate;

static void aggregate_stats(const CpuStat* stats, Aggregate* aggregates, size_t size);
static void compute_usage(const Aggregate* old, const Aggregate* new, float* usages, size_t size);
static void swap(Aggregate** a, Aggregate**b) {
    Aggregate* tmp = *a;
    *a = *b;
    *b = tmp;
}
static size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

int Analyzer_run(void* analyzer_v) {
    Analyzer* const analyzer = analyzer_v;
    Logger* const logger = analyzer->logger;
    const ReaderOutput* const input = analyzer->input;
    Lock* const input_lock = analyzer->input_lock;
    AnalyzerOutput* const output = analyzer->output;
    Lock* const output_lock = analyzer->output_lock;
    const volatile sig_atomic_t* const done = analyzer->done;

    Logger_log(logger, "Analyzer: Waiting for first read");
    
    while (!Lock_for_read(input_lock, 100)) {
        if (*done) {
            Watchdog_notify_finished(analyzer->wdi);
            return 0;
        }
        Watchdog_notify_active(analyzer->wdi);
    }

    const size_t core_count = input->core_count;
    Aggregate* old = calloc(core_count, sizeof(Aggregate));
    Aggregate* new = malloc(core_count * sizeof(Aggregate));
    aggregate_stats(input->cpu_stats, new, core_count);

    Lock_unlock(input_lock);

    Logger_log(logger, "Analyzer: First read done");

    Logger_log(logger, "Analyzer: Waiting for first write");

    while(!Lock_for_write(output_lock, 100)) {
        if (*done) {
            free(old);
            free(new);
            Watchdog_notify_finished(analyzer->wdi);
            return 0;
        }
        Watchdog_notify_active(analyzer->wdi);
    }

    output->core_usage = malloc(core_count * sizeof(float));
    output->core_count = core_count;
    compute_usage(old, new, output->core_usage, core_count);

    Lock_unlock(output_lock);
    swap(&old, &new);
    Logger_log(logger, "Analyzer: First write done");

    while (!*done) {
        Watchdog_notify_active(analyzer->wdi);
        
        Logger_log(logger, "Analyzer: Trying to read");
        if (Lock_for_read(input_lock, 500)) {
            aggregate_stats(
                input->cpu_stats, 
                new, 
                min(core_count, input->core_count));
            Lock_unlock(analyzer->input_lock);
            Logger_log(logger, "Analyzer: Read done");
        }
        Logger_log(logger, "Analyzer: Trying to write");
        if (Lock_for_write(output_lock, 500)) {
            compute_usage(old, new, output->core_usage, core_count);
            Lock_unlock(analyzer->output_lock);
            swap(&old, &new);
            Logger_log(logger, "Analyzer: Write done");
        }
    }

    Logger_log(logger, "Analyzer: Terminating");

    free(old);
    free(new);
    Lock_forced(output_lock);
    output->core_count = 0;
    free(output->core_usage);
    Lock_unlock_forced(output_lock);

    Watchdog_notify_finished(analyzer->wdi);
    return 0;
}

static void aggregate_stats(const CpuStat* stats, Aggregate* aggregates, size_t size) {
    for (size_t i = 0; i != size; ++i) {
        aggregates[i].non_idle = 
            stats[i].user +
            stats[i].nice +
            stats[i].system +
            stats[i].irq +
            stats[i].soft_irq +
            stats[i].steal;
        aggregates[i].total = aggregates[i].non_idle + stats[i].idle + stats[i].io_wait;
    }
}

static void compute_usage(const Aggregate* old, const Aggregate* new, float* usages, size_t size) {
    for (size_t i = 0; i != size; ++i) {
        size_t non_idle_diff = new[i].non_idle - old[i].non_idle;
        size_t total_diff = new[i].total - old[i].total;
        usages[i] = total_diff != 0 ? ((float)non_idle_diff / (float)total_diff) : 0;
    }
}
