#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

PenBuffer_t *pen_buffer_get(size_t init_size)
    PEN_VISIBILITY_INTERNAL
    PEN_WARN_UNUSED_RESULT
    PEN_NOTHROW
    PEN_RETURNS_NONNULL;

size_t pen_buffer_read(PenBuffer_t *self, void *buf, const size_t sz)
    PEN_VISIBILITY_INTERNAL
    PEN_WARN_UNUSED_RESULT
    PEN_NOTHROW
    PEN_NONNULL(1,2);

size_t pen_buffer_write(PenBuffer_t *self, const void *data, const size_t sz)
    PEN_VISIBILITY_INTERNAL
    PEN_WARN_UNUSED_RESULT
    PEN_NOTHROW
    PEN_NONNULL(1,2);

void pen_buffer_free(PenBuffer_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW
    PEN_NONNULL(1);

void pen_buffer_print(PenBuffer_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW
    PEN_NONNULL(1);

#ifdef __cplusplus
}
#endif
