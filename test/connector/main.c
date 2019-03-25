#include <assert.h>

#include "../../source/pen_event.h"
#include "../../source/pen_callback.h"
#include "../../source/pen_connector.h"

static int debug = 0;

static void *
__callback_connected(const char *name, void *data)
{
    PenEvent_t *ev = data;

    PEN_LOG_DEBUG("%s connected.\n", name);
    debug ++;
    if (debug == 4)
        pen_event_stop(ev);

    return ev;
}

static void
__callback_disconnected(const char *name, void *data, void *user)
{
    assert(data == user);
    PEN_LOG_DEBUG("%s disconnected.\n", name);
}

int
main(int argc, char *argv[])
{
    PenEvent_t *ev = NULL;

    argv[0] = "libpen_connector";
    pen_callback_init();

    ev = pen_event_init(8);

    assert(pen_callback_regist_connected(1, __callback_connected, ev));
    assert(pen_callback_regist_disconnected(1, __callback_disconnected, ev));

    pen_connector_init(ev);


    pen_event_start(ev);

    // exit
    pen_connector_destroy();
    pen_event_destroy(ev);

    assert(pen_callback_unregist_connected(1));
    assert(pen_callback_unregist_disconnected(1));

    pen_callback_destroy();
    return 0;
}
