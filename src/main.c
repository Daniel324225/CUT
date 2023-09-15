#include <signal.h>
#include <stdio.h>
#include "watchdog.h"
#include "logger.h"

static volatile sig_atomic_t done = 0;

static void signal_handler(int signum) {
    (void)signum;
    done = 1;
}

int main(void) {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    Logger logger;
    Watchdog wd;

    const size_t threads = 1;

    Watchdog_init(&wd, &done, threads, (const char*[]){"Logger"});
    Logger_init(&logger, "log.txt", 10, (WatchdogInterface){&wd, 0});

    thrd_t logger_thread, wd_thread;
    thrd_create(&logger_thread, Logger_run, &logger);
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

    thrd_join(logger_thread, NULL);
    thrd_join(wd_thread, NULL);
    Logger_destroy(&logger);
    Watchdog_destroy(&wd);

    return 0;
}
