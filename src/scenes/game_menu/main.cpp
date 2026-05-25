//Game Menu Scene
#include <bits/stdc++.h>
#include "../../objects/player.cpp"
#include "../../objects/header.cpp"
#include "../../objects/button.cpp"

#include "raylib.h"
using namespace std;

const float buttonDefaultWidth = 300.0f, buttonDefaultHeight = 100.0f;
const float buttonDefaultFontSize = 64.0f;

Color initButtonBgColor = BLACK;
Color initButtonTextColor = WHITE;

int main() {
    //Screen Mode: Full Screen
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Among Us (Alpha)");
    SetTargetFPS(60);

    //Among Us Font Load
    Font font = LoadFontEx("../../../resources/fonts/title.ttf", 256, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); //For Sharp Edges
    
    //Game Name and Version
    Header header;

    //Test Button
    Button testButton(
        GetMonitorWidth(0)/2 - 300.0f/2, //posX
        600.0f, //posY
        initButtonBgColor, //bgColor
        initButtonTextColor, //testColor
        buttonDefaultFontSize, //fontSize
        buttonDefaultWidth, //buttonWidth
        buttonDefaultHeight, //buttonHeight
        "Test", //Text
        font, //font-family
        "center", //test-align
        1.0f, //padding
        WHITE, //border-color
        2, //border-weight
        0.3f, //border-radius
        10.0f //spacing
    );

    //Settings Button
    Button settingsButton(
        GetMonitorWidth(0)/2 - 300.0f/2,
        600.0f + testButton.rect.height + 10.0f,
        initButtonBgColor,
        initButtonTextColor,
        buttonDefaultFontSize,
        buttonDefaultWidth,
        buttonDefaultHeight,
        "Settings",
        font,
        "center",
        1.0f,
        WHITE,
        2,
        0.3f,
        10.0f
    );

    //Game Loop
    while (!WindowShouldClose()) {
        //Get Mouse Position
        Vector2 mousePos = GetMousePosition();

        //Hover Effect on both buttons
        if ((mousePos.x >= testButton.rect.x && mousePos.x <= testButton.rect.x + testButton.rect.width) && (mousePos.y >= testButton.rect.y && mousePos.y <= testButton.rect.y + testButton.rect.height)){
            testButton.backgroundColor = WHITE;
            testButton.textColor = BLACK;
        } else testButton.backgroundColor = initButtonBgColor, testButton.textColor = initButtonTextColor;

        if ((mousePos.x >= settingsButton.rect.x && mousePos.x <= settingsButton.rect.x + settingsButton.rect.width) && (mousePos.y >= settingsButton.rect.y && mousePos.y <= settingsButton.rect.y + settingsButton.rect.height)){
            settingsButton.backgroundColor = WHITE;
            settingsButton.textColor = BLACK;
        } else settingsButton.backgroundColor = initButtonBgColor, settingsButton.textColor = initButtonTextColor;

        //client fps
        const char *fps = ("Client FPS: " + to_string(GetFPS())).c_str();

        //Drawing Objects on Scene
        BeginDrawing();
            ClearBackground(BLACK); //Background
            header.Draw(font, "Among Us", "v0.1 (Alpha)"); //Title and Game Version
            testButton.Draw(); //Test Button
            settingsButton.Draw(); //Settings Button

            DrawText(fps, 10, 10, 24, WHITE); //Client FPS Text (top-left corner)
        EndDrawing();
    }
    //Unload font
    UnloadFont(font);

    CloseWindow();
    return 0;
}