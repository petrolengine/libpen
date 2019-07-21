#include "../include/petrol_engine.h"
#include <stdio.h>

#ifdef _MSC_VER
#include "log.h"
#include "option.h"

void
petrol_engine_init(int argc, const char *argv[])
{
    pen_log_init(argc, argv);
    pen_option_init(argc, argv);   
}

void
petrol_engine_destroy(void)
{

}

#endif
