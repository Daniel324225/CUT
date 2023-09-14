#pragma once

#include <stddef.h>
#include <threads.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <signal.h>

typedef enum WatchdogThreadStatus {
    WATCHDOG_THREAD_ACTIVE,
    WATCHDOG_THREAD_INACTIVE,
    WATCHDOG_THREAD_FINISHED,
} WatchdogThreadStatus;

typedef struct WatchdogThreadInfo {
    WatchdogThreadStatus status;
    const char* name;
} WatchdogThreadInfo;

typedef struct Watchdog {
    size_t size;
    WatchdogThreadInfo* threads;
    volatile sig_atomic_t* done;
    bool stopped;
    mtx_t mutex;
    cnd_t cv;
} Watchdog;

void Watchdog_init(Watchdog* wd, volatile sig_atomic_t* done, size_t size, const char* names[]);
void Watchdog_destroy(Watchdog*);
void Watchdog_wait_for_all(Watchdog*);
int Watchdog_run(void* watchdog);
