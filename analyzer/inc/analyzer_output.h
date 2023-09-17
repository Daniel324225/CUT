#pragma once

#include <stddef.h>

typedef struct AnalyzerOutput {
    size_t core_count;
    float* core_usage;
} AnalyzerOutput;
