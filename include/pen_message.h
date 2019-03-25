#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct {
    uint32_t len_;
    uint16_t reqid_;
    uint16_t seqid_;
} PenTcpHeader_t;

typedef struct {
    uint64_t tm_;
} PenTcpPingPong_t;

#pragma pack(pop)


// TODO http support
#if 0

typedef struct {

} PenHttpHeader_t;

#endif

#ifdef __cplusplus
}
#endif
