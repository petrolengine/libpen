#include <stdio.h>
#include <string.h>

#include <3rd/llist.h>

#include "pen_threads.h"
#include "pen_mqueue.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef PEN_MQUEUE_DEBUG
#define PEN_MQUEUE_DEBUG 0
#endif

typedef struct MqueueItem {
    void *data_;
    LListNode_t lnode_;
} MqueueItem_t;

typedef struct MqueueItemPool {
    void *mem_;
    LListNode_t lnode_;
    size_t count_;
    MqueueItem_t *data_;
} MqueueItemPool_t;

typedef struct MqueueDesc {
    LList2Head_t queue_;
    LList2Head_t free_;
    PenMtx_t lock_;
} MqueueDesc_t;

struct PenMqueue {
    void *mem_;
    const char *name_;

    MqueueDesc_t put_desc_;
    MqueueDesc_t get_desc_;
    LListHead_t pool_;
    PenCnd_t cond_;
    bool eXit_;
    size_t capacity_;
#if (PEN_MQUEUE_DEBUG == 1)
    size_t debug_;
#endif
};

PEN_RETURNS_NONNULL
PEN_NONNULL(1)
PEN_NOTHROW
PEN_CONST
static inline MqueueItemPool_t *
__get_pool(PenMqueue_t *self)
{
    return llist_entry(self->pool_.first, MqueueItemPool_t, lnode_);
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline MqueueItem_t *
__get_desc_queue_item(PenMqueue_t *self)
{
    LListNode_t *node = llist2_del_first(&self->get_desc_.queue_);
    return node ? llist_entry(node, MqueueItem_t, lnode_) : NULL;
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline MqueueItem_t *
__put_desc_free_item(PenMqueue_t *self)
{
    LListNode_t *node = llist2_del_first(&self->put_desc_.free_);
    return node ? llist_entry(node, MqueueItem_t, lnode_) : NULL;
}

PenMqueue_t *
pen_mqueue_init(const char *name, size_t init_size)
{
    MqueueItemPool_t *pool = NULL;
    PenMqueue_t *self = NULL;
    size_t total_sz = sizeof(PenMqueue_t) + sizeof(MqueueItemPool_t)
            + sizeof(MqueueItem_t) * init_size + alignof(MqueueItem_t)
            + alignof(PenMqueue_t) + alignof(MqueueItemPool_t);
    void *mem = calloc(1, total_sz);
    if (mem == NULL) {
        PEN_LOG_ERROR("create pen mqueue error, out of memory.\n");
        abort();
    }

    self = (PenMqueue_t*)pen_align_ptr(mem, alignof(PenMqueue_t));
    self->mem_ = mem;
    Mtx_init(&self->put_desc_.lock_, mtx_plain);
    Mtx_init(&self->get_desc_.lock_, mtx_plain);
    Cnd_init(&self->cond_);
    self->name_ = strdup(name);
    self->capacity_ = init_size;
    init_llist_head(&self->pool_);
    init_llist2_head(&self->put_desc_.queue_);
    init_llist2_head(&self->put_desc_.free_);
    init_llist2_head(&self->get_desc_.queue_);
    init_llist2_head(&self->get_desc_.free_);

    pool = (MqueueItemPool_t*)pen_align_ptr(
            (char*)self + sizeof(PenMqueue_t),
            alignof(MqueueItemPool_t));
    pool->count_ = init_size;
    pool->data_ = (MqueueItem_t*)pen_align_ptr(
            (char*)pool + sizeof(MqueueItemPool_t),
            alignof(MqueueItem_t));

    llist_add(&pool->lnode_, &self->pool_);

    return self;
}

PEN_WARN_UNUSED_RESULT
PEN_NOTHROW
PEN_NONNULL(1)
static inline void *
__get_data_from_get_internal(PenMqueue_t *self)
{
    void *data = NULL;
    MqueueItem_t *item = NULL;

    item = __get_desc_queue_item(self);
    if (item == NULL) {
        return NULL;
    }

    data = item->data_;
    llist2_add(&item->lnode_, &self->get_desc_.free_);

#if (PEN_MQUEUE_DEBUG == 1)
    self->debug_++;
#endif
    return data;
}

PEN_WARN_UNUSED_RESULT
PEN_NOTHROW
PEN_NONNULL(1)
static inline void *
__get_data_from_data_pool(PenMqueue_t *self)
{
    void *data = NULL;

    Mtx_lock(&self->get_desc_.lock_);
    data = __get_data_from_get_internal(self);
    Mtx_unlock(&self->get_desc_.lock_);
    return data;
}

PEN_WARN_UNUSED_RESULT
PEN_NOTHROW
PEN_NONNULL(1)
static inline void *
__move_putdesc_to_getdesc(PenMqueue_t *self)
{
    void *data = NULL;

    Mtx_lock(&self->get_desc_.lock_);
    if (!llist2_empty(&self->put_desc_.queue_)) {
        llist2_add_list(&self->put_desc_.queue_, &self->get_desc_.queue_);
        llist2_del_all(&self->put_desc_.queue_);
    }

    data = __get_data_from_get_internal(self);

    if (!llist2_empty(&self->get_desc_.free_)) {
        llist2_add_list(&self->get_desc_.free_, &self->put_desc_.free_);
        llist2_del_all(&self->get_desc_.free_);
    }

    Mtx_unlock(&self->get_desc_.lock_);
    return data;
}

void *
pen_mqueue_get(PenMqueue_t *self)
{
    void *data = __get_data_from_data_pool(self);

    if (data != NULL) {
        return data;
    }

    Mtx_lock(&self->put_desc_.lock_);

    while ((data = __move_putdesc_to_getdesc(self)) == NULL) {
        if (self->eXit_) {
            Mtx_unlock(&self->put_desc_.lock_);
            return NULL;
        }
        Cnd_wait(&self->cond_, &self->put_desc_.lock_);
    }
    Mtx_unlock(&self->put_desc_.lock_);
    return data;
}

PEN_NOTHROW PEN_NONNULL(1)
PEN_WARN_UNUSED_RESULT
PEN_RETURNS_NONNULL
static inline MqueueItem_t *
__pen_mqueueitempool_new(PenMqueue_t *self)
{
    MqueueItemPool_t *np = NULL;
    size_t total_sz = sizeof(MqueueItemPool_t)
            + sizeof(MqueueItem_t) * self->capacity_
            + alignof(MqueueItemPool_t) + alignof(MqueueItem_t);
    void *mem = calloc(1, total_sz);
    if (mem == NULL) {
        PEN_LOG_ERROR("pen mqueue item pool new error, out of memory.\n");
        abort();
    }

    np = (MqueueItemPool_t*) pen_align_ptr(mem, alignof(MqueueItemPool_t));
    np->mem_ = mem;
    np->count_ = self->capacity_ - 1; // '-1' for return one
    np->data_ = (MqueueItem_t*)pen_align_ptr(
            (char*)np + sizeof(MqueueItemPool_t),
            alignof(MqueueItem_t));

    llist_add(&np->lnode_, &self->pool_);
    self->capacity_ += self->capacity_;
    PEN_LOG_DEBUG("%s new item pool block, capacity: %lu.\n"
            , self->name_, self->capacity_);
    return np->data_++;
}

PEN_NOTHROW
PEN_NONNULL(1)
PEN_RETURNS_NONNULL
PEN_WARN_UNUSED_RESULT
static inline MqueueItem_t *
__pen_mqueueitem_get(PenMqueue_t *self)
{
    MqueueItemPool_t *pool = NULL;
    MqueueItem_t *ret = NULL;

    ret = __put_desc_free_item(self);
    if (ret == NULL) {
        pool = __get_pool(self);
        if (pool->count_ == 0) {
            ret = __pen_mqueueitempool_new(self);
        } else {
            ret = pool->data_++;
            pool->count_ --;
        }
    }
#if (PEN_MQUEUE_DEBUG == 1)
    else
    {
        self->debug_ --;
    }
#endif
    return ret;
}

void
pen_mqueue_put(PenMqueue_t *self, void *data)
{
    MqueueItem_t *item = NULL;
    bool need_signal = false;

    Mtx_lock(&self->put_desc_.lock_);

    item = __pen_mqueueitem_get(self);
    item->data_ = data;

    need_signal = llist2_add(&item->lnode_, &self->put_desc_.queue_);

    Mtx_unlock(&self->put_desc_.lock_);

    if (need_signal) {
        Cnd_signal(&self->cond_);
    }
}

void
pen_mqueue_exit(PenMqueue_t *self)
{
    self->eXit_ = true;
    Cnd_broadcast(&self->cond_);
}

void
pen_mqueue_destroy(PenMqueue_t *self)
{
    LListNode_t *pos = NULL, *n = NULL;
    MqueueItemPool_t *cur = NULL;

#if (PEN_MQUEUE_DEBUG == 1)
    PEN_LOG_DEBUG("%s total get %lu\n", self->name_, self->debug_);
#endif

    Mtx_destroy(&self->put_desc_.lock_);
    Mtx_destroy(&self->get_desc_.lock_);
    Cnd_destroy(&self->cond_);
    llist_for_each_safe(pos, n, self->pool_.first) {
        cur = llist_entry(pos, MqueueItemPool_t, lnode_);
        if (cur->mem_ != NULL)
            free(cur->mem_);
    }
    free((char*)self->name_);
    free(self->mem_);
}

void
pen_mqueue_print(PenMqueue_t *self)
{
    PEN_LOG_INFO("PenQueue %s: capacity: %lu\n"
            , self->name_, self->capacity_);
}
