//
//  pen_connector.h
//  pen
//
//  Created by Linas on 2019/4/18.
//  Copyright © 2019 Linas. All rights reserved.
//

#ifndef pen_connector_h
#define pen_connector_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void pen_connector_send(uint8_t tp, const void *data, size_t len)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(2)))
#endif
    ;

void pen_connector_broadcast(uint8_t tp, const void *data, size_t len)
#ifdef __GNUC__
    __attribute__((nothrow))
    __attribute__((nonnull(2)))
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif /* pen_connector_h */
