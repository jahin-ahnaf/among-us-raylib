// Minimal raylib stub for Windows demo builds (no rendering)
#pragma once
#include <string>
#include <vector>
#include <cmath>

struct Color { int r,g,b,a; };
struct Vector2 { float x,y; };
struct Rectangle { float x,y,width,height; };
struct Font { int dummy; };

static const Color WHITE{255,255,255,255};
static const Color LIGHTGRAY{200,200,200,255};
static const Color ORANGE{255,165,0,255};
static const Color BLACK{0,0,0,255};
static const Color SKYBLUE{135,206,235,255};
static const Color GREEN{0,255,0,255};
static const Color BLUE{0,0,255,255};
static const Color PURPLE{128,0,128,255};
static const Color YELLOW{255,255,0,255};

inline Vector2 GetMousePosition(){ return {0,0}; }
inline bool CheckCollisionPointRec(Vector2, const Rectangle&) { return false; }
inline int GetCharPressed(){ return 0; }
inline bool IsKeyPressed(int){ return false; }
inline bool IsMouseButtonPressed(int){ return false; }
inline float GetMouseWheelMove(){ return 0.0f; }

inline int GetScreenWidth(){ return 1280; }
inline int GetScreenHeight(){ return 720; }
inline int GetMonitorWidth(int){ return 1920; }
inline int GetMonitorHeight(int){ return 1080; }
inline int GetFPS(){ return 60; }

inline void DrawTextEx(const Font&, const char*, Vector2, float, float, Color) {}
inline void DrawText(const char*, int, int, int, Color) {}
inline void DrawRectangleRounded(const Rectangle&, float, int, Color) {}
inline void DrawRectangleRoundedLines(const Rectangle&, float, int, Color) {}
inline Vector2 MeasureTextEx(const Font&, const char*, float, float){ return {0,0}; }

inline void ToggleFullscreen() {}
inline bool IsWindowState(int){ return false; }
inline void ClearWindowState(int) {}
inline void SetWindowState(int) {}
inline void SetWindowSize(int,int) {}
inline void SetWindowPosition(int,int) {}
inline void MaximizeWindow() {}

// minimal constants
#define MOUSE_LEFT_BUTTON 0
#define KEY_BACKSPACE 8
#define KEY_ESCAPE 27

inline const char* TextFormat(const char* fmt, ...) { return ""; }
