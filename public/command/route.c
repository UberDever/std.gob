#include "api.h"
#include "std.gob/internal/gob/api.h"
#include "std.gob/internal/util.string/api.h"
#include "std.gob/third_party/arena-allocator/arena.h"
#include "std.gob/third_party/flag.h/flag.h"
#include "std.gob/third_party/nob.h/nob.h"
#include "std.gob/third_party/stb_ds/stb_ds.h"
#include <stdbool.h>

static void command_put_route(
    gob_command_route_t** routes,
    const char* key,
    int (*handler)(Arena*, const gob_command_desc_t* desc, int, char**)) {
  stbds_shput(
      *routes,
      key,
      ((gob_command_desc_t){
          .routes = NULL,
          .handler = handler,
      }));
}

static int command_help(Arena* arena, const gob_command_desc_t* desc, int argc, char** argv) {
  if (argc > 1) {
    const char* topic = nob_shift(argv, argc);
    nob_log(NOB_ERROR, "no help topic for '%s'", topic);
    return false;
  }
  fprintf(stderr, "    The commands are:\n");
  for (size_t i = 0; i < stbds_shlen(desc->routes); ++i) {
    const gob_command_route_t* route = &desc->routes[i];
    Nob_String_View route_key = nob_sv_from_cstr(route->key);
    nob_sv_chop_by_delim(&route_key, ' ');
    if (route_key.count != 0) { continue; }
    fprintf(stderr, "        %s\n", route->key);
  }

  fprintf(stderr, "    Additional help topics:\n");
  for (size_t i = 0; i < stbds_shlen(desc->routes); ++i) {
    const gob_command_route_t* route = &desc->routes[i];
    Nob_String_View route_key = nob_sv_from_cstr(route->key);
    nob_sv_chop_by_delim(&route_key, ' ');
    if (route_key.count == 0) { continue; }
    fprintf(stderr, "        %s\n", route->key);
  }
  return true;
}

static int command_build(Arena* arena, const gob_command_desc_t* desc, int argc, char** argv) {
  char* const* build_dir = flag_str("dir", "build", "Build directory");
  char* const* flag_target = flag_str("target", NULL, "The target we need to execute command for");

  if (argc == 1) {
    flag_print_options(stderr);
    return false;
  }

  if (!flag_parse(argc, argv)) {
    flag_print_error(stderr);
    return false;
  }
  return true;
}

static int command_help_build(Arena* arena, const gob_command_desc_t* desc, int argc, char** argv) {
  if (argc > 1) {
    const char* topic = nob_shift(argv, argc);
    nob_log(NOB_ERROR, "no help topic for '%s'", topic);
    return false;
  }

  return true;
}

bool gob_command_fill_routes(Arena* arena, int argc, char** argv, gob_command_route_t** out) {
  gob_command_route_t* routes = NULL;

  command_put_route(&routes, "help", command_help);
  command_put_route(&routes, "help build", command_help_build);

  command_put_route(&routes, "build", command_build);

  for (size_t i = 0; i < stbds_shlen(routes); ++i) {
    routes[i].value.routes = routes;
  }

  *out = routes;
  return true;
}

int gob_command_run(Arena* arena, int argc, char** argv, gob_command_route_t* routes) {
  const char* program = nob_shift(argv, argc);
  char* help_argv = "help";
  if (argc == 0) {
    argc = 1;
    argv = &help_argv;
  }
  opt_str_t command = str_from_cstr(arena, nob_shift(argv, argc));
  while (argc > 0) {
    const char* arg = argv[0];
    if (arg != NULL && arg[0] == '-') { break; }
    opt_str_t next =
        str_concat_cstr(arena, str_to_cstr(arena, command.value), " ", nob_shift(argv, argc));
    command = next;
  }
  assert(command.has_value);

  const char* command_cstr = str_to_cstr(arena, command.value);
  ptrdiff_t slot = stbds_shgeti(routes, command_cstr);
  if (slot < 0) {
    gob_log(NOB_ERROR, "no such command '%s'", command_cstr);
    return 1;
  }

  // flag_parse expects argv[0] to be the program name, but the dispatcher stripped it off.
  int adjusted_argc = argc + 1;
  char** adjusted_argv = arena_alloc(arena, sizeof(char*) * adjusted_argc);
  ASSERT_LOG(adjusted_argv != NULL, "failed to allocate argv for build command");
  adjusted_argv[0] = "nob";
  for (int i = 0; i < argc; ++i) {
    adjusted_argv[i + 1] = argv[i];
  }

  gob_command_desc_t desc = routes[slot].value;

  return desc.handler(arena, &desc, adjusted_argc, adjusted_argv);
}
