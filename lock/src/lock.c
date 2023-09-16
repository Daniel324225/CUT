#include <stdlib.h>

#include "utility.h"
#include "lock.h"

static bool ready_for_write(void* lock) {
    return !((Lock*)lock)->value_updated;
}

static bool ready_for_read(void* lock) {
    return ((Lock*)lock)->value_updated;
}


void Lock_init(Lock* lock) {
    lock->value_updated = false;
    mtx_init(&lock->mutex, mtx_plain);
    cnd_init(&lock->cv_updated);
    cnd_init(&lock->cv_read);
}

bool Lock_for_write(Lock* lock, long long timeout_ms) {
    mtx_lock(&lock->mutex);

    bool success = cnd_wait_until(&lock->cv_read, &lock->mutex, timeout_ms, ready_for_write, lock);
    if (!success) {
        mtx_unlock(&lock->mutex);
    }

    return success;
}


bool Lock_for_read(Lock* lock, long long timeout_ms) {
    mtx_lock(&lock->mutex);

    bool success = cnd_wait_until(&lock->cv_updated, &lock->mutex, timeout_ms, ready_for_read, lock);
    if (!success) {
        mtx_unlock(&lock->mutex);
    }

    return success;
}

void Lock_forced(Lock* lock) {
    mtx_lock(&lock->mutex);
}


void Lock_unlock(Lock* lock) {
    bool value_updated = lock->value_updated = !lock->value_updated;
    mtx_unlock(&lock->mutex);
    if (value_updated) {
        cnd_signal(&lock->cv_updated);
    } else {
        cnd_signal(&lock->cv_updated);
    }
}

void Lock_unlock_forced(Lock* lock) {
    bool value_updated = lock->value_updated;
    mtx_unlock(&lock->mutex);
    if (value_updated) {
        cnd_signal(&lock->cv_updated);
    } else {
        cnd_signal(&lock->cv_updated);
    }
}


void Lock_destroy(Lock* lock) {
    mtx_destroy(&lock->mutex);
    cnd_destroy(&lock->cv_updated);
    cnd_destroy(&lock->cv_read);
}
