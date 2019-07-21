#pragma once

#include "pen_file_monitor.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef _MSC_VER
void petrol_engine_init(int argc, const char *argv[]);
void petrol_engine_destroy(void);
#endif

#ifdef __cplusplus
}
#endif
