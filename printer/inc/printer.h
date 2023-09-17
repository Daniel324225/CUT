#pragma once

#include <signal.h>

#include "analyzer_output.h"
#include "logger.h"
#include "lock.h"
#include "watchdog_interface.h"

typedef struct Printer {
    WatchdogInterface wdi;
    Logger* logger;
    const volatile sig_atomic_t* done;
    
    const AnalyzerOutput* input;
    Lock* input_lock;
} Printer;

int Printer_run(void* printer_v);
