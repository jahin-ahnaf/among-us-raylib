#include <bits/stdc++.h>
#include "raylib.h"
using namespace std;

struct Header {
    float fontSize = 250.0f;
    float versionSize = 80.0f;
    float spacing = 2.0f;

    void Draw(Font font, const char *title, const char *version) const {
        int screenW = GetMonitorWidth(0);
        int screenH = GetMonitorHeight(0);

        Vector2 textSize = MeasureTextEx(font, title, fontSize, spacing);
        Vector2 pos = {(screenW - textSize.x) / 2.0f, (screenH - textSize.y) / 2.0f - 200.0f};
        DrawTextEx(font, title, pos, fontSize, spacing, WHITE);

        Vector2 versionTextSize = MeasureTextEx(font, version, versionSize, spacing);
        Vector2 versionPos = {(screenW - versionTextSize.x) / 2.0f - 200.0f, pos.y + textSize.y - 250.0f};
        Vector2 versionOrigin = {versionTextSize.x / 2.0f, versionTextSize.y / 2.0f};
        DrawTextPro(font, version, versionPos, versionOrigin, -25.0f, versionSize, spacing, RED);
    }
};