#pragma once

#include "platform.h"
#include "../include/pen_connector.h"

#ifdef __cplusplus
extern "C" {
#endif

void pen_connector_init(PenEvent_t *ev)
    PEN_NONNULL(1)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

void pen_connector_destroy(void)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

#ifdef __cplusplus
}
#endif
