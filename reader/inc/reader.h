#pragma once

#include "reader_output.h"
#include "lock.h"
#include "watchdog_interface.h"
#include "logger.h"
#include "signal.h"

typedef struct Reader {
    WatchdogInterface wdi;
    Logger* logger;
    const volatile sig_atomic_t* done;
    ReaderOutput* output;
    Lock* output_lock;
} Reader;

int Reader_run(void* reader_v);
