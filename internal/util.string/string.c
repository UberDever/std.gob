#include "api.h"
#include "std.gob/third_party/arena-allocator/arena.h"
#include <stdarg.h>
#include <string.h>

opt_str_t str_from_cstr(Arena* arena, const char* cstr) {
  size_t len = strlen(cstr);
  size_t off = arena->index;
  const char* p = arena_alloc_aligned(arena, len + 1, 1);
  if (!p) { return opt_none(str_t); }
  memcpy(arena->region + off, cstr, len + 1);
  return opt_some(str_t, ((str_t){.off = off, .len = len}));
}

opt_str_t str_concat_v(Arena* arena, ...) {
  size_t len = 0;
  va_list args;
  va_start(args, arena);
  while (1) {
    opt_str_t arg = va_arg(args, opt_str_t);
    if (!arg.has_value) { break; }
    len += arg.value.len;
  }
  va_end(args);
  if (len == 0) { return opt_none(str_t); }

  size_t off = arena->index;
  char* result = arena_alloc_aligned(arena, len + 1, 1);
  if (!result) { return opt_none(str_t); }
  result[0] = '\0';

  va_start(args, arena);
  while (1) {
    opt_str_t arg = va_arg(args, opt_str_t);
    if (!arg.has_value) { break; }
    strcat(result, str_to_cstr(arena, arg.value));
  }
  va_end(args);
  return opt_some(str_t, ((str_t){.off = off, .len = len}));
}

opt_str_t str_concat_cstr_v(Arena* arena, ...) {
  size_t len = 0;
  va_list args;
  va_start(args, arena);
  while (1) {
    const char* arg = va_arg(args, const char*);
    if (arg == NULL) { break; }
    len += strlen(arg);
  }
  va_end(args);
  if (len == 0) { return opt_none(str_t); }

  size_t off = arena->index;
  char* result = arena_alloc_aligned(arena, len + 1, 1);
  if (!result) { return opt_none(str_t); }
  result[0] = '\0';

  va_start(args, arena);
  while (1) {
    const char* arg = va_arg(args, const char*);
    if (arg == NULL) { break; }
    strcat(result, arg);
  }
  va_end(args);
  return opt_some(str_t, ((str_t){.off = off, .len = len}));
}

bool str_append(Arena* arena, str_t str, bool insert_null) {
  size_t off = arena->index;
  size_t len = str.len + (insert_null ? 1 : 0);
  char* p = arena_alloc_aligned(arena, len, 1);
  if (!p) { return false; }
  memcpy(arena->region + off, str_to_cstr(arena, str), str.len);
  if (insert_null) { p[len] = '\0'; }
  return true;
}

bool str_append_cstr(Arena* arena, const char* cstr, bool insert_null) {
  if (!cstr) { return false; }
  size_t off = arena->index;
  size_t len = strlen(cstr);
  char* p = arena_alloc_aligned(arena, len + (insert_null ? 1 : 0), 1);
  if (!p) { return false; }
  memcpy(arena->region + off, cstr, len);
  if (insert_null) { p[len] = '\0'; }
  return true;
}
