#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

enum {
    PEN_FILE_MONITOR_EVENT_MODIFY = 1,
    PEN_FILE_MONITOR_EVENT_DELETE,
};

typedef void (*PenFileMonitorCallback_f)(const char *filename, uint32_t mask);

bool pen_file_monitor_add(const char *filename, PenFileMonitorCallback_f cb)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1,2)))
    __attribute__((warn_unused_result))
#endif
    ;

bool pen_file_monitor_del(const char *filename)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1)))
    __attribute__((warn_unused_result))
#endif
    ;

#ifdef __cplusplus
}
#endif
