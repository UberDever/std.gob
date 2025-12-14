#ifndef __PUBLIC_BUILD_API_H__
#define __PUBLIC_BUILD_API_H__

#include "std.gob/internal/util.string/api.h"
#include <assert.h>

typedef struct {
  const char* key;
  char* value;
} gob_build_arg_t;

gob_build_arg_t gob_build_new_arg(Arena* arena, const char* key, const char* value);

typedef struct {
  gob_build_arg_t* items;
  size_t count;
  size_t capacity;
} gob_build_args_t;

char* gob_build_get_args(gob_build_args_t args, const char* key);

struct build_target_t;
bool gob_build_target_is_built(const struct build_target_t* t);
bool gob_build_target_run(const struct build_target_t* t);
void gob_build_target_free_rec(const struct build_target_t* t);

bool gob_build_rule_compile(const struct build_target_t* t);
bool gob_build_rule_link_exe(const struct build_target_t* t);

#endif
