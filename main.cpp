#include "raylib.h"
#include <bits/stdc++.h>
#include "scripts/objects.cpp"

using namespace std;

const int windowHeight = 450;
const int windowWidth = 800;

int main() {
    InitWindow(windowWidth, windowHeight, "Something Cool");

    int display = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(display) / 2, GetMonitorHeight(display) / 2);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.offset = (Vector2){ windowWidth / 2.0f, windowHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Player player1, player2, player3, player4;
    player1.Create(
        "Afnan",
        "resources/animation/red_base_spritesheet.png",  // texture / animation source
        0.0f, 0.0f,                                      // position
        0.2f,                                            // size
        10.0f,                                           // speed
        5,                                               // frame count (1 = no animation)
        "resources/sprites/playerShadow.png"             // shadow path (nullptr = no shadow)
    );
    player2.Create(
        "Kazi",
        "resources/animation/blue_base_spritesheet.png",
        100.0f, 100.0f,
        0.2f,
        10.0f,
        5,
        "resources/sprites/playerShadow.png"
    );
    player3.Create(
        "Rakin", "resources/animation/green_base_spritesheet.png", 200.0f, 200.0f, 0.2f, 10.0f, 5, "resources/sprites/playerShadow.png");
    player4.Create(
        "Shonamoni",
        "resources/animation/pink_base_spritesheet.png",  // texture / animation source
        0.0f, 0.0f,                                      // position
        0.2f,                                            // size
        10.0f,                                           // speed
        5,                                               // frame count (1 = no animation)
        "resources/sprites/playerShadow.png"             // shadow path (nullptr = no shadow)
    );

    vector players = { &player1, &player2, &player3, &player4 };

    int cur = 0;
    Player* currentPlayer = players[cur];

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_TAB)) {
            if (cur < players.size() - 1) cur++;
            else cur = 0;
            currentPlayer = players[cur];
        }

        bool moving = IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S);

        float dx = 0.0f, dy = 0.0f;
        if (IsKeyDown(KEY_A)) { dx -= 1.0f; currentPlayer->flipLeft(); }
        if (IsKeyDown(KEY_D)) { dx += 1.0f; currentPlayer->flipRight(); }
        if (IsKeyDown(KEY_W)) dy -= 1.0f;
        if (IsKeyDown(KEY_S)) dy += 1.0f;

        if (dx != 0.0f && dy != 0.0f) {
            const float invSqrt2 = 0.70710678f;
            dx *= invSqrt2;
            dy *= invSqrt2;
        }

        currentPlayer->x += dx * currentPlayer->speed;
        currentPlayer->y += dy * currentPlayer->speed;

        currentPlayer->Animate(moving);

        camera.target = (Vector2){ currentPlayer->x, currentPlayer->y };

        sort(players.begin(), players.end(), [](Player* a, Player* b) { return a->y < b->y; });
        BeginDrawing();
            ClearBackground(GRAY);

            BeginMode2D(camera);
                for (int i = 0; i < players.size(); i++) {
                    players[i]->Draw();
                }
            EndMode2D();

        EndDrawing();
    }

    for (Player* p : players) {
        p->Unload();
    }
    CloseWindow();
    return 0;
}
