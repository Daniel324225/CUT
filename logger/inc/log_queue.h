#include <stddef.h>
#include <threads.h>
#include <stdbool.h>

typedef struct LogQueueNode {
    union {
        const char* msg;
        char* dynamic_msg;
    };
    bool is_dynamic;
} LogQueueNode;

typedef struct LogQueue {
    LogQueueNode* nodes;
    size_t max_size;
    size_t current;
    size_t size;
    bool closed;
    mtx_t mutex;
    cnd_t not_empty;
    cnd_t not_full;
} LogQueue;

typedef enum LogQueueResult {
    LOG_QUEUE_SUCCESS,
    LOG_QUEUE_CLOSED,
    LOG_QUEUE_TIMEOUT,
} LogQueueResult;

void LogQueue_init(LogQueue*, size_t max_size);
void LogQueue_destroy(LogQueue*);
void LogQueue_close(LogQueue*);
LogQueueResult LogQueue_pop(LogQueue*, LogQueueNode*, long long timeout_ms);
LogQueueResult LogQueue_push(LogQueue*, LogQueueNode, long long timeout_ms);
