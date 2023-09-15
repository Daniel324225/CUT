#include <stdlib.h>
#include <stdio.h>

#include "logger.h"

void Logger_init(Logger* logger, const char* out_file, size_t queue_size, WatchdogInterface wdi) {
    logger->log_file = fopen(out_file, "w");
    logger->wdi = wdi;
    LogQueue_init(&logger->log_queue, queue_size);
}

void Logger_destroy(Logger* logger) {
    fclose(logger->log_file);
    LogQueue_destroy(&logger->log_queue);
}

bool Logger_log(Logger* logger, const char* msg) {
    return LogQueue_push(&logger->log_queue, (LogQueueNode){.msg = msg, false}, 100) == LOG_QUEUE_SUCCESS;
}

bool Logger_log_dynamic(Logger* logger, char* msg) {
    return LogQueue_push(&logger->log_queue, (LogQueueNode){.dynamic_msg = msg, true}, 100) == LOG_QUEUE_SUCCESS;
}

int Logger_run(void* logger_v) {
    Logger* logger = logger_v;
    LogQueueNode node;

    while(1) {
        Watchdog_notify_active(logger->wdi);
        LogQueueResult result = LogQueue_pop(&logger->log_queue, &node, 500);
        fflush(stdout);
        if (result == LOG_QUEUE_CLOSED) {
            break;
        } else if (result == LOG_QUEUE_TIMEOUT) {
            continue;
        }
        fprintf(logger->log_file, "%s\n", node.msg);
        fflush(logger->log_file);
        if (node.is_dynamic) {
            free(node.dynamic_msg);
        }
    }

    Watchdog_notify_finished(logger->wdi);
    return 0;
}

void Logger_stop(Logger* logger) {
    LogQueue_close(&logger->log_queue);
}
