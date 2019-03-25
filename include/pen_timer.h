#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

typedef void (*PenTimerCallback_f)(void *data);
typedef struct PenTimer PenTimer_t;

PenTimer_t *pen_timer_add(int ms, PenTimerCallback_f cb, void *data)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(2)))
    __attribute__((warn_unused_result))
    __attribute__((returns_nonnull))
#endif
    ;

bool pen_timer_del(PenTimer_t *item)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1)))
    __attribute__((warn_unused_result))
#endif
    ;

bool pen_timer_update(PenTimer_t *item, int ms)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1)))
    __attribute__((warn_unused_result))
#endif
    ;


#ifdef __cplusplus
}
#endif
