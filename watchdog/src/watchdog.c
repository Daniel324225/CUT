#include <stdlib.h>

#include "watchdog_interface.h"
#include "watchdog.h"

void Watchdog_init(Watchdog* wd, volatile sig_atomic_t* done, size_t size, const char* names[]) {
    *wd = (Watchdog) {
        .size = size,
        .threads = malloc(sizeof(WatchdogThreadInfo) * size),
        .done = done,
    };

    for (size_t i = 0; i < size; ++i) {
        wd->threads[i] = (WatchdogThreadInfo) {
            .status = WATCHDOG_THREAD_INACTIVE,
            .name = names[i],
        };
    }

    mtx_init(&wd->mutex, mtx_plain);
    cnd_init(&wd->cv);
}

void Watchdog_destroy(Watchdog* wd) {
    free(wd->threads);
    mtx_destroy(&wd->mutex);
}

int Watchdog_run(void* watchdog_v) {
    Watchdog* wd = watchdog_v;
    size_t finished;
    do {
        thrd_sleep(&(struct timespec){.tv_sec=2}, NULL);
        mtx_lock(&wd->mutex);
        finished = 0;
        for (size_t i = 0; i < wd->size; ++i) {
            if (wd->threads[i].status == WATCHDOG_THREAD_INACTIVE) {
                *wd->done = 1;
                wd->stopped = true;
                mtx_unlock(&wd->mutex);
                cnd_broadcast(&wd->cv);
                return 0;
            }
        }

        for (size_t i = 0; i < wd->size; ++i) {
            if (wd->threads[i].status == WATCHDOG_THREAD_ACTIVE) {
                wd->threads[i].status = WATCHDOG_THREAD_INACTIVE;
            } else {
                ++finished;
            }
        }
        if (finished == wd->size) {
            wd->stopped = true;
        }
        mtx_unlock(&wd->mutex);
    } while (finished != wd->size);
    cnd_broadcast(&wd->cv);

    return 0;
}

void Watchdog_wait_for_all(Watchdog* wd) {
    mtx_lock(&wd->mutex);
    
    while(!wd->stopped) {
        cnd_wait(&wd->cv, &wd->mutex);
    }

    mtx_unlock(&wd->mutex);
}

void Watchdog_notify_active(WatchdogInterface wdi) {
    mtx_lock(&wdi.wd->mutex);

    if (!wdi.wd->stopped) {
        wdi.wd->threads[wdi.thread_idx].status = WATCHDOG_THREAD_ACTIVE;
    }

    mtx_unlock(&wdi.wd->mutex);
}

void Watchdog_notify_finished(WatchdogInterface wdi) {
    mtx_lock(&wdi.wd->mutex);

    if (!wdi.wd->stopped) {
        wdi.wd->threads[wdi.thread_idx].status = WATCHDOG_THREAD_FINISHED;
    }

    mtx_unlock(&wdi.wd->mutex);
}
