#include "pen_event.h"

#if HAVE_SYS_EVENT_H

#include <errno.h>
#include <sys/event.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#define PEN_KQUEUE_DEBUG 1

typedef struct kevent KEvent_t;

struct PenEvent {
    int epfd_;
    int buf_size_;
    KEvent_t *event_buf_;
    bool eXit_;
};

PenEvent_t *
pen_event_init(size_t buffer_size)
{
    size_t total_sz = sizeof(PenEvent_t) + sizeof(KEvent_t) * buffer_size
                        + alignof(KEvent_t);
    PenEvent_t *self = calloc(1, total_sz);
    self->epfd_ = kqueue();
    self->buf_size_ = (int)buffer_size;
    self->event_buf_ = (KEvent_t*) pen_align_ptr(
                            (char*)self + sizeof(PenEvent_t),
                            alignof(PenEvent_t));
    return self;
}

static inline bool
__set_event(
            PenEvent_t *self,
            PenEventBase_t *ctx,
            int16_t filter,
            uint16_t flags,
            uint32_t fflags,
            intptr_t data)
{
    KEvent_t ev;

    EV_SET(&ev, ctx->fd_, filter, flags, fflags, data, ctx);
    return (kevent(self->epfd_, &ev, 1, NULL, 0, NULL) >= 0);
}

bool
pen_event_timer_add(PenEvent_t *self, PenEventBase_t *ctx, struct timespec *tm)
{
    return __set_event(self, ctx, EVFILT_TIMER, EV_ADD,
                       NOTE_ABSOLUTE, (intptr_t)tm);
}

bool
pen_event_add(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
{
    uint16_t flags = EV_ADD | EV_CLEAR;
    uint32_t fflags = 0;
    bool ret = true;

    ctx->events_ = event;

    if (event & PEN_EVENT_READ)
        ret &= __set_event(self, ctx, EVFILT_READ, flags, 0, 0);
    if (event & PEN_EVENT_WRITE)
        ret &= __set_event(self, ctx, EVFILT_WRITE, flags, 0, 0);
    if (event & PEN_EVENT_FILE_MONITOR) {
        fflags = NOTE_DELETE |  NOTE_WRITE | NOTE_EXTEND
                | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;
        ret &= __set_event(self, ctx, EVFILT_VNODE, flags, fflags, 0);
    }

    return ret;
}

bool
pen_event_mod(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
{
    bool ret = false;

    if (event & PEN_EVENT_READ && !(ctx->events_ & PEN_EVENT_READ))
        ret = __set_event(self, ctx, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0);

    if (!(event & PEN_EVENT_READ) && (ctx->events_ & PEN_EVENT_READ))
        ret = __set_event(self, ctx, EVFILT_READ, EV_DELETE, 0, 0);

    if (event & PEN_EVENT_WRITE && !(ctx->events_ & PEN_EVENT_WRITE))
        ret = __set_event(self, ctx, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0);

    if (!(event & PEN_EVENT_WRITE) && (ctx->events_ & PEN_EVENT_WRITE))
        ret = __set_event(self, ctx, EVFILT_WRITE, EV_DELETE, 0, 0);

    ctx->events_ = event;

    return ret;
}

bool
pen_event_del(PenEvent_t *self, PenEventBase_t *ctx)
{
    if (ctx->events_ & PEN_EVENT_READ)
        return __set_event(self, ctx, EVFILT_READ, EV_DELETE, 0, 0);
    if (ctx->events_ & PEN_EVENT_WRITE)
        return __set_event(self, ctx, EVFILT_WRITE, EV_DELETE, 0, 0);
    if (ctx->events_ & PEN_EVENT_FILE_MONITOR)
        return __set_event(self, ctx, EVFILT_VNODE, EV_DELETE, 0, 0);
    return false;
}

static inline void
__dispatch_event(KEvent_t *ev, PenEventBase_t *ctx)
{
    if (ctx->cb_ == NULL) {
        PEN_LOG_WARN("unsupport event. flags:\n"
                     "%hu, filter: %hd, fflags: %u, data: %d\n"
                     "fd: %d\n",
                     ev->flags, ev->filter, ev->fflags, (int)ev->data, ctx->fd_);
        return ;
    }
#if PEN_KQUEUE_DEBUG
    PEN_LOG_DEBUG("flags: %hu, filter: %hd, fflags: %u, data: %d\n",
                  ev->flags, ev->filter, ev->fflags, (int)ev->data);
#endif
    if (ev->flags & EV_ERROR) {
        pen_event_set_close(ctx);
#if PEN_KQUEUE_DEBUG
        PEN_LOG_ERROR("fd close with error %d: %s\n",
                      (int)ev->data, strerror((int)ev->data));
#endif
    } else if (ev->filter == EVFILT_READ)
        pen_event_set_read(ctx);
    else if (ev->filter == EVFILT_WRITE)
        pen_event_set_write(ctx);
    else if (ev->filter == EVFILT_TIMER)
        pen_event_set_timeout_ev(ctx);
    else if (ev->filter == EVFILT_VNODE) {
        if (ev->fflags & NOTE_DELETE)
            pen_event_set_close(ctx);
        if (ev->fflags & NOTE_WRITE)
            pen_event_set_read(ctx);
    }
    else
        return;

    ctx->data_size_ = (int)ev->data;

    // TODO add signal here!!!
    (*ctx->cb_)(ctx);
}

void
pen_event_start(PenEvent_t *self)
{
    int ret = 0, i;
    KEvent_t *ptr = NULL;
    PenEventBase_t *ctx = NULL;

    while (!self->eXit_) {
        ret = kevent(self->epfd_, NULL, 0,
                     self->event_buf_, self->buf_size_, NULL);
        if (ret == 0)
            continue;
        if (ret == -1) {
            if (errno == EINTR) {
                errno = 0;
                continue;
            }
            abort();
        }
        for (i = 0, ptr = self->event_buf_; i < ret; i++, ptr++) {
            ctx = ptr->udata;
            __dispatch_event(ptr, ctx);
        }
    }
}

void
pen_event_stop(PenEvent_t *self)
{
    self->eXit_ = true;
}

void
pen_event_destroy(PenEvent_t *self)
{
    close(self->epfd_);
    free(self);
}

#endif
