#ifndef WIN32_SNAKE_H
#define WIN32_SNAKE_H

extern void EngineToggleFullscreen(void);

extern void EngineSetWindowSize(int32 width, int32 height);

extern void EngineSetWindowPosition(int32 x, int32 y);

extern void EngineGetWindowSize(int32* width, int32* height);

extern void EngineSetGraphicsBufferSize(int32 width, int32 height);

extern void EngineStop();

extern void EngineWriteDebugString(const char* str);

#endif