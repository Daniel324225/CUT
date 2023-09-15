#include <assert.h>
#include <threads.h>

#include "lock.h"

static int read_thread(void* lock_v) {
    Lock* lock = lock_v;

    bool success = Lock_for_read(lock, 500);
    assert(success);
    Lock_unlock(lock);

    success = Lock_for_read(lock, 500);
    assert(success);
    Lock_unlock(lock);

    return 0;
}

int main(void) {
    Lock lock;
    Lock_init(&lock);

    bool success;
    success = Lock_for_read(&lock, 10);
    assert(!success);

    success = Lock_for_write(&lock, 10);
    assert(success);

    Lock_unlock(&lock);

    success = Lock_for_write(&lock, 10);
    assert(!success);
    
    success = Lock_for_read(&lock, 10);
    assert(success);

    Lock_unlock(&lock);


    thrd_t thread;
    thrd_create(&thread, read_thread, &lock);

    success = Lock_for_write(&lock, 100);
    Lock_unlock(&lock);
    success = Lock_for_write(&lock, 100);
    Lock_unlock(&lock);

    thrd_join(thread, NULL);
    
    Lock_destroy(&lock);
}