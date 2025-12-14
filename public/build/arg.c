
#include "std.gob/public/build/api.h"
#include "std.gob/third_party/arena-allocator/arena.h"

gob_build_arg_t gob_build_new_arg(Arena* arena, const char* key, const char* value) {
  opt_str_t k_opt = str_concat_cstr(arena, key);
  assert(k_opt.has_value);
  const char* k = str_to_cstr(arena, k_opt.value);
  opt_str_t v_opt = str_concat_cstr(arena, value);
  assert(v_opt.has_value);
  char* v = str_to_cstr_mut(arena, v_opt.value);
  return (gob_build_arg_t){k, v};
}

char* gob_build_get_args(gob_build_args_t args, const char* key) {
  if (!args.items) { return NULL; }
  assert(key);
  for (size_t i = 0; i < args.count; ++i) {
    if (0 == strcmp(args.items[i].key, key)) { return args.items[i].value; }
  }
  return NULL;
}
