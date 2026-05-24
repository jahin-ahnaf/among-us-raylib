//
// Created by jahin on 2026-05-23.
//

#include <raylib.h>
#include <bits/stdc++.h>
using namespace std;

struct Player {
    float x;
    float y;
    float shadowX;
    float shadowY;
    float size;
    float speed;
    float width;
    float height;
    string name;

    Texture2D texture;
    Texture2D shadow;
    Rectangle rec;
    Rectangle shadowRec;

    int frames;
    int currentFrame;
    int framesCounter;
    int framesSpeed;
    bool hasShadow;

    void Create(
        string NAME,
        const char *texturePath,
        float X, float Y,
        float SIZE,
        float SPEED,
        int FRAMES = 1,
        const char *shadowPath = nullptr,
        int FRAMES_SPEED = 8
    ) {
        name = NAME;
        x = X;
        y = Y;
        size = SIZE;
        speed = SPEED;
        frames = (FRAMES < 1) ? 1 : FRAMES;
        framesSpeed = FRAMES_SPEED;
        currentFrame = 0;
        framesCounter = 0;

        texture = LoadTexture(texturePath);
        width = (float)texture.width;
        height = (float)texture.height;

        rec = {0.0f, 0.0f, width / frames, height};

        hasShadow = (shadowPath != nullptr);
        if (hasShadow) {
            shadow = LoadTexture(shadowPath);
            shadowRec = {0.0f, 0.0f, (float)shadow.width, (float)shadow.height};
        }
    }

    void Animate(bool moving) {
        if (frames <= 1) return;

        if (!moving) {
            framesCounter = 0;
            currentFrame = 0;
        } else {
            framesCounter++;
            if (framesCounter >= (60 / framesSpeed)) {
                framesCounter = 0;
                currentFrame++;
                if (currentFrame < 1 || currentFrame > frames - 1) currentFrame = 1;
            }
        }

        rec.x = (float)currentFrame * (width / frames);
    }

    void Draw() {
        int fontSize = static_cast<int>(100.0f * size);
        int centerX = x;

        if (name != "") DrawText(name.c_str(), centerX, y - 20, fontSize, WHITE);
        else DrawText("Guest", centerX, y - 20, fontSize, WHITE);
        if (hasShadow) DrawShadow();
        DrawPlayer();
    }

    void DrawShadow() {
        float drawnPlayerWidth = fabsf(rec.width) * size;
        float drawnPlayerHeight = rec.height * size;
        float drawnShadowWidth = shadow.width * size * 1.7f;
        float drawnShadowHeight = shadow.height * size * 1.7f;

        shadowX = x + (drawnPlayerWidth - drawnShadowWidth) / 2.0f;
        shadowY = y + drawnPlayerHeight - drawnShadowHeight;

        Rectangle dest = {shadowX, shadowY, drawnShadowWidth, drawnShadowHeight};
        DrawTexturePro(shadow, shadowRec, dest, (Vector2){0, 0}, 0.0f, Fade(WHITE, 0.7f));
    }

    void DrawPlayer() {
        Rectangle dest = {x, y, fabsf(rec.width) * size, rec.height * size};
        DrawTexturePro(texture, rec, dest, (Vector2){0, 0}, 0.0f, WHITE);
    }

    void Unload() {
        UnloadTexture(texture);
        if (hasShadow) UnloadTexture(shadow);
    }

    void flipRight() {
        rec.width = fabsf(rec.width);
    }

    void flipLeft() {
        rec.width = -fabsf(rec.width);
    }
};
