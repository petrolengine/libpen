#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

PenMqueue_t *pen_mqueue_init(const char *name, size_t init_size)
    PEN_RETURNS_NONNULL
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW
    PEN_WARN_UNUSED_RESULT;

void *pen_mqueue_get(PenMqueue_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW
    PEN_WARN_UNUSED_RESULT;

void pen_mqueue_put(PenMqueue_t *self, void *data)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1, 2)
    PEN_NOTHROW;

void pen_mqueue_exit(PenMqueue_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_mqueue_destroy(PenMqueue_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_mqueue_print(PenMqueue_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;

#ifdef __cplusplus
}
#endif
