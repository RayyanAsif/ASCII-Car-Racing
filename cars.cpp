#include "cars.h"
#include <fstream>
#include <algorithm>

Entity::Entity(float x, float y, Texture2D tex, float sc)
    : texture(tex), scale(sc) {
    bounds = { x, y, (float)tex.width * sc, (float)tex.height * sc };
}

void Entity::Draw() const {
    DrawTexturePro(texture,
        { 0, 0, (float)texture.width, (float)texture.height },
        bounds,
        { 0, 0 }, 0, WHITE);
}

Rectangle Entity::GetBounds() const { return bounds; }

Rectangle Entity::GetCollisionBounds() const {
    float hReduction = 0.5f;
    float vReduction = 0.6f;
    return {
        bounds.x + bounds.width * hReduction / 2,
        bounds.y + bounds.height * vReduction / 2,
        bounds.width * (1 - hReduction),
        bounds.height * (1 - vReduction)
    };
}

void Entity::SetPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
}

Car::Car(float x, float y, Texture2D tex)
    : Entity(x, y, tex, PLAYER_SCALE), moveSpeed(800.f) {
}

void Car::MoveLeft(float delta) {
    bounds.x -= moveSpeed * delta;
}

void Car::MoveRight(float delta) {
    bounds.x += moveSpeed * delta;
}

void Car::Update(float delta) {}

Obstacle::Obstacle(float x, float y, Texture2D tex)
    : Entity(x, y, tex, OBSTACLE_SCALE), verticalSpeed(250.f), lane(0) {
}

void Obstacle::Update(float delta) {
    bounds.y += verticalSpeed * delta;
}

void Obstacle::IncreaseSpeed(float amount) {
    verticalSpeed += amount;
}

Game::Game(int screenWidth, int screenHeight, Texture2D carTex, Texture2D obstacleTex)
    : roadWidth(screenWidth * 0.6f),
    roadStartX((screenWidth - roadWidth) / 2),
    laneWidth(roadWidth / TOTAL_LANES),
    player(screenWidth / 2 - (carTex.width * Car::PLAYER_SCALE) / 2,
        screenHeight - carTex.height * Car::PLAYER_SCALE - 20,
        carTex),
    obstacleSpawnTimer(1.5f),
    lastBoostScore(-1),
    score(0.f),
    state(TITLE),
    finalScore(0),
    musicVolume(0.5f) {
    InitAudioDevice();
    bgMusic = LoadMusicStream("police.mp3");
    SetMusicVolume(bgMusic, musicVolume);
    PlayMusicStream(bgMusic);
    highScore = LoadHighScore();

}

Game::~Game() {
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
}

void Game::HandleInput() {
    if (IsKeyPressed(KEY_M)) {
        musicVolume = (musicVolume == 0.0f) ? 0.5f : 0.0f;
        SetMusicVolume(bgMusic, musicVolume);
    }

    switch (state) {
    case TITLE:
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE)) {
            state = PLAYING;
        }
        break;

    case PLAYING: {
        float delta = GetFrameTime();
        if (IsKeyDown(KEY_A)) player.MoveLeft(delta);
        if (IsKeyDown(KEY_D)) player.MoveRight(delta);

        player.SetPosition(
            Clamp(player.GetBounds().x, roadStartX, roadStartX + roadWidth - player.GetBounds().width),
            player.GetBounds().y
        );
        break;
    }

    case GAME_OVER:
        if (IsKeyPressed(KEY_R)) {
            Reset();
            state = PLAYING;
        }
        break;
    }
}

void Game::SpawnObstacle(Texture2D obstacleTex) {
    int lane = GetRandomValue(0, TOTAL_LANES - 1);
    float obstacleWidth = obstacleTex.width * Obstacle::OBSTACLE_SCALE;
    float x = roadStartX + (lane * laneWidth) + (laneWidth - obstacleWidth) / 2;
    obstacles.emplace_back(std::make_unique<Obstacle>(x, -100, obstacleTex));
}

void Game::Update(Texture2D obstacleTex) {
    UpdateMusicStream(bgMusic);

    if (state != PLAYING) return;

    float delta = GetFrameTime();
    score += delta;
    float spawnSpeedMultiplier = 1 + score * 0.02f;
    spawnSpeedMultiplier = std::min(spawnSpeedMultiplier, 2.0f); 
    obstacleSpawnTimer -= delta * spawnSpeedMultiplier;

    if (obstacleSpawnTimer <= 0) {
        SpawnObstacle(obstacleTex);
        
        obstacleSpawnTimer = INITIAL_SPAWN_INTERVAL - (score * 0.015f);
        if (obstacleSpawnTimer < MIN_SPAWN_INTERVAL)
            obstacleSpawnTimer = MIN_SPAWN_INTERVAL;
    }
    for (auto& obs : obstacles) {
        obs->Update(delta);
        if (CheckCollisionRecs(obs->GetCollisionBounds(), player.GetCollisionBounds())) {
            finalScore = static_cast<int>(score);
            if (finalScore > highScore) {
                highScore = finalScore;
                SaveHighScore(highScore);
            }
            state = GAME_OVER;

        }
    }

    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [](const auto& obs) { return obs->GetBounds().y > GetScreenHeight(); }),
        obstacles.end());

    int whole = static_cast<int>(score);
    if (whole % 4 == 0 && whole != lastBoostScore) {
        lastBoostScore = whole;
        for (auto& obs : obstacles) obs->IncreaseSpeed(whole * 0.6f);
    }
}
int Game::LoadHighScore() {
    std::ifstream in("highscore.txt");
    int hs = 0;
    if (in >> hs) return hs;
    return 0;
}

void Game::SaveHighScore(int hs) {
    std::ofstream out("highscore.txt");
    out << hs;
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (state) {
    case TITLE:
        DrawText("LAWLESS LANES", GetScreenWidth() / 2 - MeasureText("LAWLESS LANES", 50) / 2, 100, 50, MAROON);
        DrawText("Created by", GetScreenWidth() / 2 - MeasureText("Created by", 40) / 2, 180, 40, DARKGRAY);
        DrawText("Rayyan Dosani", GetScreenWidth() / 2 - MeasureText("Rayyan Dossani", 30) / 2, 240, 30, DARKBROWN);
        DrawText("Ali Hasnain", GetScreenWidth() / 2 - MeasureText("Ali Hasnain", 30) / 2, 280, 30, DARKBROWN);
        DrawText("Akbar Younsi", GetScreenWidth() / 2 - MeasureText("Akbar Younsi", 30) / 2, 320, 30, DARKBROWN);
        DrawText("Click to Play", GetScreenWidth() / 2 - MeasureText("Click to Play", 30) / 2, GetScreenHeight() - 100, 30, DARKGRAY);
        DrawText("Press M to toggle music", 10, GetScreenHeight() - 40, 20, DARKGRAY);
        break;

    case PLAYING:
        DrawRectangle(roadStartX, 0, roadWidth, GetScreenHeight(), DARKGRAY);
        for (int i = 1; i < TOTAL_LANES; i++) {
            float x = roadStartX + i * laneWidth;
            DrawLineEx({ x, 0 }, { x, (float)GetScreenHeight() }, 3.f, ColorAlpha(WHITE, 0.4f));
        }
        player.Draw();
        for (const auto& obs : obstacles) obs->Draw();
        DrawText(TextFormat("Score: %d", static_cast<int>(score)), 20, 20, 30, BLACK);
        DrawText("Press M to toggle music", 10, GetScreenHeight() - 40, 20, DARKGRAY);
        break;

    case GAME_OVER: {
        const char* gameOverText = "GAME OVER!";
        const char* scoreText = TextFormat("Final Score: %d", finalScore);

        const char* restartText = "Press R to restart";

        int textWidth = MeasureText(gameOverText, 50);
        DrawText(gameOverText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 80, 50, RED);

        textWidth = MeasureText(scoreText, 30);
        DrawText(scoreText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 20, 30, DARKGRAY);
        DrawText(TextFormat("High Score: %d", highScore),
            GetScreenWidth() / 2 - MeasureText(TextFormat("High Score: %d", highScore), 30) / 2,
            GetScreenHeight() / 2 + 20, 30, DARKGRAY);
        textWidth = MeasureText(restartText, 25);
        DrawText(restartText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 + 70, 25, DARKGRAY);
        break;
    }
    }

    EndDrawing();
}


void Game::Reset() {
    obstacles.clear();
    player.SetPosition(GetScreenWidth() / 2 - player.GetBounds().width / 2,
        GetScreenHeight() - player.GetBounds().height - 20);
    score = 0.f;
    lastBoostScore = -1;
}

GameState Game::GetState() const { return state; }