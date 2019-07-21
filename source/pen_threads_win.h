#ifndef PEN_THREADS_H
# error "Don't include this header direct. include pen_threads.h insterd."
#endif

#ifdef PEN_WINDOWS

#include <thr/xthreads.h>
#include "log.h"

enum
{
	thrd_success = _Thrd_success,
	thrd_busy = _Thrd_busy,
	thrd_error = _Thrd_error,
	thrd_nomem = _Thrd_nomem,
	thrd_timedout = _Thrd_timedout
};

/* Mutex types.  */
enum
{
	mtx_plain = _Mtx_plain,
	mtx_recursive = _Mtx_recursive,
	mtx_timed = _Mtx_timed
};



typedef _Mtx_t PenMtx_t;
typedef _Cnd_t PenCnd_t;

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_init(PenMtx_t* mtx, int type)
{
	if (_Mtx_init(mtx, type) != _Thrd_success) {
		PEN_LOG_ERROR("mtx init error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_lock(PenMtx_t* mtx)
{
	if (_Mtx_lock(*mtx) != _Thrd_success) {
		PEN_LOG_ERROR("mtx lock error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_unlock(PenMtx_t* mtx)
{
	if (_Mtx_unlock(*mtx) != _Thrd_success) {
		PEN_LOG_ERROR("mtx unlock error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Mtx_destroy(PenMtx_t* mtx)
{
	_Mtx_destroy(*mtx);
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_init(PenCnd_t* cnd)
{
	if (_Cnd_init(cnd) != _Thrd_success) {
		PEN_LOG_ERROR("cnd init error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_wait(PenCnd_t* cnd, PenMtx_t* mtx)
{
	if (_Cnd_wait(*cnd, *mtx) != _Thrd_success) {
		PEN_LOG_ERROR("cnd wait error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_signal(PenCnd_t* cnd)
{
	if (_Cnd_signal(*cnd) != _Thrd_success) {
		PEN_LOG_ERROR("cnd signal error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_broadcast(PenCnd_t* cnd)
{
	if (_Cnd_broadcast(*cnd) != _Thrd_success) {
		PEN_LOG_ERROR("cnd broadcast error.\n");
		abort();
	}
}

PEN_NONNULL(1)
PEN_NOTHROW
static inline void
Cnd_destroy(PenCnd_t* cnd)
{
	_Cnd_destroy(*cnd);
}


#endif