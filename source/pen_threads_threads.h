#ifndef PEN_THREADS_H
# error "Don't include this header direct. include pen_threads.h insterd."
#endif

#if HAVE_THREADS_H
#include <threads.h>

typedef mtx_t PenMtx_t;
typedef cnd_t PenCnd_t;

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_init(PenMtx_t *mtx, int type)
{
    if (mtx_init(mtx, type) != thrd_success) {
        PEN_LOG_ERROR("mtx init error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_lock(PenMtx_t *mtx)
{
    if (mtx_lock(mtx) != thrd_success) {
        PEN_LOG_ERROR("mtx lock error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_unlock(PenMtx_t *mtx)
{
    if (mtx_unlock(mtx) != thrd_success) {
        PEN_LOG_ERROR("mtx unlock error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_destroy(PenMtx_t *mtx)
{
    mtx_destroy(mtx);
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_init(PenCnd_t *cnd)
{
    if (cnd_init(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd init error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_wait(PenCnd_t *cnd, PenMtx_t *mtx)
{
    if (cnd_wait(cnd, mtx) != thrd_success) {
        PEN_LOG_ERROR("cnd wait error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_signal(PenCnd_t *cnd)
{
    if (cnd_signal(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd signal error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_broadcast(PenCnd_t *cnd)
{
    if (cnd_broadcast(cnd) != thrd_success) {
        PEN_LOG_ERROR("cnd broadcast error.\n");
        abort();
    }
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_destroy(PenCnd_t *cnd)
{
    cnd_destroy(cnd);
}

#endif
