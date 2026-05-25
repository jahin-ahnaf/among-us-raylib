#include "raylib.h"
#include "steam/steam_api.h"
#include <iostream>

int main() {
    if (!SteamAPI_Init()) {
        std::cerr << "Steamworks failed to initialize! Make sure Steam is running.\n";
        return 1;
    }

    InitWindow(800, 450, "Raylib + Steamworks Ubuntu");
    SetTargetFPS(60);

    std::cout << "Logged in as Steam User: " << SteamFriends()->GetPersonaName() << "\n";

    while (!WindowShouldClose()) {
        SteamAPI_RunCallbacks();

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Steamworks Connected Successfully!", 20, 20, 20, GREEN);
            DrawText(TextFormat("Hello, %s", SteamFriends()->GetPersonaName()), 20, 60, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();
    SteamAPI_Shutdown();
    return 0;
}
