#pragma once
#define PEN_THREADS_H

#include "platform.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_THREADS_H
#include "pen_threads_threads.h"
#elif HAVE_PTHREAD_H
#include "pen_threads_pthread.h"
#elif defined(PEN_WINDOWS)
#include "pen_threads_win.h"
#endif

#ifdef __cplusplus
}
#endif
