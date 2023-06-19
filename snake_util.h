#ifndef SNAKE_UTIL_H
#define SNAKE_UTIL_H

#include <stdint.h>

#define ArraySize(arr) (sizeof(arr) / sizeof(*arr))

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;

typedef int64_t  int64;
typedef int32_t  int32;
typedef int16_t  int16;
typedef int8_t   int8;

typedef float    float32;
typedef double   float64;

typedef int32_t bool32;
#define true 1
#define false 0

#endif