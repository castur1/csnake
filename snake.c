// TODO: Sound?
#include "snake.h"
#include "win32_snake.h"
#include "snake_math.h"
#include "snake_util.h"


#define BOARD_SIZE 21
#define SECONDS_PER_FRAME 0.15f


typedef enum direction {
    direction_up = 0,
    direction_down,
    direction_left,
    direction_right
} direction;

typedef struct game_memory {
    ivec2 snakePos[BOARD_SIZE * BOARD_SIZE];
    int32 snakeLengthCurrent;
    int32 snakeLengthTarget;
    direction snakeDirectionCurrent;
    direction snakeDirectionLast;
    ivec2 applePos;
    float32 timer;
    bool32 isPaused;
} game_memory;


static game_memory g_gameMemory;


static inline uint32 RGB(uint8 red, uint8 green, uint8 blue) {
    return (red << 16) | (green << 8) | blue;
}

static void SetPixelColour(bitmap_buffer* graphicsBuffer, int32 x, int32 y, uint32 colour) {
    int32* memory = graphicsBuffer->memory;
    memory[y * graphicsBuffer->width + x] = colour;
}

static void DrawBackground(bitmap_buffer* graphicsBuffer) {
    uint8* row = graphicsBuffer->memory;
    for (int32 y = 0; y < graphicsBuffer->height; ++y) {
        uint32* pixel = row;
        for (int32 x = 0; x < graphicsBuffer->width; ++x) {
            uint32 colour = RGB(153, 243, 89);
            if ((x + y) % 2) {
                colour = RGB(171, 252, 114);
            }

            *pixel++ = colour;
        }
        row += graphicsBuffer->pitch;
    }
}

void OnStartup() {
    EngineSetGraphicsBufferSize(BOARD_SIZE, BOARD_SIZE);

    g_gameMemory = (game_memory){
        .snakePos = { { BOARD_SIZE / 4, BOARD_SIZE / 2 } },
        .snakeLengthCurrent = 1,
        .snakeLengthTarget = 3,
        .snakeDirectionCurrent = direction_right,
        .snakeDirectionLast = direction_right,
        .applePos = { 3 * BOARD_SIZE / 4, BOARD_SIZE / 2 },
        .timer = 0.0f,
        .isPaused = false
    };
}

void Update(bitmap_buffer* graphicsBuffer, const keyboard_state* keyboardState, float32 deltaTime) {
    if (keyboardState->P.isDown && keyboardState->P.didChangeState) {
        g_gameMemory.isPaused = !g_gameMemory.isPaused;
    }
    if (g_gameMemory.isPaused) {
        return;
    }

    bool32 up    = keyboardState->up.isDown    || keyboardState->W.isDown;
    bool32 down  = keyboardState->down.isDown  || keyboardState->S.isDown;
    bool32 left  = keyboardState->left.isDown  || keyboardState->A.isDown;
    bool32 right = keyboardState->right.isDown || keyboardState->D.isDown;
    if (up && g_gameMemory.snakeDirectionLast != direction_down) {
        g_gameMemory.snakeDirectionCurrent = direction_up;
    }
    else if (right && g_gameMemory.snakeDirectionLast != direction_left) {
        g_gameMemory.snakeDirectionCurrent = direction_right;
    }
    else if (down && g_gameMemory.snakeDirectionLast != direction_up) {
        g_gameMemory.snakeDirectionCurrent = direction_down;
    }
    else if (left && g_gameMemory.snakeDirectionLast != direction_right) {
        g_gameMemory.snakeDirectionCurrent = direction_left;
    }

    g_gameMemory.timer += deltaTime;
    if (g_gameMemory.timer > SECONDS_PER_FRAME) {
        g_gameMemory.timer = 0.0f;

        if (g_gameMemory.snakeLengthCurrent < g_gameMemory.snakeLengthTarget) {
            ++g_gameMemory.snakeLengthCurrent;
        }

        for (int32 i = g_gameMemory.snakeLengthCurrent - 1; i >= 1; --i) {
            g_gameMemory.snakePos[i] = g_gameMemory.snakePos[i - 1];
        }

        switch (g_gameMemory.snakeDirectionCurrent) {
            case direction_up: {
                ++g_gameMemory.snakePos[0].y;
            } break;
            case direction_down: {
                --g_gameMemory.snakePos[0].y;
            } break;
            case direction_left: {
                --g_gameMemory.snakePos[0].x;
            } break;
            case direction_right: {
                ++g_gameMemory.snakePos[0].x;
            } break;
        }

        g_gameMemory.snakeDirectionLast = g_gameMemory.snakeDirectionCurrent;

        if (g_gameMemory.snakePos[0].x < 0 || g_gameMemory.snakePos[0].x >= graphicsBuffer->width || \
            g_gameMemory.snakePos[0].y < 0 || g_gameMemory.snakePos[0].y >= graphicsBuffer->height) {
            OnStartup(); // Janky solution
            return;
        }

        for (int32 i = 1; i < g_gameMemory.snakeLengthCurrent; ++i) {
            if (ivec2_IsEqual(g_gameMemory.snakePos[i], g_gameMemory.snakePos[0])) {
                OnStartup();
                return;
            }
        }
    }

    if (ivec2_IsEqual(g_gameMemory.snakePos[0], g_gameMemory.applePos)) {
        bool32 isAlreadyOccupied;
        do {
            g_gameMemory.applePos = (ivec2){ RandInt32(0, graphicsBuffer->width - 1), RandInt32(0, graphicsBuffer->height - 1) };

            isAlreadyOccupied = false;
            for (int32 i = 0; i < g_gameMemory.snakeLengthCurrent; ++i) {
                if (ivec2_IsEqual(g_gameMemory.snakePos[i], g_gameMemory.applePos)) {
                    isAlreadyOccupied = true;
                    break;
                }
            }
        } while (isAlreadyOccupied);
        ++g_gameMemory.snakeLengthTarget;
    }

    DrawBackground(graphicsBuffer);

    // Render apple
    SetPixelColour(graphicsBuffer, g_gameMemory.applePos.x, g_gameMemory.applePos.y, RGB(255, 110, 38));

    // Render snake
    for (int32 i = 0; i < g_gameMemory.snakeLengthCurrent; ++i) {
        SetPixelColour(graphicsBuffer, g_gameMemory.snakePos[i].x, g_gameMemory.snakePos[i].y, RGB(13, 177, 214));
    }
}