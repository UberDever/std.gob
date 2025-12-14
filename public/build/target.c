#include "impl.h"
#include "std.gob/internal/gob/api.h"
#include "std.gob/public/build/api.h"
#include "std.gob/third_party/nob.h/nob.h"

bool gob_build_target_is_built(const build_target_t* t) {
  int result = nob_file_exists(t->name);
  return result == 1;
}

bool gob_build_target_run(const build_target_t* t) {
  if (gob_build_target_is_built(t)) { return true; }
  for (size_t i = 0; i < t->deps.count; ++i) {
    if (!gob_build_target_run(&t->deps.items[i])) { return false; }
  }
  return t->rule(t);
}

void gob_build_target_free_rec(const build_target_t* t) {
  for (size_t i = 0; i < t->deps.count; ++i) {
    gob_build_target_free_rec(&t->deps.items[i]);
  }
  nob_da_free(t->ins);
  nob_da_free(t->deps);
  nob_da_free(t->build_args);
}

bool gob_build_rule_compile(const build_target_t* t) {
  Nob_Cmd cmd = {0};
  const char* cc = gob_build_get_args(t->build_args, "cc");
  assert(cc);
  nob_cmd_append(&cmd, cc);

  char* cflags = gob_build_get_args(t->build_args, "cflags");
  assert(cflags);
  for (char* p = strtok(cflags, " "); p != NULL; p = strtok(NULL, " ")) {
    nob_cmd_append(&cmd, p);
  }

  ASSERT_LOG(t->ins.count == 1, "%zu", t->ins.count);
  nob_cmd_append(&cmd, "-c");
  const char* in = t->ins.items[0];
  nob_cmd_append(&cmd, in);

  nob_cmd_append(&cmd, "-o");
  assert(t->name);
  nob_cmd_append(&cmd, t->name);

  bool result = false;
  if (!nob_cmd_run(&cmd)) { goto defer; }
  result = true;

defer:
  nob_cmd_free(cmd);
  return result;
}

bool gob_build_rule_link_exe(const build_target_t* t) {
  Nob_Cmd cmd = {0};
  const char* cc = gob_build_get_args(t->build_args, "cc");
  assert(cc);
  nob_cmd_append(&cmd, cc);

  assert(t->ins.items);
  for (size_t i = 0; i < t->ins.count; ++i) {
    assert(t->ins.items[i]);
    nob_cmd_append(&cmd, t->ins.items[i]);
  }

  nob_cmd_append(&cmd, "-o");
  assert(t->name);
  nob_cmd_append(&cmd, t->name);

  bool result = false;
  if (!nob_cmd_run(&cmd)) { goto defer; }
  result = true;

defer:
  nob_cmd_free(cmd);
  return result;
}
