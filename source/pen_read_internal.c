#include "pen_read_internal.h"

#if HAVE_THREADS_H
#include <threads.h>
#endif

thread_local char g_read_buf[PEN_READ_BUFFER_SIZE];
