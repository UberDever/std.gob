#ifndef __INTERNAL_UTIL_STRING_STRING_API_H__
#define __INTERNAL_UTIL_STRING_STRING_API_H__

#include "std.gob/internal/util.optional/api.h"
#include "std.gob/third_party/arena-allocator/arena.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// Null-terminated utf-8 string stored in arena
typedef struct {
  size_t off;
  size_t len;
} str_t;

DEFINE_OPTIONAL(str_t);

static inline const char* str_to_cstr(Arena* arena, str_t str) {
  return (const char*)(arena->region + str.off);
}

static inline char* str_to_cstr_mut(Arena* arena, str_t str) {
  return (char*)(arena->region + str.off);
}

opt_str_t str_from_cstr(Arena* arena, const char* cstr);
opt_str_t str_concat_v(Arena* arena, ...);
#define str_concat(arena, ...) str_concat_v(arena, __VA_ARGS__, opt_none(str_t))
opt_str_t str_concat_cstr_v(Arena* arena, ...);
#define str_concat_cstr(arena, ...) str_concat_cstr_v(arena, __VA_ARGS__, NULL)
bool str_append(Arena* arena, str_t str, bool insert_null);
bool str_append_cstr(Arena* arena, const char* cstr, bool insert_null);

#endif
