#pragma once

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "pen_types.h"
#include "pen_constructor.h"

#ifdef __GNUC__

#define PEN_CHECK_FMT(a,b) __attribute__((format(printf,a,b)))
#define PEN_VISIBILITY_INTERNAL __attribute__((visibility("internal")))
#define PEN_NOTHROW __attribute__((nothrow))
#define PEN_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define PEN_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define PEN_RETURNS_NONNULL __attribute__((returns_nonnull))
#define PEN_CONST __attribute__((__const__))
#define PEN_PURE __attribute__((__pure__))
#define PEN_UNUSED __attribute__((unused))
#define PEN_CONSTRUCTOR(pri) __attribute__((constructor(pri))) static
#define PEN_DESTRUCTOR(pri) __attribute__((destructor(pri))) static

#elif defined(PEN_WINDOWS)

#define PEN_CHECK_FMT(a,b)
#define PEN_VISIBILITY_INTERNAL
#define PEN_NOTHROW
#define PEN_WARN_UNUSED_RESULT
#define PEN_NONNULL(...)
#define PEN_RETURNS_NONNULL
#define PEN_UNUSED
#define PEN_CONST
#define PEN_PURE
#define PEN_CONSTRUCTOR(pri)
#define PEN_DESTRUCTOR(pri)

#else

#error "unsupport platform"

#endif


#if HAVE_THREADS_H
#include <threads.h>
#elif defined(__GNUC__)
#define thread_local __thread
#elif defined(PEN_WINDOWS)
#define thread_local __declspec(thread)
#endif


#include "log.h"
#include "option.h"

#define PEN_FOREACH_SERVER_TYPES() \
    for (size_t i = 0; i < PEN_OPTION_NAME(server_types_size); i++)
#define PEN_IS_VALIDE_SERVER_TYPES(t) \
    (t > 0 && t <= PEN_OPTION_NAME(server_types_size))

#define PEN_STRUCT_ENTRY(ptr, type, member)        \
    ((type *)((char*)(ptr) - offsetof(type, member)))

#define pen_timespec_cmp(a, b, CMP) \
          (((a).tv_sec == (b).tv_sec) ? \
           ((a).tv_nsec CMP (b).tv_nsec) : \
            ((a).tv_sec CMP (b).tv_sec))

#define PEN_MAX(a, b) (((a) > (b)) ? (a) : (b))

extern const char *appname;
