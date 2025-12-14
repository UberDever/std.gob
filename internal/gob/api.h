#ifndef __INTERNAL_NOB_API_H__
#define __INTERNAL_NOB_API_H__

#include "std.gob/third_party/nob.h/nob.h"
#include <stddef.h>

#define gob_log(level, fmt, ...) gob_log_impl(__FILE__, __LINE__, level, fmt, ##__VA_ARGS__)

void gob_log_impl(const char* filepath, size_t line, size_t level, const char* fmt, ...)
    NOB_PRINTF_FORMAT(4, 5);

#endif // __INTERNAL_NOB_API_H__
