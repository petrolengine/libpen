/*
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Copyright (c) 2006 Maxim Yegorushkin <maxim.yegorushkin@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MinHeapItem {
    int idx;
    union {
        int64_t val;
        time_t timestamp;
        struct timespec tm;
    };
};

struct MinHeap {
    MinHeapItem_t **p;
    unsigned n, a;
    PenMtx_t *mtx;
};

#define min_heap_elem_greater_tm(a, b) pen_timespec_cmp((a)->tm, (b)->tm, >)
#define min_heap_elem_greater_val(a, b) ((a)->val > (b)->val)

static inline void
min_heap_init(MinHeap_t *s, PenMtx_t *mtx)
{
    bzero(s, sizeof(MinHeap_t));
    s->mtx = mtx;
}

static inline void
min_heap_destroy(MinHeap_t *s)
{
    if (s->p) free(s->p);
}

static inline void
min_heap_elem_init(MinHeapItem_t *e)
{
    e->val = 0;
    e->idx = -1;
}

static inline bool
min_heap_elt_is_top(const MinHeapItem_t *e)
{
    return e->idx == 0;
}

static inline bool
min_heap_empty(MinHeap_t *s)
{
    return 0u == s->n;
}

static inline unsigned
min_heap_size(MinHeap_t *s)
{
    return s->n;
}

static inline MinHeapItem_t *
min_heap_top(MinHeap_t* s)
{
    return s->n ? *s->p : 0;
}

static inline void
__min_heap_reserve(MinHeap_t *s, unsigned n)
{
    if (s->a < n) {
        MinHeapItem_t **p;
        unsigned a = s->a ? s->a * 2 : 8;
        if (a < n)
            a = n;
        if (!(p = (MinHeapItem_t**)realloc(s->p, a * sizeof *p))) {
            PEN_LOG_ERROR("minheap reserve error. out of memory.\n");
            abort();
        }
        s->p = p;
        s->a = a;
    }
}

static inline void
__min_heap_shift_up(MinHeap_t *s, unsigned hole_index, MinHeapItem_t *e)
{
    unsigned parent = (hole_index - 1) / 2;
    while (hole_index && min_heap_elem_greater(s->p[parent], e)) {
        (s->p[hole_index] = s->p[parent])->idx = hole_index;
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->idx = hole_index;
}

static inline bool
min_heap_push(MinHeap_t *s, MinHeapItem_t *e)
{
    __min_heap_reserve(s, s->n + 1);
    __min_heap_shift_up(s, s->n++, e);
    return min_heap_elt_is_top(e);
}

static inline bool
Min_heap_push_tsafe(MinHeap_t *s, MinHeapItem_t *e)
{
    bool ret = false;

    Mtx_lock(s->mtx);
    ret = min_heap_push(s, e);
    Mtx_unlock(s->mtx);

    return ret;
}

static inline void
__min_heap_shift_down(MinHeap_t *s, unsigned hole_index, MinHeapItem_t *e)
{
    unsigned min_child = 2 * (hole_index + 1);
    while (min_child <= s->n) {
        min_child -= min_child == s->n ||
            min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);
        if (!(min_heap_elem_greater(e, s->p[min_child])))
            break;
        (s->p[hole_index] = s->p[min_child])->idx = hole_index;
        hole_index = min_child;
        min_child = 2 * (hole_index + 1);
    }
    (s->p[hole_index] = e)->idx = hole_index;
}

static inline MinHeapItem_t *
min_heap_pop(MinHeap_t *s)
{
    if (s->n) {
        MinHeapItem_t *e = *s->p;
        __min_heap_shift_down(s, 0u, s->p[--s->n]);
        e->idx = -1;
        return e;
    }
    return NULL;
}

static inline MinHeapItem_t *
Min_heap_pop_tsafe(MinHeap_t *s)
{
    MinHeapItem_t *ret = NULL;

    Mtx_lock(s->mtx);
    ret = min_heap_pop(s);
    Mtx_unlock(s->mtx);

    return ret;
}

static inline void
__min_heap_shift_up_unconditional(
        MinHeap_t *s,
        unsigned hole_index,
        MinHeapItem_t *e)
{
    unsigned parent = (hole_index - 1) / 2;

    do {
        (s->p[hole_index] = s->p[parent])->idx = hole_index;
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    } while (hole_index && min_heap_elem_greater(s->p[parent], e));

    (s->p[hole_index] = e)->idx = hole_index;
}

static inline int
min_heap_adjust(MinHeap_t *s, MinHeapItem_t *e)
{
    if (-1 != e->idx) {
        unsigned parent = (e->idx - 1) / 2;
        /* The position of e has changed; we shift it up or down
         * as needed.  We can't need to do both. */
        if (e->idx > 0 && min_heap_elem_greater(s->p[parent], e))
            __min_heap_shift_up_unconditional(s, e->idx, e);
        else
            __min_heap_shift_down(s, e->idx, e);
        return min_heap_elt_is_top(e) ? 0 : 1;
    }
    return -1;
}

static inline int
Min_heap_adjust_tsafe(MinHeap_t *s, MinHeapItem_t *e)
{
    int ret = 0;

    Mtx_lock(s->mtx);
    ret = min_heap_adjust(s, e);
    Mtx_unlock(s->mtx);
    return ret;
}

static inline int
min_heap_erase(MinHeap_t *s, MinHeapItem_t *e)
{
    int ret = -1;

    if (-1 != e->idx) {
        MinHeapItem_t *last = s->p[--s->n];
        unsigned parent = (e->idx - 1) / 2;
        /* we replace e with the last element in the heap.  We might need to
           shift it upward if it is less than its parent, or downward if it is
           greater than one or both its children. Since the children are known
           to be less than the parent, it can't need to shift both up and
           down. */
        if (e->idx > 0 && min_heap_elem_greater(s->p[parent], last))
            __min_heap_shift_up_unconditional(s, e->idx, last);
        else
            __min_heap_shift_down(s, e->idx, last);

        ret = (e->idx == 0) ? 0 : 1;
        e->idx = -1;
    }
    return ret;
}

static inline int
Min_heap_erase_tsafe(MinHeap_t *s, MinHeapItem_t *e)
{
    int ret = 0;

    Mtx_lock(s->mtx);
    ret = min_heap_erase(s, e);
    Mtx_unlock(s->mtx);
    return ret;
}

#ifdef __cplusplus
}
#endif
