#include <Windows.h>
#include "snake.h"
#include "snake_util.h"

#include <stdio.h> // Debug

typedef struct win32_bitmap_buffer {
    BITMAPINFO info;
    void* memory;
    int32 width;
    int32 height;
    int32 pitch;
} win32_bitmap_buffer;

typedef struct ivec2 {
    int32 x;
    int32 y;
} ivec2;

static bool32 g_isRunning;
static HWND g_window;
static win32_bitmap_buffer g_graphicsBuffer;
static WINDOWPLACEMENT g_windowPosition = { sizeof(g_windowPosition) };

// Credit: Raymond Chen
static void ToggleFullscreen(HWND window) {
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        if (GetWindowPlacement(window, &g_windowPosition) && GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, \
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, \
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &g_windowPosition);
        SetWindowPos(window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static ivec2 GetWindowDimensions(HWND window) {
    RECT clientRect;
    GetClientRect(window, &clientRect);
    return (ivec2){ clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };
}

#define BYTES_PER_PIXEL 4
static void ResizeBitmap(win32_bitmap_buffer* buffer, int32 width, int32 height) {
    if (buffer->memory != NULL) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width  = width;
    buffer->height = height;
    buffer->pitch  = width * BYTES_PER_PIXEL;

    buffer->info.bmiHeader = (BITMAPINFOHEADER){
        .biSize = sizeof(buffer->info.bmiHeader),
        .biWidth = width,
        .biHeight = height,
        .biPlanes = 1,
        .biBitCount = BYTES_PER_PIXEL * 8,
        .biCompression = BI_RGB
    };

    int32 memorySize = width * height * BYTES_PER_PIXEL;
    buffer->memory = VirtualAlloc(NULL, memorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
#undef BYTES_PER_PIXEL

#define KEEP_ASPECT_RATIO 1
static void DisplayBufferInWindow(const win32_bitmap_buffer* buffer, HDC deviceContext, int32 windowWidth, int32 windowHeight) { // replace HDC with HWND?
    if (!buffer->memory) {
        return;
    }

#if KEEP_ASPECT_RATIO
    float32 bufferAspectRatio = (float32)buffer->width / buffer->height;

    if (windowHeight * bufferAspectRatio < windowWidth) {
        int32 xOffset = (windowWidth - windowHeight * bufferAspectRatio) / 2.0f + 1; 

        PatBlt(deviceContext, 0, 0, xOffset, windowHeight, BLACKNESS);
        PatBlt(deviceContext, windowWidth - xOffset, 0, xOffset, windowHeight, BLACKNESS);

        SetStretchBltMode(deviceContext, STRETCH_DELETESCANS);
        StretchDIBits(deviceContext, xOffset, 0, windowHeight * bufferAspectRatio, windowHeight, \
            0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
    }
    else {
        int32 yOffset = (windowHeight - windowWidth / bufferAspectRatio) / 2.0f + 1;

        PatBlt(deviceContext, 0, 0, windowWidth, yOffset, BLACKNESS);
        PatBlt(deviceContext, 0, windowHeight - yOffset, windowWidth, windowHeight, BLACKNESS);

        SetStretchBltMode(deviceContext, STRETCH_DELETESCANS);
        StretchDIBits(deviceContext, 0, yOffset, windowWidth, windowWidth / bufferAspectRatio, \
            0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
    }
#elif 0 
    // Enforce aspect ratio?
#else
    SetStretchBltMode(deviceContext, STRETCH_DELETESCANS);
    StretchDIBits(deviceContext, 0, 0, windowWidth, windowHeight, \
        0, 0, buffer->width, buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
#endif
}
#undef KEEP_ASPECT_RATIO

static void UpdateKeyboardKey(keyboard_key_state* keyState, bool32 isDown) {
    if (keyState->isDown != isDown) {
        keyState->isDown = isDown;
        keyState->didChangeState = true;
    }
}

static void ProcessPendingMessages(keyboard_state* keyboardState) {
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        switch (message.message) {
            case WM_QUIT: {
                g_isRunning = false;
            } break;
            case WM_KEYDOWN:
            case WM_KEYUP: {
                uint32_t vkCode = message.wParam;
                bool32 wasDown = (message.lParam & (1 << 30)) != 0;
                bool32 isDown  = (message.lParam & (1 << 31)) == 0;

                bool32 altKeyIsDown = false;
                if (isDown) {
                    altKeyIsDown = (message.lParam & (1 << 29)) != 0;
                }

                if (wasDown != isDown) {
                    switch (vkCode) {
                        case VK_ESCAPE: {
                            g_isRunning = false;
                        } break;
                        // Debug
                        case 'F': {
                            if (isDown) {
                                ToggleFullscreen(g_window);
                            }
                        } break;
                        // Temporary solution
                        case 'W': {
                            UpdateKeyboardKey(&keyboardState->W.isDown, isDown);
                        } break;
                        case 'A': {
                            UpdateKeyboardKey(&keyboardState->A.isDown, isDown);
                        } break;
                        case 'S': {
                            UpdateKeyboardKey(&keyboardState->S.isDown, isDown);
                        } break;
                        case 'D': {
                            UpdateKeyboardKey(&keyboardState->D.isDown, isDown);
                        } break;
                        case VK_UP: {
                            UpdateKeyboardKey(&keyboardState->up.isDown, isDown);
                        } break;
                        case VK_DOWN: {
                            UpdateKeyboardKey(&keyboardState->down.isDown, isDown);
                        } break;
                        case VK_LEFT: {
                            UpdateKeyboardKey(&keyboardState->left.isDown, isDown);
                        } break;
                        case VK_RIGHT: {
                            UpdateKeyboardKey(&keyboardState->right.isDown, isDown);
                        } break;
                        case 'P': {
                            UpdateKeyboardKey(&keyboardState->P.isDown, isDown);
                        } break;
                    }
                }
            }
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

static inline LARGE_INTEGER GetCurrentPerformanceCount(void) {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value;
}

static inline float32 PerformanceCountDiffInSeconds(LARGE_INTEGER startCount, LARGE_INTEGER endCount, LARGE_INTEGER performanceFrequency) {
    return (endCount.QuadPart - startCount.QuadPart) / (float32)performanceFrequency.QuadPart;
}

static LRESULT CALLBACK Wndproc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
        case WM_CLOSE: {
            g_isRunning = false;
        } break;
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            ivec2 windowDimensions = GetWindowDimensions(window);
            DisplayBufferInWindow(&g_graphicsBuffer, deviceContext, windowDimensions.x, windowDimensions.y);
            EndPaint(window, &paint);
        } break;
    }

    return DefWindowProc(window, message, wParam, lParam);
}

int CALLBACK WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR cmdLine, _In_ int showCmd) {
    WNDCLASSA windowClass = { 
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = Wndproc,
        .hInstance = instance,
        .lpszClassName = "snake window class"
    };
    if (!RegisterClass(&windowClass)) {
        return 1;
    }

    g_window = CreateWindowEx(0, windowClass.lpszClassName, L"Snake", WS_OVERLAPPEDWINDOW | WS_VISIBLE, \
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (g_window == NULL) {
        return 1;
    }

    HDC deviceContext = GetDC(g_window);

    ResizeBitmap(&g_graphicsBuffer, 960, 540);

    timeBeginPeriod(1);

    LARGE_INTEGER performanceFrequence;
    QueryPerformanceFrequency(&performanceFrequence);

    int32 screenRefreshRate = 60;
    int32 actualRefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
    if (actualRefreshRate > 1) {
        screenRefreshRate = actualRefreshRate;
    }
    float32 secondsPerFrame = 1.0f / screenRefreshRate;

    LARGE_INTEGER performanceCountAtStartOfFrame = GetCurrentPerformanceCount();

    keyboard_state keyboardState = { 0 };

    ShowCursor(FALSE);

    OnStartup();

    g_isRunning = true;
    while (g_isRunning) {
        for (int32 i = 0; i < ArraySize(keyboardState.keyStates); ++i) { // Hacky solution
            keyboardState.keyStates[i].didChangeState = false;
        }
        ProcessPendingMessages(&keyboardState);

        bitmap_buffer graphicsBuffer = {
            .memory = g_graphicsBuffer.memory,
            .width  = g_graphicsBuffer.width,
            .height = g_graphicsBuffer.height,
            .pitch  = g_graphicsBuffer.pitch,
            .bitsPerPixel = 4
        };

        // This assumes that secondsPerFrame is always hit, which might not be the case
        Update(&graphicsBuffer, &keyboardState, secondsPerFrame);

        ivec2 windowDimensions = GetWindowDimensions(g_window);
        DisplayBufferInWindow(&g_graphicsBuffer, deviceContext, windowDimensions.x, windowDimensions.y);

        // Is there a better solution? There must be, right?
        LARGE_INTEGER performanceCountAtEndOfFrame = GetCurrentPerformanceCount();
        float32 secondsElapsedForFrame = PerformanceCountDiffInSeconds(performanceCountAtStartOfFrame, performanceCountAtEndOfFrame, performanceFrequence);
        if (secondsElapsedForFrame < secondsPerFrame) {
            float32 millisecondsToSleep = 1000 * (secondsPerFrame - secondsElapsedForFrame);
            if (millisecondsToSleep >= 1) {
                Sleep(millisecondsToSleep - 1);
            }
            while (secondsElapsedForFrame < secondsPerFrame) {
                secondsElapsedForFrame = PerformanceCountDiffInSeconds(performanceCountAtStartOfFrame, GetCurrentPerformanceCount(), performanceFrequence);
            }
        }

#if 0
        LARGE_INTEGER debufPerfCount = GetCurrentPerformanceCount();
        float32 debugSeconds = (debufPerfCount.QuadPart - performanceCountAtStartOfFrame.QuadPart) / (float32)performanceFrequence.QuadPart;
        float debugFPS = 1.0f / debugSeconds;
        char debugBuffer[64];
        sprintf_s(debugBuffer, 64, "%.2f ms/f, %.2f fps\n", 1000.0 * debugSeconds, debugFPS);
        OutputDebugStringA(debugBuffer);
#endif

        performanceCountAtStartOfFrame = GetCurrentPerformanceCount();
    }

    timeEndPeriod(1);

    return 0;
}

//
// Functions accesible to the platform independent layer
//

void EngineToggleFullscreen(void) {
    ToggleFullscreen(g_window);
}

void EngineSetWindowSize(int32 width, int32 height) {
    SetWindowPos(g_window, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void EngineSetWindowPosition(int32 x, int32 y) {
    SetWindowPos(g_window, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void EngineGetWindowSize(int32* width, int32* height) {
    ivec2 windowDimensions = GetWindowDimensions(g_window);
    *width  = windowDimensions.x;
    *height = windowDimensions.y;
}

void EngineSetGraphicsBufferSize(int32 width, int32 height) {
    ResizeBitmap(&g_graphicsBuffer, width, height);
}

void EngineStop() {
    g_isRunning = false;
}

void EngineWriteDebugString(const char* str) {
    OutputDebugStringA(str);
}

// etc.

// Additional dependencies: winmm.lib

//void TestRender(win32_bitmap_buffer* graphicsBuffer) {
//    static int32 xOffset = 0;
//    static int32 yOffset = 0;
//    ++xOffset;
//    ++yOffset;
//
//    u8* row = graphicsBuffer->memory;
//    for (int32 y = 0; y < graphicsBuffer->height; ++y) {
//        u32* pixel = row;
//        for (int32 x = 0; x < graphicsBuffer->width; ++x) {
//            u8 green = x + xOffset;
//            u8 blue  = y + yOffset;
//
//            *pixel++ = (green << 16) | blue;
//        }
//        row += graphicsBuffer->pitch;
//    }
//}