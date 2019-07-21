#pragma once

#include "platform.h"
#include "../include/pen_file_monitor.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PEN_FILE_MONITOR_SIZE
#define PEN_FILE_MONITOR_SIZE 1
#endif

void pen_file_monitor_init(PenEvent_t *ev)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

void pen_file_monitor_destroy(PenEvent_t *ev)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

#ifdef __cplusplus
}
#endif
