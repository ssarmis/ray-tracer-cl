#ifndef _SPACE_H_
#define _SPACE_H_

#include "general.h"
#include "math.h"

typedef struct Ray {
    v3 origin;
    v3 direction;
    v3 color;
    v3 attenuation;
} Ray;

typedef enum ObjectType {
    SPHERE
} ObjectType;

typedef struct Material {
    r32 shininess;
    v3 color;
} Material;

typedef struct Object {
    v3 origin;
    Material material;
    ObjectType type;
    union {
        struct { // sphere
            r32 radius;
        };
    };
} Object;

typedef struct Space {
    u32 numberOfObjects;
    Object objects[20];
} Space;

#endif // _SPACE_H_
