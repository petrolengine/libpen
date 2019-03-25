#pragma once

#include "platform.h"


#ifdef __cplusplus
extern "C" {
#endif

void pen_listener_init(PenEvent_t *ev)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_listener_destroy(void)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

#ifdef __cplusplus
}
#endif
