#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include "Player.h"
#include "Obstacle.h"
#include "../config/Config.h"
#include "LevelManager.h"
#include "GameLogic.h"
#include "ScoreManager.h"

struct GameState {
    const Config& config;
    Player player;
    std::vector<Obstacle> obstacles;
    int score = 0;
    int level = 1; 
    int checkpoints_passed_in_level = 0;
    int checkpoints_passed = 0;
    int ui_next_checkpoint_gap_size = 0;
    int next_checkpoint_gap_size = 0;
    LevelManager level_manager;
    ScoreManager score_manager;
    ObstacleSpawner spawner;

    // Create vectors to hold rectangles for batch drawing. Reusing these vectors
    // each frame avoids repeated memory allocations.
    std::vector<SDL_Rect> hurt_rects;
    std::vector<SDL_Rect> grow_rects;
    std::vector<SDL_Rect> shrink_rects;
    std::vector<SDL_Rect> checkpoint_rects;

    // Game loop control
    bool running = true;
    bool victory = false;

    // Framerate control
    Uint32 frame_count = 0;
    Uint32 last_fps_update_time = 0;

    GameState(const Config& config, int screen_width, int screen_height);

};