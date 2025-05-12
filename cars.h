#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "raylib.h"

template<typename T>
T Clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

enum GameState { TITLE, PLAYING, GAME_OVER };

class Entity {
protected:
    Rectangle bounds;
    Texture2D texture;
    float scale;

public:
    Entity(float x, float y, Texture2D tex, float sc);
    virtual void Update(float delta) = 0;
    virtual void Draw() const;
    Rectangle GetBounds() const;
    Rectangle GetCollisionBounds() const;
    void SetPosition(float x, float y);
};

class Car : public Entity {
    float moveSpeed;

public:
    static constexpr float PLAYER_SCALE = 0.4f;

    Car(float x, float y, Texture2D tex);
    void MoveLeft(float delta);
    void MoveRight(float delta);
    void Update(float delta) override;
};

class Obstacle : public Entity {
public:
    static constexpr float OBSTACLE_SCALE = 0.4f;


private:
    float verticalSpeed;
    int lane;

public:
    Obstacle(float x, float y, Texture2D tex);
    void Update(float delta) override;
    void IncreaseSpeed(float amount);
};

class Game {
    static const int TOTAL_LANES = 3;
    float roadWidth;
    float roadStartX;
    float laneWidth;
    Car player;
    std::vector<std::unique_ptr<Obstacle>> obstacles;
    float obstacleSpawnTimer;
    int lastBoostScore;
    float score;
    GameState state;
    int finalScore;
    Music bgMusic;
    float musicVolume;
    static constexpr float MIN_SPAWN_INTERVAL = 0.7f;
	static constexpr float INITIAL_SPAWN_INTERVAL = 1.2f;
private:
    int highScore;  



public:
    Game(int screenWidth, int screenHeight, Texture2D carTex, Texture2D obstacleTex);
    ~Game();
    void HandleInput();
    void SpawnObstacle(Texture2D obstacleTex);
    void Update(Texture2D obstacleTex);
    void Draw() const;
    void Reset();
    GameState GetState() const;
    int LoadHighScore();
    void SaveHighScore(int hs);
};
