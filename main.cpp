#include "cars.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Advanced Racing Attack");
    SetTargetFPS(60);

    Texture2D carTex = LoadTexture("Audi.png");
    Texture2D obstacleTex = LoadTexture("Police.png");

    Game game(screenWidth, screenHeight, carTex, obstacleTex);

    while (!WindowShouldClose()) {
        game.HandleInput();
        if (game.GetState() == PLAYING) {
            game.Update(obstacleTex);
        }
        game.Draw();
    }

    UnloadTexture(carTex);
    UnloadTexture(obstacleTex);
    CloseWindow();
    return 0;
}