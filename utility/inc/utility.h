#pragma once

#include <stdbool.h>
#include <threads.h>

bool cnd_wait_until(cnd_t* cv, mtx_t* mutex, long long timeout_ms, bool(*condition)(void*), void* arg);
