#pragma once

#include <stddef.h>

typedef struct Watchdog Watchdog;

typedef struct WatchdogInterface {
    Watchdog* wd;
    size_t thread_idx;
} WatchdogInterface;

void Watchdog_notify_active(WatchdogInterface);
void Watchdog_notify_finished(WatchdogInterface);
