#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stddef.h>
#include <time.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "../include/pen_types.h"

#if defined(__GNUC__)

typedef int SOCKET;
#   define INVALID_SOCKET -1
#   include <strings.h>
#ifndef alloca
#   define alloca(s) __builtin_alloca((s))
#endif

#elif defined(PEN_WINDOWS)

typedef SSIZE_T ssize_t;
#   define bzero(ptr, sz) memset(ptr, '\0', sz)
#ifndef alloca
#   define alloca _alloca
#endif

#endif

// these two line, get from nginx code
// nginx/src/core/ngx_config.h#0100
#define pen_align(d,a) (((d) + (a - 1)) & ~(a - 1))
#define pen_align_ptr(p,a) pen_align((uintptr_t)(p), (uintptr_t)(a))

typedef struct PenBuffer     PenBuffer_t;
typedef struct PenMqueue     PenMqueue_t;
typedef struct PenContext    PenContext_t;
typedef struct PenEvent      PenEvent_t;
typedef struct PenEventBase  PenEventBase_t;
typedef struct PenReaderBase PenReaderBase_t;


// third types
typedef struct llist_head     LListHead_t;
typedef struct llist2_head    LList2Head_t;
typedef struct llist_node     LListNode_t;
typedef struct list_head      ListHead_t;
typedef struct hlist_head     HListHead_t;
typedef struct hlist_node     HListNode_t;
typedef struct rb_node        RBNode_t;
typedef struct rb_root        RBRoot_t;
typedef struct rb_root_cached RBRootCached_t;
typedef struct MinHeapItem    MinHeapItem_t;
typedef struct MinHeap        MinHeap_t;

// Factories
#define PEN_FACTORY_NAME(name) typedef struct name ## Factory name ## Factory_t
PEN_FACTORY_NAME(PenContext);
#undef PEN_FACTORY_NAME

typedef void (*PenEventCallback_f)(PenEventBase_t *ev);

struct PenEventBase {
    SOCKET fd_;
    PenEventCallback_f cb_;
    uint32_t state_;
#if HAVE_SYS_EVENT_H
    uint32_t events_;
    int data_size_;
#endif
};
