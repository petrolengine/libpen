#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*PenConnectedCallback_f)(const char *name, void *data);
typedef void (*PenDisconnectedCallback_f)(
        const char *name, void *data, void *user);
typedef void (*PenConnectMessageCallback_f)(
        const char *name, void *data, void *hdr, void *msg, void *user);
typedef void *(*PenNewClientCallback_f)(PenClient_t *client, void *data);
typedef void (*PenDelClientCallback_f)(void *data, void *user);
typedef void (*PenNewMessageCallback_f)(
        void *data, void *hdr, void *msg, void *user);

enum {
    PEN_CONNECTOR_EVENT_CONNECTED,
    PEN_CONNECTOR_EVENT_DISCONNECTED,
    PEN_CONNECTOR_EVENT_NEW_MESSAGE,
    PEN_CLIENT_EVENT_NEW_CLIENT,
    PEN_CLIENT_EVENT_DEL_CLIENT,
    PEN_CLIENT_EVENT_NEW_MESSAGE,
    PEN_CALLBACK_EVENT_MAX,
};

PEN_VISIBILITY_INTERNAL
PEN_NOTHROW
bool __pen_callback_set_internal(
        uint8_t server_type,
        int event,
        void *cb,
        void *data);

static inline bool
pen_callback_regist_connected(
        uint8_t server_type,
        PenConnectedCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_CONNECTED, cb, data);
}

static inline bool
pen_callback_unregist_connected(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_CONNECTED, NULL, NULL);
}

static inline bool
pen_callback_regist_disconnected(
        uint8_t server_type,
        PenDisconnectedCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_DISCONNECTED, cb, data);
}

static inline bool
pen_callback_unregist_disconnected(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_DISCONNECTED, NULL, NULL);
}

static inline bool
pen_callback_regist_connector_message(
        uint8_t server_type,
        PenConnectMessageCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_NEW_MESSAGE, cb, data);
}

static inline bool
pen_callback_unregist_connector_message(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CONNECTOR_EVENT_NEW_MESSAGE, NULL, NULL);
}

static inline bool
pen_callback_regist_new_client(
        uint8_t server_type,
        PenNewClientCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_NEW_CLIENT, cb, data);
}

static inline bool
pen_callback_unregist_new_client(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_NEW_CLIENT, NULL, NULL);
}

static inline bool
pen_callback_regist_del_client(
        uint8_t server_type,
        PenDelClientCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_DEL_CLIENT, cb, data);
}

static inline bool
pen_callback_unregist_del_client(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_DEL_CLIENT, NULL, NULL);
}

static inline bool
pen_callback_regist_new_message(
        uint8_t server_type,
        PenNewMessageCallback_f cb,
        void *data)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_NEW_MESSAGE, cb, data);
}

static inline bool
pen_callback_unregist_new_message(uint8_t server_type)
{
    return __pen_callback_set_internal(server_type,
            PEN_CLIENT_EVENT_NEW_MESSAGE, NULL, NULL);
}

#ifdef __cplusplus
}
#endif
