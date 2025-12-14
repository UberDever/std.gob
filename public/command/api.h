#ifndef __PUBLIC_COMMAND_API_H__
#define __PUBLIC_COMMAND_API_H__

#include "std.gob/third_party/arena-allocator/arena.h"
#include <stdbool.h>

typedef struct gob_command_route_t gob_command_route_t;
typedef struct gob_command_desc_t gob_command_desc_t;

struct gob_command_desc_t {
  const gob_command_route_t* routes;
  int (*handler)(Arena* arena, const gob_command_desc_t* desc, int argc, char** argv);
};

struct gob_command_route_t {
  char* key;
  gob_command_desc_t value;
};

bool gob_command_fill_routes(Arena* arena, int argc, char** argv, gob_command_route_t** out);
int gob_command_run(Arena* arena, int argc, char** argv, gob_command_route_t* routes);

#endif
