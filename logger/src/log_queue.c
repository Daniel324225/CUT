#include <stdlib.h>
#include <stdio.h>

#include "utility.h"
#include "log_queue.h"

void LogQueue_init(LogQueue* log_queue, size_t max_size) {
    *log_queue = (LogQueue){
        .max_size = max_size,
        .nodes = malloc(sizeof(LogQueueNode) * max_size),
    };
    mtx_init(&log_queue->mutex, mtx_plain | mtx_timed);
    cnd_init(&log_queue->not_empty);
    cnd_init(&log_queue->not_full);
}

void LogQueue_destroy(LogQueue* log_queue) {
    free(log_queue->nodes);
    mtx_destroy(&log_queue->mutex);
    cnd_destroy(&log_queue->not_empty);
    cnd_destroy(&log_queue->not_full);
}

void LogQueue_close(LogQueue* log_queue) {
    mtx_lock(&log_queue->mutex);
    log_queue->closed = true;
    mtx_unlock(&log_queue->mutex);
    cnd_broadcast(&log_queue->not_empty);
    cnd_broadcast(&log_queue->not_full);
}

static bool closed_or_not_empty(void* log_queue_v);

static bool closed_or_not_full(void* log_queue_v) {
    LogQueue* log_queue = log_queue_v;
    return log_queue->closed || log_queue->size != log_queue->max_size;
}

static bool closed_or_not_empty(void* log_queue_v) {
    LogQueue* log_queue = log_queue_v;
    return log_queue->closed || log_queue->size != 0;
}

LogQueueResult LogQueue_pop(LogQueue* log_queue, LogQueueNode* out_node, long long timeout_ms) {
    mtx_lock(&log_queue->mutex);

    bool ok = cnd_wait_until(
        &log_queue->not_empty,
        &log_queue->mutex,
        timeout_ms,
        closed_or_not_empty,
        log_queue
    );

    bool notify = false;
    LogQueueResult result;
    if (!ok) {
        result = LOG_QUEUE_TIMEOUT;
    }
    else if (log_queue->size != 0) {
        notify = true;
        *out_node = log_queue->nodes[log_queue->current];
        log_queue->current = (log_queue->current + 1) % log_queue->max_size;
        --log_queue->size;
        result = LOG_QUEUE_SUCCESS;
    }
    else {
        result = LOG_QUEUE_CLOSED;
    }

    mtx_unlock(&log_queue->mutex);
    if (notify) {
        cnd_signal(&log_queue->not_full);
    }
    return result;
}

LogQueueResult LogQueue_push(LogQueue* log_queue, LogQueueNode node, long long timeout_ms) {
    mtx_lock(&log_queue->mutex);

    bool ok = cnd_wait_until(
        &log_queue->not_full,
        &log_queue->mutex,
        timeout_ms,
        closed_or_not_full,
        log_queue
    );

    bool notify = false;
    LogQueueResult result;
    if (!ok) {
        result = LOG_QUEUE_TIMEOUT;
    }
    else if (log_queue->closed) {
        result = LOG_QUEUE_CLOSED;
    }
    else {
        notify = true;
        size_t last = (log_queue->current + log_queue->size) % log_queue->max_size;
        log_queue->nodes[last] = node;
        ++log_queue->size;
        result = LOG_QUEUE_SUCCESS;
    }

    mtx_unlock(&log_queue->mutex);
    if (notify) {
        cnd_signal(&log_queue->not_empty);
    }
    return result;
}
