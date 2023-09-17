#include <signal.h>
#include <stdio.h>
#include "watchdog.h"
#include "logger.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"

static volatile sig_atomic_t done = 0;

static void signal_handler(int signum) {
    (void)signum;
    done = 1;
}

int main(void) {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    const size_t threads = 4;
    Watchdog wd;
    Watchdog_init(&wd, &done, threads, (const char*[]){"Logger", "Reader", "Analyzer", "Printer"});
    
    Logger logger;
    Logger_init(&logger, "log.txt", 10, (WatchdogInterface){&wd, 0});

    ReaderOutput reader_out;
    Lock reader_out_lock;
    Lock_init(&reader_out_lock);
    Reader reader = {
        .wdi = {&wd, 1},
        .logger = &logger,
        .done = &done,
        .output = &reader_out,
        .output_lock = &reader_out_lock
    };

    AnalyzerOutput analyzer_out;
    Lock analyzer_out_lock;
    Lock_init(&analyzer_out_lock);
    Analyzer analyzer = {
        .wdi = {&wd, 2},
        .logger = &logger,
        .done = &done,
        .input = &reader_out,
        .input_lock = &reader_out_lock,
        .output = &analyzer_out,
        .output_lock = &analyzer_out_lock
    };

    Printer printer = {
        .wdi = {&wd, 3},
        .logger = &logger,
        .done = &done,
        .input = &analyzer_out,
        .input_lock = &analyzer_out_lock
    };

    thrd_t logger_thread, wd_thread, reader_thread, analyzer_thread, printer_thread;
    thrd_create(&logger_thread, Logger_run, &logger);
    thrd_create(&reader_thread, Reader_run, &reader);
    thrd_create(&analyzer_thread, Analyzer_run, &analyzer);
    thrd_create(&printer_thread, Printer_run, &printer);
    thrd_create(&wd_thread, Watchdog_run, &wd);

    while(!done) {
        thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    }

    Logger_log(&logger, "Terminating");

    Logger_stop(&logger);
    Watchdog_wait_for_all(&wd);

    bool ok = true;
    for (size_t i = 0; i < threads; ++i) {
        if (wd.threads[i].status == WATCHDOG_THREAD_INACTIVE) {
            printf("%s thread not responding\n", wd.threads[i].name);
            ok = false;
        }
    }
    if (!ok) {
        return 1;
    }

    thrd_join(reader_thread, NULL);
    thrd_join(analyzer_thread, NULL);
    thrd_join(printer_thread, NULL);
    thrd_join(logger_thread, NULL);
    thrd_join(wd_thread, NULL);
    Logger_destroy(&logger);
    Watchdog_destroy(&wd);
    Lock_destroy(&reader_out_lock);
    Lock_destroy(&analyzer_out_lock);

    return 0;
}
