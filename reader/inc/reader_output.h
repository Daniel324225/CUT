#pragma once

#include <stddef.h>

typedef struct CpuStat {
    size_t user;
    size_t nice;
    size_t system;
    size_t idle;
    size_t io_wait;
    size_t irq;
    size_t soft_irq;
    size_t steal;
} CpuStat;

typedef struct ReaderOutput {
    size_t core_count;
    CpuStat* cpu_stats;
} ReaderOutput;
