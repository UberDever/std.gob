// clang-format off
/// NOB_BUILD_LINUX: cc -D_POSIX_C_SOURCE=200809L --std=c99 -Wall -Wextra -I.. -o nob nob.c internal/headeronly/nob.c internal/nob/rebuild_urself.c
// clang-format on

#include "std.gob/third_party/nob.h/nob.h"
#include "std.gob/internal/nob/api.h"

int main(int argc, char** argv) {
  gob_rebuild_from_directives(argc, argv, __FILE__);

  return 0;
}
