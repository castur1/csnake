#include "snake_math.h"
#include "snake_util.h"

inline bool32 ivec2_IsEqual(ivec2 a, ivec2 b) {
    return a.x == b.x && a.y == b.y;
}

// Temporary
int32 RandInt32(int32 min, int32 max) {
    static uint32 seed = 0; // Ranomize seed?
    seed = (214013 * seed + 2531011);

    float32 t = (float32)((seed >> 16) & 0x7FFF) / 32768.0f;
    return min + t * (max - min + 1);
}