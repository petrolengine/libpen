#pragma once

#include "pen_file_monitor.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

bool petrol_engine_init(int argc, char *argv[]);
bool petrol_engine_destroy(void);

#ifdef __cplusplus
}
#endif
