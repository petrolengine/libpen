#include "pen_event.h"

#if HAVE_SYS_EPOLL_H

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "log.h"

typedef struct epoll_event EpollEvent_t;

struct PenEvent {
    void *mem_;
    int epfd_;
    size_t buf_szie_;
    EpollEvent_t *event_buf_;
    bool eXit_;
};


PenEvent_t *
pen_event_init(size_t buffer_size)
{
    PenEvent_t *self = NULL;
    size_t total_sz = sizeof(PenEvent_t) + alignof(PenEvent_t)
            + sizeof(EpollEvent_t) * buffer_size + alignof(EpollEvent_t);
    void *mem = calloc(1, total_sz);
    if (mem == NULL) {
        PEN_LOG_ERROR("pen event init error, out of memory.\n");
        abort();
    }
    self = (PenEvent_t*)pen_align_ptr(mem, alignof(PenEvent_t));
    self->mem_ = mem;
    self->buf_szie_ = buffer_size;
    self->event_buf_ = (EpollEvent_t*)pen_align_ptr(
            (void*)self + sizeof(PenEvent_t), alignof(EpollEvent_t));
    self->epfd_ = epoll_create(1);
    if (self->epfd_ == -1) {
        PEN_LOG_ERROR("epoll create error.\n");
        abort();
    }

    return self;
}

static inline uint32_t
__get_epoll_event_flags(uint32_t event)
{
    uint32_t ret = EPOLLET | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

    if (event & PEN_EVENT_READ) {
        ret |= EPOLLIN;
    }
    if (event & PEN_EVENT_WRITE) {
        ret |= EPOLLOUT;
    }
#define _(name) EPOLL ## name
    //ret |= _(PRI) | _(RDNORM) | _(RDBAND) | _(WRNORM) | _(WRBAND)
        //| _(MSG) | _(EXCLUSIVE) | _(WAKEUP) | _(ONESHOT);
#undef _
    return ret;
}

bool
pen_event_add(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
{
    EpollEvent_t ev;

    ev.data.ptr = ctx;
    ev.events = __get_epoll_event_flags(event);

    if (epoll_ctl(self->epfd_, EPOLL_CTL_ADD, ctx->fd_, &ev) != 0) {
        PEN_LOG_ERROR("epoll_ctl add error.\n");
        return false;
    }
    return true;
}

bool
pen_event_mod(PenEvent_t *self, uint32_t event, PenEventBase_t *ctx)
{
    EpollEvent_t ev;

    ev.data.ptr = ctx;
    ev.events = __get_epoll_event_flags(event);

    if (epoll_ctl(self->epfd_, EPOLL_CTL_MOD, ctx->fd_, &ev) != 0) {
        PEN_LOG_ERROR("epoll_ctl mod error.\n");
        return false;
    }
    return true;
}

bool
pen_event_del(PenEvent_t *self, PenEventBase_t *ctx)
{
    if (epoll_ctl(self->epfd_, EPOLL_CTL_DEL, ctx->fd_, NULL) != 0) {
        PEN_LOG_ERROR("epoll_ctl del error.\n");
        return false;
    }
    return true;
}

static inline uint32_t
__get_event_flag_by_epoll(uint32_t event)
{
    uint32_t ret = 0;

    if (event & EPOLLIN) {
        ret |= PEN_EVENT_STATE_READ;
    }
    if (event & EPOLLOUT) {
        ret |= PEN_EVENT_STATE_WRITE;
    }
    if (event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        ret |= PEN_EVENT_STATE_CLOSE;
    }
    return ret;
}

void
pen_event_start(PenEvent_t *self)
{
    int ret = 0, i;
    EpollEvent_t *ptr = NULL;
    PenEventBase_t *ctx = NULL;

    while (!self->eXit_) {
        ret = epoll_wait(self->epfd_, self->event_buf_, self->buf_szie_, -1);
        if  (ret == 0) {
            continue;
        }
        if (ret == -1) {
            if (errno == EINTR) {
                errno = 0;
                continue;
            }
            abort();
        }
        for (i = 0, ptr = self->event_buf_; i < ret; i++, ptr++) {
            ctx = ptr->data.ptr;
            ctx->state_ |= __get_event_flag_by_epoll(ptr->events);
            (*ctx->cb_)(ctx);
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
    free(self->mem_);
}

#endif
