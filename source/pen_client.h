#pragma once

#include "platform.h"
#include "../include/pen_client.h"

#ifdef __cplusplus
extern "C" {
#endif

void pen_client_init(PenEvent_t *ev)
    PEN_NOTHROW
    PEN_NONNULL(1)
    PEN_VISIBILITY_INTERNAL;

void pen_client_destroy(void)
    PEN_NOTHROW
    PEN_VISIBILITY_INTERNAL;

void pen_client_add(int fd, uint8_t servertype)
    PEN_NOTHROW
    PEN_VISIBILITY_INTERNAL;


#ifdef __cplusplus
}
#endif

