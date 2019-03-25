#ifndef PEN_THREADS_H
# error "Don't include this header direct. include pen_threads.h insterd."
#endif

#if HAVE_PTHREAD_H

#include <pthread.h>

typedef pthread_mutex_t PenMtx_t;
typedef pthread_cond_t PenCnd_t;

enum
{
  thrd_success  = 0,
  thrd_busy     = 1,
  thrd_error    = 2,
  thrd_nomem    = 3,
  thrd_timedout = 4
};

/* Mutex types.  */
enum
{
  mtx_plain     = 0,
  mtx_recursive = 1,
  mtx_timed     = 2
};

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_init(PenMtx_t *mtx, int type)
{
    (void) type;
    if (pthread_mutex_init(mtx, NULL) != thrd_success) {
        PEN_LOG_ERROR("mtx init error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_lock(PenMtx_t *mtx)
{
    if (pthread_mutex_lock(mtx) != thrd_success) {
        PEN_LOG_ERROR("mtx lock error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_unlock(PenMtx_t *mtx)
{
    if (pthread_mutex_unlock(mtx) != thrd_success) {
        PEN_LOG_ERROR("mtx unlock error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_destroy(PenMtx_t *mtx)
{
    if (pthread_mutex_destroy(mtx) != thrd_success) {
        PEN_LOG_ERROR("mtx unlock error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_init(PenCnd_t *cnd)
{
    if (pthread_cond_init(cnd, NULL) != thrd_success) {
        PEN_LOG_ERROR("cnd init error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_wait(PenCnd_t *cnd, PenMtx_t *mtx)
{
    if (pthread_cond_wait(cnd, mtx) != thrd_success) {
        PEN_LOG_ERROR("cnd wait error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_signal(PenCnd_t *cnd)
{
    if (pthread_cond_signal(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd signal error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_broadcast(PenCnd_t *cnd)
{
    if (pthread_cond_broadcast(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd broadcast error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_destroy(PenCnd_t *cnd)
{
    if (pthread_cond_destroy(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd destroy error.\n");
        abort();
    }
}

#endif

