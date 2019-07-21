#include "pen_file_monitor.h"

#ifdef HAVE_SYS_INOTIFY_H

#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "pen_event.h"

#define PEN_IN_EVENTS (IN_CLOSE_WRITE | IN_IGNORED)

typedef struct PenFileMonitor {
    PenEventBase_t ctx_;
    char buf_[1024];
    uint32_t offset_;
    const char *filenames_[PEN_FILE_MONITOR_SIZE];
    int wds_[PEN_FILE_MONITOR_SIZE];
    PenFileMonitorCallback_f cbs_[PEN_FILE_MONITOR_SIZE];
} PenFileMonitor_t;

static PenFileMonitor_t self;

static inline int
__get_idx_by_wd(int wd)
{
    for (int i = 0; i < PEN_FILE_MONITOR_SIZE; i++) {
        if (wd == self.wds_[i]) {
            return i;
        }
    }
    return -1;
}

static inline void
__do_rewatch(int idx)
{
    const char *fn = self.filenames_[idx];

    self.wds_[idx] = inotify_add_watch(self.ctx_.fd_, fn, PEN_IN_EVENTS);
    if (self.wds_[idx] == -1) {
        PEN_LOG_DEBUG("inotify rewatch add watch error.\n");
    }
}

static inline uint32_t
__get_event_mask(uint32_t mask, int idx)
{
    if (mask & IN_IGNORED) {
        if (access(self.filenames_[idx], F_OK) == 0) {
            __do_rewatch(idx);
            return PEN_FILE_MONITOR_EVENT_MODIFY;
        }
        return PEN_FILE_MONITOR_EVENT_DELETE;
    }
    if (mask & IN_CLOSE_WRITE) {
        return PEN_FILE_MONITOR_EVENT_MODIFY;
    }
    return 0;
}

static inline void
__do_event(uint32_t len)
{
    uint32_t offset = 0, mask = 0;
    int idx = -1;
    struct inotify_event *ev = NULL;
    char *ptr = self.buf_;

    while (len >= sizeof(struct inotify_event) + offset) {
        ev = (struct inotify_event*)(ptr + offset);
        if (ev->len > 0) {
            if (offset + ev->len + sizeof(struct inotify_event) > len) {
                PEN_LOG_WARN("inotify warning!!! Get with len: %u\n", ev->len);
                // need more data
                break;
            }
            PEN_LOG_WARN("inotify warning!!! Get with name: %s\n", ev->name);
            offset += ev->len;
        }
        offset += sizeof(struct inotify_event);
        idx = __get_idx_by_wd(ev->wd);
        if (idx == -1) {
            continue;
        }
        mask = __get_event_mask(ev->mask, idx);

        if (mask > 0) {
            (*self.cbs_[idx])(self.filenames_[idx], mask);
            if (mask == PEN_FILE_MONITOR_EVENT_DELETE) {
                self.wds_[idx] = 0;
                free((char*)self.filenames_[idx]);
                self.filenames_[idx] = NULL;
                self.cbs_[idx] = NULL;
            }
        }
    }

    if (offset < len) {
        PEN_LOG_DEBUG("pen file monitor, read twice! %u %u\n", offset, len);
        memmove(self.buf_, self.buf_ + offset, len - offset);
        self.offset_ = len - offset;
    }
}

static void
__event_callback(PenEventBase_t *ctx)
{
    int len = 0;

    if (!pen_event_is_readable(ctx)) {
        return ;
    }
    pen_event_clear_read(ctx);
    while (true) {
        len = read(ctx->fd_,
                self.buf_ + self.offset_,
                sizeof(self.buf_) - self.offset_);
        if (len <= 0) {
            break;
        }
        __do_event(len + self.offset_);
    }
}

void
pen_file_monitor_init(PenEvent_t *ev)
{
    bzero(&self, sizeof(self));
    self.ctx_.fd_ = inotify_init1(IN_NONBLOCK);

    if (self.ctx_.fd_ == -1) {
        PEN_LOG_ERROR("inotify_init1 error.\n");
        abort();
    }
    self.ctx_.cb_ = __event_callback;
    self.ctx_.state_ = 0;

    if (!pen_event_add(ev, PEN_EVENT_READ, &self.ctx_)) {
        PEN_LOG_ERROR("inotify add event error.\n");
        abort();
    }
}

void
pen_file_monitor_destroy(PenEvent_t *ev)
{
    if(!pen_event_del(ev, &self.ctx_)) {
        PEN_LOG_ERROR("inotify del event error.\n");
        abort();
    }
    for (int i = 0; i < PEN_FILE_MONITOR_SIZE; i++) {
        if (self.wds_[i] != 0) {
            if (inotify_rm_watch(self.ctx_.fd_, self.wds_[i]) == -1) {
                PEN_LOG_ERROR("inotify rm watch error.\n");
            }
            free((char*)self.filenames_[i]);
        }
    }
    close(self.ctx_.fd_);
}

static inline int
__get_free_fds_idx()
{
    for (int i = 0; i < PEN_FILE_MONITOR_SIZE; i++) {
        if (self.wds_[i] == 0) {
            return i;
        }
    }
    return -1;
}

bool
pen_file_monitor_add(const char *filename, PenFileMonitorCallback_f cb)
{
    int idx = __get_free_fds_idx();

    if (idx == -1) {
        PEN_LOG_WARN("pen file monitor add failed, no more place to add.\n");
        return false;
    }

    self.cbs_[idx] = cb;
    self.filenames_[idx] = strdup(filename);
    self.wds_[idx] = inotify_add_watch(self.ctx_.fd_, filename, PEN_IN_EVENTS);

    if (self.wds_[idx] == -1) {
        PEN_LOG_ERROR("inotify add watch error.");
        abort();
    }
    return true;
}

static inline int
__get_idx_by_filename(const char *filename)
{
    for (int i = 0; i < PEN_FILE_MONITOR_SIZE; i++) {
        if (strcmp(self.filenames_[i], filename) == 0) {
            return i;
        }
    }
    return -1;
}

bool
pen_file_monitor_del(const char *filename)
{
    int idx = __get_idx_by_filename(filename);

    if (idx == -1) {
        PEN_LOG_WARN("pen file monitor del failed, no found.\n");
        return false;
    }
    if (inotify_rm_watch(self.ctx_.fd_, self.wds_[idx]) == -1) {
        PEN_LOG_ERROR("inotify rm watch error.\n");
        abort();
    }
    self.cbs_[idx] = NULL;
    free((char*)self.filenames_[idx]);
    self.filenames_[idx] = NULL;
    self.wds_[idx] = 0;
    return true;
}

#endif
