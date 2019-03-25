#include <assert.h>

#include "../../source/pen_event.h"
#include "../../source/pen_timer.h"


static void
__timer_callback(void *data)
{
    static int debug = 0;
    PenEvent_t *ev = data;
    PenTimer_t *timer = NULL;

    debug ++;

    if (debug == 99) {
        pen_event_stop(ev);
    } else {
        timer = pen_timer_add(100, __timer_callback, ev);
    }

    PEN_LOG_DEBUG("timer callabck.\n");
}

int
main(int argc, char *argv[])
{
    PenEvent_t *ev = NULL;
    PenTimer_t *timer = NULL;

    PEN_OPTION_NAME(log_level) = 0;

    ev = pen_event_init(8);
    pen_timer_init(ev);

    timer = pen_timer_add(100, __timer_callback, ev);
    timer = pen_timer_add(100, __timer_callback, ev);
    timer = pen_timer_add(200, __timer_callback, ev);
    timer = pen_timer_add(100, __timer_callback, ev);
    assert(timer != NULL);

    pen_event_start(ev);

    // exit
    pen_timer_destroy();
    pen_event_destroy(ev);
    return 0;
}
