#include <stdio.h>
#include <stdlib.h>

#include "reader.h"

static size_t get_core_count(void);
static void ignore_line(FILE*);
static void get_cpu_stats(CpuStat[], size_t core_count);

int Reader_run(void* reader_v) {
    Reader* reader = reader_v;
    Logger* logger = reader->logger;

    const size_t core_count = get_core_count();

    Logger_log_formatted(logger, "Reader: Found %zu cores", core_count);

    Logger_log(logger, "Reader: Waiting for first write");
    while(!Lock_for_write(reader->output_lock, 100)) {
        if (*(reader->done)) {
            Watchdog_notify_finished(reader->wdi);
            return 0;
        }
        Watchdog_notify_active(reader->wdi);
    }

    reader->output->core_count = core_count;
    reader->output->cpu_stats = malloc(sizeof(CpuStat) * core_count);
    get_cpu_stats(reader->output->cpu_stats, core_count);

    Lock_unlock(reader->output_lock);
    Logger_log(logger, "Reader: First write done");

    while (!*(reader->done)) {
        Watchdog_notify_active(reader->wdi);
        Logger_log(logger, "Reader: Waiting for write");
        if (Lock_for_write(reader->output_lock, 100)) {
            get_cpu_stats(reader->output->cpu_stats, core_count);
            Lock_unlock(reader->output_lock);
            Logger_log(logger, "Reader: Write done");
        }
    }

    Logger_log(logger, "Reader: Terminating");

    Lock_forced(reader->output_lock);
    reader->output->core_count = 0;
    free(reader->output->cpu_stats);
    Lock_unlock_forced(reader->output_lock);

    Watchdog_notify_finished(reader->wdi);
    return 0;
}

static void ignore_line(FILE* file) {
    int c;
    do {
        c = fgetc(file);
    } while (c != '\n' && c != EOF);
}

static size_t get_core_count(void) {
    FILE* file = fopen("/proc/stat", "r");
    size_t core_count = 0;
    int core_idx;
    int count;
    for(;;) {
        int res = fscanf(file, "cpu %n%d", &count, &core_idx);
        if (res == EOF) {
            break;
        }
        if (res == 1 && count == 3) {
            ++core_count;
        }
        ignore_line(file);
    }
    fclose(file);
    return core_count;
}

static void get_cpu_stats(CpuStat cpu_stats[], size_t core_count) {
    FILE* file = fopen("/proc/stat", "r");
    size_t core_idx = 0;
    int count;
    for(;;) {
        int res = fscanf(file, "cpu %n%zu", &count, &core_idx);
        if (res == EOF) {
            break;
        }
        if (res == 1 && count == 3 && core_idx < core_count) {
            fscanf(file, " %zu %zu %zu %zu %zu %zu %zu %zu",
                &cpu_stats[core_idx].user,
                &cpu_stats[core_idx].nice,
                &cpu_stats[core_idx].system,
                &cpu_stats[core_idx].idle,
                &cpu_stats[core_idx].io_wait,
                &cpu_stats[core_idx].irq,
                &cpu_stats[core_idx].soft_irq,
                &cpu_stats[core_idx].steal
            );
        }
        ignore_line(file);
    }
    fclose(file);
}
