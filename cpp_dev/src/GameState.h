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
        : player(config.getPlayerInitialX(),                               // initial x position
                 screen_height / 2 - config.getPlayerHeight() / 2,         // initial y position (centered)
                 config.getPlayerWidth(),                                  // width
                 config.getPlayerHeight(),                                 // height
                 config.getPlayerSpeed(),                                  // speed
                 config.getPlayerColor()),                                 // color
          spawner(config.getSpawnInterval(),                               // interval for regular obstacles
                  config.getCheckpointInterval(),                         // interval for checkpoints
                  screen_width,                                           // screen width
                  screen_height,                                          // screen height
                  OBSTACLE_SPEED,                                         // speed of obstacles
                  config.getGrowChance(),                                 // chance for a grow block
                  config.getShrinkChance(),                               // chance for a shrink block
                  config.getBaseCheckpointGap(),                          // base gap size for checkpoints
                  config.getGrowDimensions(),                             // dimensions for grow blocks
                  config.getShrinkDimensions(),                           // dimensions for shrink blocks
                  config.getHurtDimensions()),                            // dimensions for hurt blocks
          last_fps_update_time(SDL_GetTicks())                            // initialize FPS timer
    {}
};