#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

PenContextFactory_t *pen_context_factory_init(
        size_t init_size,
        size_t ctx_size,
        size_t ctx_align)
    PEN_VISIBILITY_INTERNAL
    PEN_RETURNS_NONNULL
    PEN_WARN_UNUSED_RESULT
    PEN_NOTHROW;
void *pen_context_factory_get(PenContextFactory_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_RETURNS_NONNULL
    PEN_WARN_UNUSED_RESULT
    PEN_NONNULL(1)
    PEN_NOTHROW;
void pen_context_factory_put(PenContextFactory_t *self, void *ctx)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1,2)
    PEN_NOTHROW;
void *pen_context_factory_pop(PenContextFactory_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;
void pen_context_factory_destroy(PenContextFactory_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;
size_t pen_context_factory_size(PenContextFactory_t *self)
    PEN_VISIBILITY_INTERNAL
    PEN_NONNULL(1)
    PEN_NOTHROW;

#define PEN_CONTEXT_FACTORY_INIT(sz, tp) \
        pen_context_factory_init(sz, sizeof(tp), alignof(tp))

#ifdef __cplusplus
}
#endif
