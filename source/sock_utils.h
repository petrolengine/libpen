#pragma once

#include <fcntl.h>

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

PEN_NOTHROW
static inline bool
pen_set_nonblock(SOCKET fd)
{
#ifdef PEN_WINDOWS
	u_long flags = 1;
	return (NO_ERROR == ioctlsocket(fd, FIONBIO, &flags));
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    if (flags & O_NONBLOCK)
        return true;
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) == 0;
#endif
}

#ifdef __cplusplus
}
#endif
