#pragma once

#include <string.h>

#include "platform.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef __cpluspluc
extern "C" {
#endif

#define PEN_READ_BUFFER_SIZE 10240

typedef void (*PenReaderFinishCallback_f)(PenEventBase_t *ctx);
typedef void (*PenReaderClosedCallback_f)(PenEventBase_t *ctx);
typedef void (*PenReaderTcpMessageCallback_f)(
                PenEventBase_t *ctx, PenTcpHeader_t *hdr, void *data);

#if 0
// TODO add http support
typedef void (*PenReaderHttpMessageCallback_f)();
#endif

typedef struct {
    PenReaderFinishCallback_f finish_;
    PenReaderClosedCallback_f closed_;
    PenReaderTcpMessageCallback_f tcp_message_;
} PenReaderCallback_t;

struct PenReaderBase {
    PenReaderCallback_t *cbs_;
    void *rest_;
    size_t rest_len_;
    size_t rest_capacity_;
    bool is_readable_;
};

void pen_read_tcp(PenReaderBase_t *read_ctx, PenEventBase_t *ctx)
    PEN_NONNULL(1,2)
    PEN_NOTHROW;

PEN_VISIBILITY_INTERNAL
extern thread_local char g_read_buf[PEN_READ_BUFFER_SIZE];

PEN_WARN_UNUSED_RESULT
PEN_NOTHROW
static inline ssize_t
pen_read_internal(int fd, const char *rest, size_t len)
{
    if (len > 0)
        memcpy(g_read_buf, rest, len);

    return read(fd, g_read_buf + len, PEN_READ_BUFFER_SIZE - len);
}

void pen_write_tcp(int fd, const void *data, size_t len)
    PEN_NONNULL(2)
    PEN_NOTHROW;

static inline void
pen_write_internal(int fd, int type, const void *data, size_t len)
{
    switch (type) {
        case SOCK_TYPE_TCP:
            pen_write_tcp(fd, data, len);
            break;

        default:
            break;
    }
}

#ifdef __cpluspluc
}
#endif
