#include <assert.h>

#include <3rd/llist.h>
#include <3rd/list.h>

#include "pen_threads.h"
#include "pen_context_factory.h"

typedef struct PenContextItem {
    ListHead_t node_;
    LListNode_t lnode_;
} PenContextItem_t;

typedef struct PenContextPool {
    void *mem_;
    LListNode_t lnode_;
    size_t count_;
    char *data_;
} PenContextPool_t;

struct PenContextFactory {
    void *mem_;
    LListHead_t free_list_;
    LListHead_t pool_;
    ListHead_t outers_;
    PenMtx_t lock_;
    size_t ctx_size_;
    size_t size_;
    size_t capacity_;
};

PEN_RETURNS_NONNULL
PEN_NONNULL(1)
PEN_NOTHROW
PEN_CONST
static inline PenContextPool_t *
__get_pool(PenContextFactory_t *self)
{
    return llist_entry(self->pool_.first, PenContextPool_t, lnode_);
}

PEN_RETURNS_NONNULL
PEN_NONNULL(1)
PEN_NOTHROW
PEN_CONST
static inline PenContextItem_t *
__get_item(PenContextFactory_t *self)
{
    return llist_entry(llist_del_first(&self->free_list_),
            PenContextItem_t,
            lnode_);
}

PenContextFactory_t *
pen_context_factory_init(size_t init_size, size_t ctx_size, size_t ctx_align)
{
    PenContextPool_t *pool = NULL;
    PenContextFactory_t *self = NULL;
    size_t total_sz = sizeof(PenContextFactory_t) + sizeof(PenContextPool_t)
                + (ctx_size + sizeof(PenContextItem_t)) * init_size
                + alignof(PenContextFactory_t)
                + alignof(PenContextPool_t) + alignof(PenContextItem_t);
    void *mem = calloc(1, total_sz);

    // TODO add suuport for other ctx_align.
    assert(ctx_align == alignof(PenContextItem_t));
    if (mem == NULL) {
        PEN_LOG_ERROR("create pen context factory error, out of memory.\n");
        abort();
    }
    self =
        (PenContextFactory_t*)pen_align_ptr(mem, alignof(PenContextFactory_t));
    self->mem_ = mem;
    self->ctx_size_ = ctx_size;
    init_llist_head(&self->free_list_);
    init_llist_head(&self->pool_);
    INIT_LIST_HEAD(&self->outers_);

    pool = (PenContextPool_t*)pen_align_ptr(
            (char*)self + sizeof(PenContextFactory_t),
            alignof(PenContextFactory_t));
    pool->count_ = init_size;
    pool->data_ = (char*)pen_align_ptr(
            (char*)pool + sizeof(PenContextPool_t),
            alignof(PenContextItem_t));

    llist_add(&pool->lnode_, &self->pool_);

    Mtx_init(&self->lock_, mtx_plain);
    self->capacity_ = init_size;
    return self;
}

PEN_RETURNS_NONNULL
PEN_NOTHROW
PEN_NONNULL(1)
PEN_WARN_UNUSED_RESULT
static inline PenContextItem_t *
__pen_context_pool_get(PenContextPool_t *self, size_t ctx_size)
{
    return (PenContextItem_t*)&self->data_[
        (--self->count_) * (sizeof(PenContextItem_t) + ctx_size)];
}

PEN_RETURNS_NONNULL
PEN_NOTHROW
PEN_NONNULL(1)
PEN_WARN_UNUSED_RESULT
static inline PenContextItem_t *
__pen_context_pool_new(PenContextFactory_t *self)
{
    PenContextPool_t *pool = NULL;
    size_t total_sz = sizeof(PenContextPool_t)
                + (sizeof(PenContextItem_t) + self->ctx_size_) * self->capacity_
                + alignof(PenContextPool_t) + alignof(PenContextItem_t);
    void *mem = calloc(1, total_sz);
    if (mem == NULL) {
        PEN_LOG_ERROR("create pen context pool error, out of memory.\n");
        abort();
    }
    pool = (PenContextPool_t*)pen_align_ptr(mem, alignof(PenContextPool_t));
    pool->mem_ = mem;
    pool->count_ = self->capacity_;
    pool->data_ = (char*)pen_align_ptr(
            (void*)pool + sizeof(PenContextPool_t), alignof(PenContextItem_t));
    llist_add(&pool->lnode_, &self->pool_);
    self->capacity_ += self->capacity_;

    PEN_LOG_DEBUG("[ContextFactory] create new pool, capacity: %lu.\n"
            , self->capacity_);

    return __pen_context_pool_get(pool, self->ctx_size_);
}

void *
pen_context_factory_get(PenContextFactory_t *self)
{
    PenContextItem_t *item = NULL;

    Mtx_lock(&self->lock_);
    if (!llist_empty(&self->free_list_)) {
        item = __get_item(self);
    } else if (__get_pool(self)->count_ == 0) {
        item = __pen_context_pool_new(self);
    } else {
        item = __pen_context_pool_get(__get_pool(self), self->ctx_size_);
    }
    bzero(item, sizeof(*item));
    list_add(&item->node_, &self->outers_);
    self->size_ ++;
    Mtx_unlock(&self->lock_);

    return (char*)item + sizeof(*item);
}

void
pen_context_factory_put(PenContextFactory_t *self, void *ctx)
{
    PenContextItem_t *item = ctx - sizeof(PenContextItem_t);

    bzero(ctx, self->ctx_size_);
    Mtx_lock(&self->lock_);
    list_del(&item->node_);
    llist_add(&item->lnode_, &self->free_list_);
    self->size_ --;
    Mtx_unlock(&self->lock_);
}

void *
pen_context_factory_pop(PenContextFactory_t *self)
{
    PenContextItem_t *item = NULL;

    Mtx_lock(&self->lock_);

    if (!list_empty(&self->outers_))
        item = list_entry(self->outers_.next, PenContextItem_t, node_);

    Mtx_unlock(&self->lock_);

    return item == NULL ? NULL : (char*)item + sizeof(*item);
}

void
pen_context_factory_destroy(PenContextFactory_t *self)
{
    LListNode_t *pos = NULL, *n = NULL;
    PenContextPool_t *cur = NULL;

    Mtx_destroy(&self->lock_);
    llist_for_each_safe(pos, n, self->pool_.first) {
        cur = llist_entry(pos, PenContextPool_t, lnode_);
        if (cur->mem_ != NULL)
            free(cur->mem_);
    }
    free(self->mem_);
}

size_t
pen_context_factory_size(PenContextFactory_t *self)
{
    return self->size_;
}
