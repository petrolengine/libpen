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

#ifdef __cplusplus
}
#endif
