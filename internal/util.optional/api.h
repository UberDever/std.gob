#ifndef __INTERNAL_UTIL_OPTIONAL_OPTIONAL_API_H__
#define __INTERNAL_UTIL_OPTIONAL_OPTIONAL_API_H__

#define DEFINE_OPTIONAL(T)                                                                         \
  typedef struct {                                                                                 \
    int has_value;                                                                                 \
    T value;                                                                                       \
  } opt_##T

#define opt_none(T)    ((opt_##T){.has_value = 0})
#define opt_some(T, v) ((opt_##T){.has_value = 1, .value = (v)})

#endif
