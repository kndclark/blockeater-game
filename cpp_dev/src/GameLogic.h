#pragma once

#include <vector>
#include <SDL2/SDL.h> // For SDL_Log
#include <optional>   // For std::optional
#include "Obstacle.h" // For ObstacleType
#include "Player.h"   // For Player

// This function is deterministic and easily testable.
// It takes a roll (0-99) and returns an obstacle type based on percentages.
inline ObstacleType determineObstacleType(int percent_grow, int percent_shrink, int roll) {
    // Assumes the sum of percentages is <= 100.
    // The remainder is the chance for the Hurt obstacle.
    if (roll < percent_grow) {
        return ObstacleType::Grow;
    } else if (roll < percent_grow + percent_shrink) {
        return ObstacleType::Shrink;
    } else {
        return ObstacleType::Hurt;
    }
}

// Handles the game logic for a collision between the player and an obstacle.
// Modifies the player, the list of obstacles, and the game's running state by reference.
inline void handleCollision(Player& player, std::vector<Obstacle>::iterator& it, std::vector<Obstacle>& obstacles, bool& running) {
    switch (it->type) {
        case ObstacleType::Hurt:
            SDL_Log("Collision with Hurt obstacle! Game Over.");
            running = false; // End the game
            ++it; // Advance iterator to avoid re-processing in the game loop
            break;
        case ObstacleType::Grow:
            SDL_Log("Collision with Grow obstacle! Player grows.");
            player.grow(10);
            it = obstacles.erase(it); // Erase and get next valid iterator
            break;
        case ObstacleType::Shrink:
            SDL_Log("Collision with Shrink obstacle! Player shrinks.");
            player.shrink(10);
            it = obstacles.erase(it); // Erase and get next valid iterator
            break;
    }
}

// Calculates FPS when a second has passed.
// Returns the FPS value if an update is due, otherwise returns std::nullopt.
// Manages frame_count and last_fps_update_time by reference.
inline std::optional<float> calculateFps(Uint32& frame_count, Uint32& last_fps_update_time, Uint32 current_time) {
    frame_count++;
    if (current_time - last_fps_update_time >= 1000) {
        float fps = static_cast<float>(frame_count) / ((current_time - last_fps_update_time) / 1000.0f);
        frame_count = 0;
        last_fps_update_time = current_time;
        return fps;
    }
    return std::nullopt;
}

// Separates obstacles into batches for efficient rendering.
// This is a pure function that can be easily tested.
inline void prepareObstacleBatches(const std::vector<Obstacle>& obstacles,
                                   std::vector<SDL_Rect>& hurt_rects,
                                   std::vector<SDL_Rect>& grow_rects,
                                   std::vector<SDL_Rect>& shrink_rects) {
    hurt_rects.clear();
    grow_rects.clear();
    shrink_rects.clear();
    for (const auto& obstacle : obstacles) {
        switch (obstacle.type) {
            case ObstacleType::Hurt:   hurt_rects.push_back(obstacle.rect); break;
            case ObstacleType::Grow:   grow_rects.push_back(obstacle.rect); break;
            case ObstacleType::Shrink: shrink_rects.push_back(obstacle.rect); break;
        }
    }
}

// Renders all obstacles in batches, which is more efficient than individual draw calls.
inline void batchRenderObstacles(SDL_Renderer* renderer, const std::vector<Obstacle>& obstacles, const Config& config,
                                 std::vector<SDL_Rect>& hurt_rects, std::vector<SDL_Rect>& grow_rects, std::vector<SDL_Rect>& shrink_rects) {
    prepareObstacleBatches(obstacles, hurt_rects, grow_rects, shrink_rects);

    if (!hurt_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Hurt);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, hurt_rects.data(), static_cast<int>(hurt_rects.size()));
    }
    if (!grow_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Grow);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, grow_rects.data(), static_cast<int>(grow_rects.size()));
    }
    if (!shrink_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Shrink);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, shrink_rects.data(), static_cast<int>(shrink_rects.size()));
    }
}