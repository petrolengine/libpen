#include <assert.h>
#include <errno.h>

#include "pen_context_factory.h"
#include "pen_event.h"
#include "pen_threads.h"

#define min_heap_elem_greater(a, b) min_heap_elem_greater_tm(a,b)
#include "pen_timer.h"

#include "minheap.h"

#if HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

struct PenTimer {
    MinHeapItem_t node_;
    void *data_;
    PenTimerCallback_f cb_;
};

static struct {
    PenEventBase_t ctx_;
    PenContextFactory_t *cf_;
    PenEvent_t *ev_;
    MinHeap_t mh_;
    PenMtx_t mtx_;
#if HAVE_SYS_TIMERFD_H
    uint64_t buf_;
#endif
    struct timespec min_time_;
} self;



#if HAVE_SYS_TIMERFD_H

static inline void
__update_timer(PenTimer_t *pt)
{
    int ret = 0;
    struct itimerspec new_val;

    bzero(&new_val, sizeof(new_val));
    new_val.it_value.tv_sec = pt->node_.tm.tv_sec;
    new_val.it_value.tv_nsec = pt->node_.tm.tv_nsec;
    self.min_time_.tv_sec = pt->node_.tm.tv_sec;
    self.min_time_.tv_nsec = pt->node_.tm.tv_nsec;

    ret = timerfd_settime(self.ctx_.fd_, TFD_TIMER_ABSTIME, &new_val, NULL);
    if (ret != 0) {
        PEN_LOG_ERROR("timerfd settime error.\n");
        abort();
    }
}

#else

static inline void
__update_timer(PenTimer_t *pt)
{
    self.min_time_.tv_sec = pt->node_.tm.tv_sec;
    self.min_time_.tv_nsec = pt->node_.tm.tv_nsec;

    PEN_LOG_DEBUG("UPDATE TIMER.\n");

    if(!pen_event_timer_add(self.ev_, &self.ctx_, &pt->node_.tm)) {
        PEN_LOG_ERROR("timer settime error.\n");
        abort();
    }
}

#endif

static inline void
__do_timeout()
{
    MinHeapItem_t *item = NULL;
    PenTimer_t *pt = NULL;
    bool eXit = false;
    void *data = NULL;
    PenTimerCallback_f cb = NULL;

    while (!eXit) {
        eXit = true;
        Mtx_lock(&self.mtx_);
        item = min_heap_top(&self.mh_);
        if (item) {
            pt = PEN_STRUCT_ENTRY(item, PenTimer_t, node_);
            if (!pen_timespec_cmp(item->tm, self.min_time_, >)) {
                eXit = false;
                assert(min_heap_erase(&self.mh_, item) != -1);
                cb = pt->cb_;
                data = pt->data_;
                pen_context_factory_put(self.cf_, pt);
            } else
                __update_timer(pt);
        }
        Mtx_unlock(&self.mtx_);

        if (!eXit)
            (*cb)(data);
    }
}

#if HAVE_SYS_TIMERFD_H

static void
__callback_event(PenEventBase_t *ctx)
{
    ssize_t len = 0;

    while ((len = read(ctx->fd_, &self.buf_, sizeof(self.buf_))) > 0) {
        if (len != sizeof(self.buf_)) {
            PEN_LOG_ERROR("[timer] internal error. read %ld data.\n", len);
            abort();
        }

        if (self.buf_ == 0)
            return ;

        __do_timeout();
    }
    errno = 0;
}

#else

static void
__callback_event(PenEventBase_t *ctx)
{
    pen_event_clear_timeout_ev(ctx);
    __do_timeout();
}

#endif

void
pen_timer_init(PenEvent_t *ev)
{
    int fd = -1;
    size_t init_sz = PEN_OPTION_NAME(timer_size);

    if (init_sz == 0)
        init_sz = 32;

    bzero(&self, sizeof(self));
    self.cf_ = pen_context_factory_init(
                                        init_sz,
                                        sizeof(PenTimer_t),
                                        alignof(PenTimer_t));
    min_heap_init(&self.mh_, &self.mtx_);

#if HAVE_SYS_TIMERFD_H
    fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1) {
        PEN_LOG_ERROR("timerfd_create error.\n");
        abort();
    }
#else
    fd = 1;
#endif

    self.ctx_.fd_ = fd;
    self.ctx_.cb_ = __callback_event;
    self.ev_ = ev;
    Mtx_init(&self.mtx_, mtx_plain);
#if HAVE_SYS_TIMERFD_H
    if (!pen_event_add(ev, PEN_EVENT_READ, &self.ctx_)) {
        PEN_LOG_ERROR("pen event_add error.\n");
        abort();
    }
#endif
}

void
pen_timer_destroy(void)
{
    Mtx_destroy(&self.mtx_);
    pen_context_factory_destroy(self.cf_);
    min_heap_destroy(&self.mh_);
#if HAVE_SYS_TIMERFD_H
    if (self.ctx_.fd_ > 0)
        close(self.ctx_.fd_);
#endif
    bzero(&self, sizeof(self));
}

PenTimer_t *
pen_timer_add(int ms, PenTimerCallback_f cb, void *data)
{
    PenTimer_t *pt = NULL;
    struct timespec now;
    long nsec = 0;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        PEN_LOG_ERROR("get time error.\n");
        abort();
    }

    now.tv_nsec /= 1E6;
    pt = pen_context_factory_get(self.cf_);
    min_heap_elem_init(&pt->node_);
    nsec = (now.tv_nsec + ms % 1000) * 1000000;
    pt->node_.tm.tv_sec = now.tv_sec + ms / 1000 + nsec / 1000000000L;
    pt->node_.tm.tv_nsec = nsec % 1000000000L;
    pt->cb_ = cb;
    pt->data_ = data;
    if (Min_heap_push_tsafe(&self.mh_, &pt->node_))
        __update_timer(pt);
    return pt;
}

bool
pen_timer_del(PenTimer_t *item)
{
    // TODO bugfix for del last timer(delete event)

    MinHeapItem_t *new_top = NULL;
    PenTimer_t *pt = NULL;

    int ret = Min_heap_erase_tsafe(&self.mh_, &item->node_);
    if (ret == -1) {
        PEN_LOG_ERROR("[timer] internal error.\n");
        return false;
    }
    pen_context_factory_put(self.cf_, item);
    if (ret == 0) {
        Mtx_lock(&self.mtx_);

        new_top = min_heap_top(&self.mh_);
        if (new_top && !pen_timespec_cmp(new_top->tm, self.min_time_, ==)) {
            pt = PEN_STRUCT_ENTRY(new_top, PenTimer_t, node_);
            __update_timer(pt);
        }
        Mtx_unlock(&self.mtx_);
    }
    return true;
}

bool
pen_timer_update(PenTimer_t *pt, int ms)
{
    struct timespec now;
    int ret = 0;

    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        PEN_LOG_ERROR("get time error.\n");
        abort();
    }

    Mtx_lock(&self.mtx_);

    pt->node_.tm.tv_sec = now.tv_sec + ms / 1000;
    pt->node_.tm.tv_nsec = now.tv_nsec + ms % 1000 * 1E6;

    ret = min_heap_adjust(&self.mh_, &pt->node_);
    if (ret == 0 && !pen_timespec_cmp(pt->node_.tm, self.min_time_, ==))
        __update_timer(pt);

    Mtx_unlock(&self.mtx_);

    return (ret != -1);
}
