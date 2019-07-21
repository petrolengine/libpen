#pragma once

#include "platform.h"
#include "../include/pen_callback.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PenConnectEventItem {
    void *cb_;
    void *data_;
} PenCallbackEvent_t;

void pen_callback_init(void);
void pen_callback_destroy(void);

const PenCallbackEvent_t *__pen_callback_get_internal(int servertype, int event)
    PEN_VISIBILITY_INTERNAL
    PEN_NOTHROW;

static inline void*
pen_callback_connected(int servertype, const char *name)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CONNECTOR_EVENT_CONNECTED);
    PenConnectedCallback_f cb = NULL;
    if (ev == NULL || ev->cb_ == NULL)
        return NULL;
    cb = ev->cb_;
    return (*cb)(name, ev->data_);
}

static inline void
pen_callback_disconnected(int servertype, const char *name, void *user)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CONNECTOR_EVENT_DISCONNECTED);
    PenDisconnectedCallback_f cb = NULL;
    if (ev != NULL && ev->cb_ != NULL) {
        cb = ev->cb_;
        (*cb)(name, ev->data_, user);
    }
}

static inline void
pen_callback_connect_message(int servertype, const char *name,
        void *hdr, void *msg, void *user)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CONNECTOR_EVENT_NEW_MESSAGE);
    PenConnectMessageCallback_f cb = NULL;
    if (ev != NULL && ev->cb_ != NULL) {
        cb = ev->cb_;
        (*cb)(name, ev->data_, hdr, msg, user);
    }
}

static inline void *
pen_callback_new_client(PenClient_t *client, int servertype)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CLIENT_EVENT_NEW_CLIENT);
    PenNewClientCallback_f cb = NULL;
    if (ev == NULL || ev->cb_ == NULL)
        return NULL;
    cb = ev->cb_;
    return (*cb)(client, ev->data_);
}

static inline void
pen_callback_del_client(int servertype, void *user)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CLIENT_EVENT_DEL_CLIENT);
    PenDelClientCallback_f cb = NULL;
    if (ev != NULL && ev->cb_ != NULL) {
        cb = ev->cb_;
        (*cb)(user, ev->data_);
    }
}

static inline void
pen_callback_new_message(int servertype, void *hdr, void *msg, void *user)
{
    const PenCallbackEvent_t *ev = __pen_callback_get_internal(servertype,
            PEN_CLIENT_EVENT_NEW_MESSAGE);
    PenNewMessageCallback_f cb = NULL;
    if (ev != NULL && ev->cb_ != NULL) {
        cb = ev->cb_;
        (*cb)(hdr, ev->data_, msg, user);
    }
}

#ifdef __cplusplus
}
#endif
