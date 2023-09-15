#include "utility.h"

bool cnd_wait_until(cnd_t* cv, mtx_t* mutex, long long timeout_ms, bool(*condition)(void*), void* arg) {
    struct timespec time_point;
    timespec_get(&time_point, TIME_UTC);
    const long long ns_in_ms = 1000 * 1000;
    const long long ns_in_s = 1000 * 1000 * 1000;
    long long ns = time_point.tv_nsec + (timeout_ms % 1000) * ns_in_ms;
    time_point.tv_sec += ns / ns_in_s;
    time_point.tv_nsec = ns % ns_in_s;

    while(!condition(arg)) {
        if (cnd_timedwait(cv, mutex, &time_point) == thrd_timedout) {
            return condition(arg);
        }
    }
    return true;
}
