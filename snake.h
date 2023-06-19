#ifndef SNAKE_H
#define SNAKE_H

#include "snake_util.h"

typedef struct bitmap_buffer {
    void* memory;
    int32 width;
    int32 height;
    int32 pitch;
    int32 bitsPerPixel;
} bitmap_buffer;

// Temporary solution
typedef struct keyboard_key_state {
    bool32 isDown;
    bool32 didChangeState;
} keyboard_key_state;

// Temporary solution
typedef union keyboard_state {
    struct {
        keyboard_key_state W;
        keyboard_key_state A;
        keyboard_key_state S;
        keyboard_key_state D;
        keyboard_key_state up;
        keyboard_key_state down;
        keyboard_key_state left;
        keyboard_key_state right;
        keyboard_key_state P;
    };
    keyboard_key_state keyStates[9];
} keyboard_state;

extern void OnStartup();

extern void Update(bitmap_buffer* graphicsBuffer, const keyboard_state* keyboardState, float32 deltaTime);

#endif