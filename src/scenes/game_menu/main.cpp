//Game Menu Scene
#include <bits/stdc++.h>
#include <steam/steam_api.h>
#include "../../objects/player.cpp"
#include "../../objects/header.cpp"
#include "../../objects/button.cpp"

#include "raylib.h"
using namespace std;

const float buttonDefaultWidth = 300.0f, buttonDefaultHeight = 100.0f;
const float buttonDefaultFontSize = 64.0f;

Color initButtonBgColor = BLACK;
Color initButtonTextColor = WHITE;

enum class SceneMode {
    Menu,
    Settings
};

enum class SettingsTab {
    Display,
    Controls,
    Graphics,
    Audio,
    Gameplay
};

enum class ScreenModePreset {
    MaximizedWindowed,
    Windowed,
    Fullscreen
};

static bool MouseOver(const Rectangle &rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

static void DrawHud(const string &playerName, bool steamInitialized) {
    string fpsText = "Client FPS: " + to_string(GetFPS());
    DrawText(fpsText.c_str(), 10, 10, 24, WHITE);
    DrawText(TextFormat("Welcome %s", steamInitialized ? playerName.c_str() : "Guest"), 10, 40, 24, WHITE);
}

static const char *GetScreenModeLabel(ScreenModePreset mode) {
    switch (mode) {
        case ScreenModePreset::MaximizedWindowed:
            return "Maximized Windowed";
        case ScreenModePreset::Windowed:
            return "Windowed";
        default:
            return "Full Screen Mode";
    }
}

static void ApplyScreenMode(ScreenModePreset mode) {
    ClearWindowState(FLAG_FULLSCREEN_MODE);
    ClearWindowState(FLAG_WINDOW_MAXIMIZED);

    switch (mode) {
        case ScreenModePreset::MaximizedWindowed:
            SetWindowState(FLAG_WINDOW_RESIZABLE);
            MaximizeWindow();
            break;
        case ScreenModePreset::Windowed:
            SetWindowState(FLAG_WINDOW_RESIZABLE);
            SetWindowSize(1280, 720);
            SetWindowPosition(
                (GetMonitorWidth(0) - 1280) / 2,
                (GetMonitorHeight(0) - 720) / 2
            );
            break;
        case ScreenModePreset::Fullscreen:
            ToggleFullscreen();
            break;
    }
}

static void DrawMenuScene(Font font, SceneMode &sceneMode, bool &showSettingsHint, const string &playerName, bool steamInitialized) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    Header header;

    Button testButton(
        screenW / 2.0f - buttonDefaultWidth / 2.0f,
        600.0f,
        initButtonBgColor,
        initButtonTextColor,
        buttonDefaultFontSize,
        buttonDefaultWidth,
        buttonDefaultHeight,
        "Test",
        font,
        "center",
        1.0f,
        WHITE,
        2,
        0.3f,
        10.0f
    );

    Button settingsButton(
        screenW / 2.0f - buttonDefaultWidth / 2.0f,
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

    if (MouseOver(testButton.rect)) {
        testButton.backgroundColor = WHITE;
        testButton.textColor = BLACK;
    } else {
        testButton.backgroundColor = initButtonBgColor;
        testButton.textColor = initButtonTextColor;
    }

    if (MouseOver(settingsButton.rect)) {
        settingsButton.backgroundColor = WHITE;
        settingsButton.textColor = BLACK;
        showSettingsHint = true;
    } else {
        settingsButton.backgroundColor = initButtonBgColor;
        settingsButton.textColor = initButtonTextColor;
        showSettingsHint = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (MouseOver(settingsButton.rect)) {
            sceneMode = SceneMode::Settings;
        }
    }

    header.Draw(font, "@", "v0.1 (Alpha)");
    testButton.Draw();
    settingsButton.Draw();

    DrawHud(playerName, steamInitialized);
}

static void DrawSettingRow(float x, float y, float width, const string &label, const string &value, const string &detail, Font font) {
    Rectangle row = {x, y, width, 76.0f};
    DrawRectangleRounded(row, 0.16f, 10, Color{28, 28, 28, 255});
    DrawRectangleRoundedLines(row, 0.16f, 10, Color{80, 80, 80, 255});

    DrawTextEx(font, label.c_str(), {x + 24.0f, y + 12.0f}, 28.0f, 1.0f, WHITE);
    DrawTextEx(font, detail.c_str(), {x + 24.0f, y + 42.0f}, 18.0f, 1.0f, LIGHTGRAY);

    Vector2 valueSize = MeasureTextEx(font, value.c_str(), 24.0f, 1.0f);
    DrawTextEx(font, value.c_str(), {x + width - valueSize.x - 24.0f, y + (row.height - valueSize.y) / 2.0f}, 24.0f, 1.0f, WHITE);
}

static void DrawScreenModeSelector(Font bodyFont, ScreenModePreset &screenModePreset, float x, float y, float width) {
    Rectangle holder = {x, y, width, 132.0f};
    DrawRectangleRounded(holder, 0.16f, 10, Color{24, 24, 24, 255});
    DrawRectangleRoundedLines(holder, 0.16f, 10, Color{70, 70, 70, 255});

    DrawTextEx(bodyFont, "Screen Mode", {x + 24.0f, y + 16.0f}, 28.0f, 1.0f, WHITE);
    DrawTextEx(bodyFont, "Pick how the window should behave.", {x + 24.0f, y + 44.0f}, 18.0f, 1.0f, LIGHTGRAY);

    const float buttonGap = 12.0f;
    const float buttonWidth = (width - 24.0f * 2.0f - buttonGap * 2.0f) / 3.0f;
    const float buttonY = y + 68.0f;
    const char *labels[] = {"Maximized Windowed", "Windowed", "Full Screen Mode"};
    ScreenModePreset modes[] = {ScreenModePreset::MaximizedWindowed, ScreenModePreset::Windowed, ScreenModePreset::Fullscreen};

    for (int index = 0; index < 3; ++index) {
        Rectangle buttonRect = {x + 24.0f + index * (buttonWidth + buttonGap), buttonY, buttonWidth, 40.0f};
        bool isActive = screenModePreset == modes[index];
        Button optionButton(
            buttonRect.x,
            buttonRect.y,
            isActive ? WHITE : Color{40, 40, 40, 255},
            isActive ? BLACK : WHITE,
            18.0f,
            buttonRect.width,
            buttonRect.height,
            labels[index],
            bodyFont,
            "center",
            1.0f,
            WHITE,
            isActive ? 2 : 1,
            0.2f,
            4.0f
        );

        if (MouseOver(buttonRect) && !isActive) {
            optionButton.backgroundColor = Color{62, 62, 62, 255};
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(buttonRect)) {
            screenModePreset = modes[index];
            ApplyScreenMode(screenModePreset);
        }

        optionButton.Draw();
    }
}

static void DrawSettingsScene(Font titleFont, Font bodyFont, SceneMode &sceneMode, int &activeTab, ScreenModePreset &screenModePreset, const string &playerName, bool steamInitialized) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float margin = 72.0f;
    float tabY = 152.0f;
    float tabGap = 14.0f;
    vector<string> tabNames = {"Display", "Controls", "Graphics", "Audio", "Gameplay"};

    Button backButton(
        screenW - 220.0f,
        50.0f,
        Color{40, 40, 40, 255},
        WHITE,
        32.0f,
        160.0f,
        56.0f,
        "Back",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.25f,
        6.0f
    );

    if (MouseOver(backButton.rect)) {
        backButton.backgroundColor = WHITE;
        backButton.textColor = BLACK;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (MouseOver(backButton.rect)) {
            sceneMode = SceneMode::Menu;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        sceneMode = SceneMode::Menu;
    }

    DrawTextEx(titleFont, "Settings", {margin, 42.0f}, 72.0f, 2.0f, WHITE);
    DrawTextEx(bodyFont, "Template layout - hook these controls to your config later.", {margin, 108.0f}, 24.0f, 1.0f, LIGHTGRAY);
    backButton.Draw();

    float availableWidth = screenW - margin * 2.0f;
    float tabWidth = (availableWidth - tabGap * (tabNames.size() - 1)) / tabNames.size();

    for (int i = 0; i < static_cast<int>(tabNames.size()); ++i) {
        Rectangle tabRect = {margin + i * (tabWidth + tabGap), tabY, tabWidth, 58.0f};
        Button tabButton(
            tabRect.x,
            tabRect.y,
            i == activeTab ? WHITE : Color{26, 26, 26, 255},
            i == activeTab ? BLACK : WHITE,
            24.0f,
            tabRect.width,
            tabRect.height,
            tabNames[i],
            bodyFont,
            "center",
            1.0f,
            WHITE,
            i == activeTab ? 2 : 1,
            0.2f,
            2.0f
        );

        if (MouseOver(tabRect) && i != activeTab) {
            tabButton.backgroundColor = Color{60, 60, 60, 255};
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(tabRect)) {
            activeTab = i;
        }

        tabButton.Draw();
    }

    Rectangle panel = {margin, 230.0f, screenW - margin * 2.0f, screenH - 290.0f};
    DrawRectangleRounded(panel, 0.03f, 10, Color{16, 16, 16, 255});
    DrawRectangleRoundedLines(panel, 0.03f, 10, Color{60, 60, 60, 255});

    DrawTextEx(bodyFont, tabNames[activeTab].c_str(), {panel.x + 24.0f, panel.y + 18.0f}, 34.0f, 1.0f, WHITE);
    DrawTextEx(bodyFont, "Template settings rows - replace any of these with your real widgets.", {panel.x + 24.0f, panel.y + 56.0f}, 20.0f, 1.0f, LIGHTGRAY);

    float rowX = panel.x + 24.0f;
    float rowY = panel.y + 110.0f;
    float rowWidth = panel.width - 48.0f;
    float rowGap = 18.0f;

    if (activeTab == static_cast<int>(SettingsTab::Display)) {
        DrawScreenModeSelector(bodyFont, screenModePreset, rowX, rowY, rowWidth);
        DrawTextEx(bodyFont, TextFormat("Active mode: %s", GetScreenModeLabel(screenModePreset)), {rowX + 6.0f, rowY + 152.0f}, 20.0f, 1.0f, LIGHTGRAY);
    } else if (activeTab == static_cast<int>(SettingsTab::Controls)) {
        DrawSettingRow(rowX, rowY, rowWidth, "Move Up", "W / Up Arrow", "Replace with your key binding editor", bodyFont);
        DrawSettingRow(rowX, rowY + 76.0f + rowGap, rowWidth, "Interact", "E", "Template action row", bodyFont);
        DrawSettingRow(rowX, rowY + (76.0f + rowGap) * 2.0f, rowWidth, "Mouse Sensitivity", "1.00", "Add a slider here later", bodyFont);
    } else if (activeTab == static_cast<int>(SettingsTab::Graphics)) {
        DrawSettingRow(rowX, rowY, rowWidth, "Quality Preset", "High", "Graphics preset placeholder", bodyFont);
        DrawSettingRow(rowX, rowY + 76.0f + rowGap, rowWidth, "VSync", "Enabled", "Swap this for a toggle", bodyFont);
        DrawSettingRow(rowX, rowY + (76.0f + rowGap) * 2.0f, rowWidth, "Bloom", "On", "Optional post-processing slot", bodyFont);
    } else if (activeTab == static_cast<int>(SettingsTab::Audio)) {
        DrawSettingRow(rowX, rowY, rowWidth, "Master Volume", "80%", "Use a slider for this later", bodyFont);
        DrawSettingRow(rowX, rowY + 76.0f + rowGap, rowWidth, "Music Volume", "70%", "Separate channel placeholder", bodyFont);
        DrawSettingRow(rowX, rowY + (76.0f + rowGap) * 2.0f, rowWidth, "SFX Volume", "90%", "Template for effect volume", bodyFont);
    } else {
        DrawSettingRow(rowX, rowY, rowWidth, "Subtitles", "On", "Accessibility toggle template", bodyFont);
        DrawSettingRow(rowX, rowY + 76.0f + rowGap, rowWidth, "Difficulty", "Normal", "Gameplay preset placeholder", bodyFont);
        DrawSettingRow(rowX, rowY + (76.0f + rowGap) * 2.0f, rowWidth, "Language", "English", "Swap for your localization system", bodyFont);
    }

    DrawHud(playerName, steamInitialized);
}

int main() {
    //Screen Mode: Full Screen
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Among Us (Alpha)");
    SetTargetFPS(60);

    bool steamInitialized = SteamAPI_Init();
    if (!steamInitialized) {
        cerr << "Steamworks failed to initialize. Run Steam and launch the game again.\n";
    }

    string playerName = steamInitialized ? SteamFriends()->GetPersonaName() : "Guest";

    //Among Us Font Load
    Font font = LoadFontEx("resources/fonts/title.ttf", 256, 0, 0);
    Font settingsFont = LoadFontEx("resources/fonts/VarelaRound-Regular.ttf", 64, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); //For Sharp Edges
    SetTextureFilter(settingsFont.texture, TEXTURE_FILTER_BILINEAR);

    SceneMode sceneMode = SceneMode::Menu;
    int activeSettingsTab = 0;
    ScreenModePreset screenModePreset = ScreenModePreset::Fullscreen;

    //Game Loop
    while (!WindowShouldClose()) {
        if (steamInitialized) {
            SteamAPI_RunCallbacks(); //Run Steam Callbacks
        }

        BeginDrawing();
        ClearBackground(BLACK); //Background

        bool showSettingsHint = false;
        if (sceneMode == SceneMode::Menu) {
            DrawMenuScene(font, sceneMode, showSettingsHint, playerName, steamInitialized);
        } else {
            DrawSettingsScene(font, settingsFont, sceneMode, activeSettingsTab, screenModePreset, playerName, steamInitialized);
        }

        EndDrawing();
    }

    //Unload font
    UnloadFont(font);
    UnloadFont(settingsFont);
    if (steamInitialized) {
        SteamAPI_Shutdown(); //Shutdown Steam API
    }

    CloseWindow();
    return 0;
}
