#ifndef __PUBLIC_BUILD_IMPL_H__
#define __PUBLIC_BUILD_IMPL_H__

#include "api.h"

#include <stddef.h>

struct build_target_t;

typedef struct {
  struct build_target_t* items;
  size_t count;
  size_t capacity;
} build_targets_t;

typedef struct {
  const char** items;
  size_t count;
  size_t capacity;
} build_target_inputs_t;

typedef struct build_target_t {
  const char* name;
  build_target_inputs_t ins;
  build_targets_t deps;
  gob_build_args_t build_args;
  bool (*rule)(const struct build_target_t*);
} build_target_t;

typedef bool (*build_rule_t)(const struct build_target_t*);

#endif
