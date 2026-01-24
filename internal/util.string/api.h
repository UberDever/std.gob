#ifndef __INTERNAL_UTIL_STRING_STRING_API_H__
#define __INTERNAL_UTIL_STRING_STRING_API_H__

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifndef USE_DEFAULT_ARENA_T
#define USE_DEFAULT_ARENA_T
#include "std.gob/third_party/arena-allocator/arena.h"
#endif

#ifndef USE_DEFUALT_OPTIONAL_T
#define USE_DEFUALT_OPTIONAL_T
#include "std.gob/internal/util.optional/api.h"
#endif

// Null-terminated utf-8 string stored in arena
typedef struct {
  size_t off;
  size_t len;
} str_t;

DEFINE_OPTIONAL(str_t);

#define str_to_cstr(arena, str)     ((const char*)(arena->region + (str).off))
#define str_to_cstr_mut(arena, str) ((char*)(arena->region + (str).off))

opt_str_t str_from_cstr(Arena* arena, const char* cstr);
opt_str_t str_concat_v(Arena* arena, ...);
#define str_concat(arena, ...) str_concat_v(arena, __VA_ARGS__, opt_none(str_t))
opt_str_t str_concat_cstr_v(Arena* arena, ...);
#define str_concat_cstr(arena, ...) str_concat_cstr_v(arena, __VA_ARGS__, NULL)
bool str_append(Arena* arena, str_t str, bool insert_null);
bool str_append_cstr(Arena* arena, const char* cstr, bool insert_null);

#endif
