/*
{
NOB_BUILD_LINUX: cc -fsanitize=address -D_POSIX_C_SOURCE=200809L --std=c99 -Wall -Wextra
-I.. -o nob nob.c internal/headeronly/nob.c internal/headeronly/stb_ds.c internal/headeronly/flag.c
internal/headeronly/arena.c internal/gob/gob_log.c internal/util.string/string.c
public/gob/rebuild_urself.c public/command/route.c
}
*/

#include "std.gob/third_party/nob.h/nob.h"
#include "std.gob/internal/gob/api.h"
#include "std.gob/public/command/api.h"
#include "std.gob/public/gob/api.h"
#include "std.gob/third_party/arena-allocator/arena.h"
#include "std.gob/third_party/flag.h/flag.h"
#include "std.gob/third_party/stb_ds/stb_ds.h"

char g_project_path[1024] = {0};

#ifdef _WIN32
#define FS_SEP "\\"
#else
#define FS_SEP "/"
#endif

#define ARENA_SIZE (10 * 1024 * 1024)

int main(int argc, char** argv) {
  int ret = 0;
  char* got_cwd = getcwd(g_project_path, NOB_ARRAY_LEN(g_project_path));
  assert(got_cwd);

  gob_rebuild_from_directives(argc, argv, __FILE__);

  bool defer1 = false, defer2 = false;

  Arena* arena = arena_create(ARENA_SIZE);
  if (arena == NULL) {
    gob_log(NOB_ERROR, "failed to create memory arena");
    ret = 1;
    goto errdefer;
  }
  defer1 = true;

  gob_command_route_t* routes = NULL;
  if (!gob_command_fill_routes(arena, argc, argv, &routes)) {
    gob_log(NOB_ERROR, "failed to fill command routes");
    ret = 1;
    goto errdefer;
  }
  defer2 = true;

  ret = gob_command_run(arena, argc, argv, routes);

errdefer:
  if (defer1) { arena_destroy(arena); }
  if (defer2) { stbds_shfree(routes); }
  return ret;
}
