#ifndef SNAKE_MATH_H
#define SNAKE_MATH_H

#include "snake_util.h"

typedef struct ivec2 {
    int32 x;
    int32 y;
} ivec2;

extern inline bool32 ivec2_IsEqual(ivec2 a, ivec2 b);

extern int32 RandInt32(int32 min, int32 max);

#endif