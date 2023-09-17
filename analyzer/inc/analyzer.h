#pragma once

#include <signal.h>

#include "analyzer_output.h"
#include "reader_output.h"
#include "logger.h"
#include "lock.h"
#include "watchdog_interface.h"

typedef struct Analyzer {
    WatchdogInterface wdi;
    Logger* logger;
    const volatile sig_atomic_t* done;
    
    const ReaderOutput* input;
    Lock* input_lock;
    AnalyzerOutput* output;
    Lock* output_lock;
} Analyzer;

int Analyzer_run(void* analyzer_v);
