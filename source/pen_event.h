#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PEN_EVENT_READ  = 0x01,
    PEN_EVENT_WRITE = 0x02,
#if HAVE_SYS_EVENT_H
    PEN_EVENT_TIMER = 0x08,
    PEN_EVENT_FILE_MONITOR = 0x10,
#endif
};

#define PEN_EVENT_RDWR (PEN_EVENT_READ | PEN_EVENT_WRITE)

enum {
    PEN_EVENT_STATE_NONE  = 0,
    PEN_EVENT_STATE_READ  = 0x01,
    PEN_EVENT_STATE_WRITE = 0x02,
    PEN_EVENT_STATE_CLOSE = 0x08,
#if HAVE_SYS_EVENT_H
    PEN_EVENT_STATE_TIMER = 0x10,
#endif
};

PenEvent_t *pen_event_init(size_t buffer_size)
    PEN_WARN_UNUSED_RESULT
    PEN_RETURNS_NONNULL
    PEN_NOTHROW;

#if HAVE_SYS_EVENT_H

bool pen_event_timer_add(PenEvent_t *self,
                         PenEventBase_t *ctx,
                         struct timespec *tm)
    PEN_WARN_UNUSED_RESULT
    PEN_NONNULL(1,2,3)
    PEN_NOTHROW;

PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_set_timeout_ev(PenEventBase_t *ctx)
{
    ctx->state_ |= PEN_EVENT_STATE_TIMER;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_clear_timeout_ev(PenEventBase_t *ctx)
{
    ctx->state_ &= PEN_EVENT_STATE_TIMER;
    return ctx;
}

#endif

bool pen_event_add(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
    PEN_WARN_UNUSED_RESULT
    PEN_NONNULL(1,3)
    PEN_NOTHROW;

bool pen_event_mod(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
    PEN_WARN_UNUSED_RESULT
    PEN_NONNULL(1)
    PEN_NOTHROW;

bool pen_event_del(PenEvent_t *self, PenEventBase_t *ctx)
    PEN_WARN_UNUSED_RESULT
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_event_start(PenEvent_t *self)
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_event_stop(PenEvent_t *self)
    PEN_NONNULL(1)
    PEN_NOTHROW;

void pen_event_destroy(PenEvent_t *self)
    PEN_NONNULL(1)
    PEN_NOTHROW;

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_set_read(PenEventBase_t *ctx)
{
    ctx->state_ |= PEN_EVENT_STATE_READ;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_set_write(PenEventBase_t *ctx)
{
    ctx->state_ |= PEN_EVENT_STATE_WRITE;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_clear_read(PenEventBase_t *ctx)
{
    ctx->state_ &= ~PEN_EVENT_STATE_READ;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_clear_write(PenEventBase_t *ctx)
{
    ctx->state_ &= ~PEN_EVENT_STATE_WRITE;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_set_close(PenEventBase_t *ctx)
{
    ctx->state_ |= PEN_EVENT_STATE_CLOSE;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_RETURNS_NONNULL
static inline PenEventBase_t *
pen_event_clear_close(PenEventBase_t *ctx)
{
    ctx->state_ &= ~PEN_EVENT_STATE_CLOSE;
    return ctx;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_WARN_UNUSED_RESULT
static inline bool
pen_event_is_readable(PenEventBase_t *ctx)
{
    return ctx->state_ & PEN_EVENT_STATE_READ;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_WARN_UNUSED_RESULT
static inline bool
pen_event_is_writeable(PenEventBase_t *ctx)
{
    return ctx->state_ & PEN_EVENT_STATE_WRITE;
}

PEN_NONNULL(1)
PEN_NOTHROW
PEN_WARN_UNUSED_RESULT
static inline bool
pen_event_is_closed(PenEventBase_t *ctx)
{
    return ctx->state_ & PEN_EVENT_STATE_CLOSE;
}

#ifdef __cplusplus
}
#endif
