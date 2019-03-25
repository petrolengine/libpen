#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

char *pen_strip(char *str)
    PEN_NONNULL(1)
    PEN_RETURNS_NONNULL
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

#ifdef __cplusplus
}
#endif
