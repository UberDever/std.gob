#ifndef __PUBLIC_GOB_API_H__
#define __PUBLIC_GOB_API_H__

#include <stddef.h>

#ifndef USE_DEFAULT_NOB
#define USE_DEFAULT_NOB
#include "std.gob/third_party/nob.h/nob.h"
#endif

#define GD_ASSERT_LOG(cond, fmt, ...)                                                              \
  if (!(cond)) {                                                                                   \
    gob_log(NOB_ERROR, fmt, ##__VA_ARGS__);                                                        \
    NOB_UNREACHABLE("assertion failed");                                                           \
  }

#define gob_log(level, fmt, ...) gob_log_impl(__FILE__, __LINE__, level, fmt, ##__VA_ARGS__)

void gob_log_impl(const char* filepath, size_t line, size_t level, const char* fmt, ...)
    NOB_PRINTF_FORMAT(4, 5);

void gob_rebuild_from_directives(int argc, char** argv, const char* source_path);

#endif
