#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void pen_client_del(PenClient_t *client)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1)))
#endif
    ;

void pen_client_send(PenClient_t *client, const void *data, size_t len)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(1,2)))
#endif
    ;

#ifdef __cplusplus
}
#endif
