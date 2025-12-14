#include "std.gob/internal/gob/api.h"
#include "std.gob/third_party/nob.h/nob.h"

void gob_log_impl(const char* filepath, size_t line, size_t level, const char* fmt, ...) {
  const char* level_str = "UNKNOWN";
  switch (level) {
    case NOB_ERROR: level_str = "ERROR"; break;
    case NOB_WARNING: level_str = "WARNING"; break;
    case NOB_INFO: level_str = "INFO"; break;
    default: break;
  }

  fprintf(stderr, "[%s] %s:%zu ", level_str, filepath, line);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
