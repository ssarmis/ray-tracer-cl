#ifndef _WORKER_H_
#define _WORKER_H_

#include "general.h"
#include "space.h"

struct Region {
    u32 x;
    u32 y;
    u32 w;
    u32 h;
};

typedef struct WorkerContext {
    Space space;
    u32 width;
    u32 height;
    v3 cameraPosition;
} WorkerContext __attribute__((packed));

#endif // _WORKER_H_
