#include <string.h>

#include "pen_buffer.h"
#include "log.h"

typedef struct BufferBlock {
    char *head_;
    char *tail_;

    struct BufferBlock *next_;

    size_t rest_;
    size_t size_;
    size_t capacity_;
    char data_[1];
} BufferBlock;

struct PenBuffer {
    size_t blocks_;
    size_t capacity_;
    size_t size_;
    BufferBlock *head_;
    BufferBlock *tail_;
    BufferBlock buf_;
};

PEN_NOTHROW
PEN_WARN_UNUSED_RESULT
PEN_RETURNS_NONNULL
static BufferBlock *
__buffer_block_get(size_t init_size)
{
    BufferBlock *self = calloc(1, init_size + sizeof(BufferBlock));
    if (self == NULL) {
        PEN_LOG_ERROR("__buffer_block_get error, out of memory.\n");
        abort();
    }
    self->head_ = self->tail_ = self->data_;
    self->next_ = self;
    self->rest_ = init_size;
    self->size_ = 0;
    self->capacity_ = init_size;
    PEN_LOG_DEBUG("new buffer block: size: %lu\n", init_size);
    return self;
}

PenBuffer_t *
pen_buffer_get(size_t init_size)
{
    PenBuffer_t *self = calloc(1, sizeof(PenBuffer_t) + init_size);
    if (self == NULL) {
        PEN_LOG_ERROR("pen_buffer_get error. out of memory.\n");
        abort();
    }
    self->blocks_ = 1;
    self->capacity_ = init_size;
    self->size_ = 0;
    self->head_ = self->tail_ = &self->buf_;
    self->buf_.head_ = self->buf_.tail_ = self->buf_.data_;
    self->buf_.next_ = self->head_;
    self->buf_.rest_ = init_size;
    self->buf_.size_ = 0;
    self->buf_.capacity_ = init_size;

    return self;
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
__buffer_block_reset(BufferBlock *self)
{
    self->head_ = self->tail_ = self->data_;
    self->size_ = 0;
    self->rest_ = self->capacity_;
    PEN_LOG_DEBUG("reset buffer block: size: %lu\n", self->capacity_);
}

size_t
pen_buffer_read(PenBuffer_t *self, void *buf, const size_t sz)
{
    size_t tmpsz = 0;
    if (self->head_->size_ >= sz) {
        memcpy(buf, self->head_->head_, sz);
        self->head_->size_ -= sz;
        self->head_->head_ += sz;
        self->size_ -= sz;
        if (self->head_->size_ == 0) {
            __buffer_block_reset(self->head_);
            if (self->head_ != self->tail_) {
                self->head_ = self->head_->next_;
            }
        }
        return sz;
    }
    if ((tmpsz = self->head_->size_) > 0) {
        memcpy(buf, self->head_->head_, tmpsz);
        __buffer_block_reset(self->head_);
        self->size_ -= tmpsz;
        if (self->head_ != self->tail_) {
            self->head_ = self->head_->next_;
        }
    } else
        return 0;
    return tmpsz + pen_buffer_read(self, (char*)buf + tmpsz, sz - tmpsz);
}

size_t
pen_buffer_write(PenBuffer_t *self, const void *data, const size_t sz)
{
    size_t tmpsz = 0;
    BufferBlock *newblock = NULL;

    if (self->tail_->rest_ >= sz) {
        memcpy(self->tail_->tail_, data, sz);
        self->tail_->size_ += sz;
        self->tail_->rest_ -= sz;
        self->tail_->tail_ += sz;
        self->size_ += sz;
        return sz;
    }
    if ((tmpsz = self->tail_->rest_) > 0) {
        memcpy(self->tail_->tail_, data, tmpsz);
        self->tail_->size_ += tmpsz;
        self->tail_->tail_ += tmpsz;
        self->tail_->rest_ = 0;
        self->size_ += tmpsz;
    }
    if (self->tail_->next_ == self->head_) {
        // create new block
        newblock = __buffer_block_get(self->capacity_);
        newblock->next_ = self->tail_->next_;
        self->tail_->next_ = newblock;
        self->blocks_ ++;
        self->capacity_ += self->capacity_;
    } else
        self->tail_ = self->tail_->next_;
    // TODO use loop instead.
    return tmpsz + pen_buffer_write(self, (char*)data + tmpsz, sz - tmpsz);
}

void
pen_buffer_free(PenBuffer_t *self)
{
    BufferBlock *todel = self->buf_.next_;
    while (todel != &self->buf_) {
        self->buf_.next_ = todel->next_;
        free(todel);
        todel = self->buf_.next_;
    }
    free(self);
}

void
pen_buffer_print(PenBuffer_t *self)
{
    BufferBlock *cur = &self->buf_;
    PEN_LOG_INFO("PenBuffer: %p, blocks: %lu, capacity: %lu, size: %lu "
            "head: %p, tail: %p\n",
            self, self->blocks_, self->capacity_, self->size_,
            self->head_, self->tail_);
    do {
        PEN_LOG_INFO("\tblock: %p, head: %p, tail: %p, next: %p, data: %p, "
            "rest: %lu, size: %lu, capacity: %lu\n",
            cur, cur->head_, cur->tail_, cur->next_, cur->data_,
            cur->rest_, cur->size_, cur->capacity_);
        cur = cur->next_;
    } while(cur != &self->buf_);
}
