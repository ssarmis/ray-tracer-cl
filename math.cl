#ifndef _MY_MATH_H_
#define _MY_MATH_H_

#include "general.h"

typedef struct v3 {
    r32 x;
    r32 y;
    r32 z;
} v3;


static inline r32 dot(v3 left, v3 right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

static inline v3 cross(v3 left, v3 right){
    v3 result;
    result.x = left.y * right.z - left.z * right.y;
    result.y = -(left.x * right.z - left.z * right.x);
    result.z = left.x * right.y - left.y * right.x;
    return result;
}

static inline v3 normalize(v3 vector){
    r32 length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    vector.x = vector.x / length;
    vector.y = vector.y / length;
    vector.z = vector.z / length;
    return vector;
}

static inline v3 makev3(r32 x, r32 y, r32 z){
    v3 result = {x, y, z};
    return result;
}

static inline v3 sub(v3 left, v3 right){
    v3 result;
    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;
    return result;
} 

static inline v3 hadamard(v3 left, v3 right){
    v3 result;
    result.x = left.x * right.x;
    result.y = left.y * right.y;
    result.z = left.z * right.z;
    return result;
} 

static inline v3 fmulRight(r32 amount, v3 right){
    v3 result;
    result.x = amount * right.x;
    result.y = amount * right.y;
    result.z = amount * right.z;
    return result;
} 

static inline v3 fmulLeft(v3 left, r32 amount){
    v3 result;
    result.x = left.x * amount;
    result.y = left.y * amount;
    result.z = left.z * amount;
    return result;
} 

static inline v3 add(v3 left, v3 right){
    v3 result;
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;
    return result;
} 

static inline r32 absf(r32 value){
    return value < 0 ? -value : value; 
}

static inline v3 lerp(v3 start, v3 end, r32 t){
    return add(fmulRight((1 - t), start), fmulRight(t, end));
}

#endif // _MY_MATH_H_
