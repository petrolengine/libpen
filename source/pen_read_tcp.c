#include <errno.h>

#include "pen_read_internal.h"


PEN_NOTHROW
static inline void
__do_tcp_read(size_t len, PenReaderBase_t *read_ctx, PenEventBase_t *ctx)
{
    PenTcpHeader_t *hdr = NULL;
    size_t rest = len, offset = 0;

    while (rest > 0) {
        if (rest < sizeof(*hdr))
            break;
        hdr = (PenTcpHeader_t*) (g_read_buf + offset);
        if (rest < hdr->len_ + sizeof(*hdr))
            break;
        offset += sizeof(*hdr);
        (*(read_ctx->cbs_->tcp_message_))(ctx, hdr, g_read_buf + offset);
        offset += hdr->len_;
        rest = len - offset;
    }
}

void
pen_read_tcp(PenReaderBase_t *read_ctx, PenEventBase_t *ctx)
{
    ssize_t ret = 0;

    ret = pen_read_internal(ctx->fd_, read_ctx->rest_, read_ctx->rest_len_);

    if (ret == 0) {
       // (*(read_ctx->cbs_->closed_))(ctx);
        return;
    }

    if (ret == -1) {
        if (errno == EWOULDBLOCK) {
            errno = 0;
            read_ctx->is_readable_ = false;
            (*(read_ctx->cbs_->finish_))(ctx);
            return;
        }
        PEN_LOG_ERROR("pen_read_tcp error!!!\n");
        (*read_ctx->cbs_->closed_)(ctx);
        return;
    }

    ret += read_ctx->rest_len_;
    __do_tcp_read(ret, read_ctx, ctx);

    read_ctx->is_readable_ = (ret == PEN_READ_BUFFER_SIZE);
    (*(read_ctx->cbs_->finish_))(ctx);
}

void
pen_write_tcp(int fd, const void *data, size_t len)
{
    PenTcpHeader_t hdr;
    hdr.len_ = (uint32_t)len;
    assert(write(fd, &hdr, sizeof(hdr)) == sizeof(hdr));
    assert(write(fd, data, len) == (ssize_t)len);
}
