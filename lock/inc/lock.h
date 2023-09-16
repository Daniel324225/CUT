#pragma once

#include <threads.h>
#include <stdbool.h>

typedef struct Lock {
    bool value_updated;
    mtx_t mutex;
    cnd_t cv_updated;
    cnd_t cv_read;
} Lock;

void Lock_init(Lock*);
bool Lock_for_write(Lock*, long long timeout_ms);
bool Lock_for_read(Lock*, long long timeout_ms);
void Lock_forced(Lock*);
void Lock_unlock_forced(Lock*);
void Lock_unlock(Lock*);
void Lock_destroy(Lock*);
