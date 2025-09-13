#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include "Player.h"
#include "Obstacle.h"
#include "../config/Config.h"
#include "GameLogic.h"

const int OBSTACLE_SPEED = 3;

struct GameState {
    Player player;
    std::vector<Obstacle> obstacles;
    int score = 0;
    ObstacleSpawner spawner;

    // Create vectors to hold rectangles for batch drawing. Reusing these vectors
    // each frame avoids repeated memory allocations.
    std::vector<SDL_Rect> hurt_rects;
    std::vector<SDL_Rect> grow_rects;
    std::vector<SDL_Rect> shrink_rects;
    std::vector<SDL_Rect> checkpoint_rects;

    // Game loop control
    bool running = true;

    // Framerate control
    Uint32 frame_count = 0;
    Uint32 last_fps_update_time = 0;

    GameState(const Config& config, int screen_width, int screen_height)
        : player(100, screen_height / 2 - 20, 40, 40, 5, config.getPlayerColor()),
          spawner(config.getSpawnInterval(), config.getCheckpointInterval(), screen_width, screen_height, OBSTACLE_SPEED, config.getGrowChance(), config.getShrinkChance(), config.getBaseCheckpointGap(),
                  config.getGrowDimensions(), config.getShrinkDimensions(), config.getHurtDimensions()),
          last_fps_update_time(SDL_GetTicks())
    {}
};