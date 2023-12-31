#pragma once

#include <stdio.h>
#include "log_queue.h"
#include "watchdog_interface.h"

typedef struct {
    FILE* log_file;
    LogQueue log_queue;
    WatchdogInterface wdi;
} Logger;

void Logger_init(Logger*, const char* out_file, size_t queue_size, WatchdogInterface wdi);
void Logger_destroy(Logger*);
bool Logger_log(Logger*, const char *);
bool Logger_log_dynamic(Logger*, char *);
__attribute__((__format__ (__printf__, 2, 3)))
bool Logger_log_formatted(Logger*, const char* format, ...);
int Logger_run(void*);
void Logger_stop(Logger*);
