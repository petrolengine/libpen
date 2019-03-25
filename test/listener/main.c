#include <assert.h>

#include "../../source/pen_event.h"
#include "../../source/pen_listener.h"
#include "../../source/pen_connector.h"
#include "../../source/pen_callback.h"
#include "../../source/pen_timer.h"
#include "../../source/pen_client.h"


static void
__callback_disconnected(const char *name, void *data, void *user)
{
    static int debug = 0;
    PenEvent_t *ev = data;

    debug ++;

    assert(user == NULL);

    PEN_LOG_DEBUG("%s disconnected.\n", name);

    if (debug == 10) {
        pen_event_stop(ev);
    }
}

static void
__callback_timeout(void *data)
{
    PenClient_t *client = data;

    pen_client_del(client);
}

static void *
__callback_new_client(PenClient_t *client, void *data)
{
    PenTimer_t *timer = NULL;

    PEN_LOG_DEBUG("on new client.\n");

    timer = pen_timer_add(1000, __callback_timeout, client);
    assert(timer != NULL);

    return data;
}

int
main(int argc, char *argv[])
{
    PenEvent_t *ev = NULL;

    pen_callback_init();

    ev = pen_event_init(8);

    assert(pen_callback_regist_new_client(1, __callback_new_client, ev));
    assert(pen_callback_regist_disconnected(1, __callback_disconnected, ev));

    pen_timer_init(ev);
    pen_listener_init(ev);
    pen_connector_init(ev);

    pen_event_start(ev);

    pen_connector_destroy();
    pen_listener_destroy();
    pen_timer_destroy();
    pen_event_destroy(ev);
    pen_callback_destroy();
    return 0;
}
