#include <assert.h>

#include "../../source/pen_event.h"
#include "../../source/pen_listener.h"
#include "../../source/pen_connector.h"
#include "../../source/pen_callback.h"
#include "../../source/pen_timer.h"
#include "../../source/pen_client.h"


static void
__callback_connect_message(const char *name, void *data
        , void *hdr, void *msg, void *user)
{
    (void) data;
    (void) hdr;
    (void) user;
    PEN_LOG_DEBUG("[%s]new connect message: %s\n", name, (char*)msg);
}

static void
__callback_new_message(void *data, void *hdr, void *msg, void *user)
{
    (void) data;
    (void) hdr;
    (void) user;
    PEN_LOG_DEBUG("[client]new message: %s\n", (char*)msg);
}

static void
__callback_timeout2(void *data)
{
    (void) data;
    pen_connector_broadcast(1, "hello", 5);
}

static void*
__callback_connected(const char *name, void *data)
{
    (void) data;
    PenTimer_t *timer = NULL;
    static int num = 0;

    if (++num == 3) {
        timer = pen_timer_add(10, __callback_timeout2, data);
        assert(timer != NULL);
    }
    return NULL;
}

static void
__callback_disconnected(const char *name, void *data, void *user)
{
    static int debug = 0;
    PenEvent_t *ev = data;

    debug ++;

    assert(user == NULL);

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

    pen_client_send(client, "hello", 5);
    timer = pen_timer_add(10000, __callback_timeout, client);
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
    assert(pen_callback_regist_connected(1, __callback_connected, ev));
    assert(pen_callback_regist_new_message(1, __callback_new_message, ev));
    assert(pen_callback_regist_connector_message(1, __callback_connect_message, ev));

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
