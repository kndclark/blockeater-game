#pragma once

#include <vector>
#include <SDL2/SDL.h> // For SDL_Log and Uint32
#include <optional>   // For std::optional
#include <cstdlib> // For rand()

#include "Obstacle.h" // For ObstacleType
#include "Player.h"   // For Player

// Handles the game logic for a collision between the player and an obstacle.
// Modifies the player, the list of obstacles, and the game's running state by reference.
inline void handleCollision(Player& player, std::vector<Obstacle>::iterator& it, std::vector<Obstacle>& obstacles, bool& running) {
    switch (it->type) {
        case ObstacleType::Checkpoint:
            // fallthrough
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

// --- Obstacle Spawner ---
// Manages the logic and state for spawning obstacles over time.
struct ObstacleSpawner {
    Uint32 last_spawn_time = 0;
    const Uint32 spawn_interval;
    Uint32 last_checkpoint_spawn_time = 0;
    const Uint32 checkpoint_spawn_interval;

    const int screen_width;
    const int screen_height;
    const int obstacle_speed;
    const int grow_chance;
    const int shrink_chance;

    ObstacleSpawner(Uint32 regular_interval, Uint32 checkpoint_int, int width, int height, int speed, int grow, int shrink)
        : spawn_interval(regular_interval), checkpoint_spawn_interval(checkpoint_int),
          screen_width(width), screen_height(height), obstacle_speed(speed),
          grow_chance(grow), shrink_chance(shrink) {}

    // Checks the current time and spawns obstacles if their respective intervals have passed.
    void spawn_obstacles(Uint32 current_time, std::vector<Obstacle>& obstacles) {
        // Spawn regular obstacles
        if (current_time >= last_spawn_time + spawn_interval) {
            last_spawn_time = current_time;
            obstacles.push_back(Obstacle::createRegular(screen_width, screen_height, obstacle_speed, grow_chance, shrink_chance));
        }

        // Spawn checkpoints
        if (current_time >= last_checkpoint_spawn_time + checkpoint_spawn_interval) {
            last_checkpoint_spawn_time = current_time;
            obstacles.push_back(Obstacle::createCheckpoint(screen_width, screen_height, obstacle_speed));
        }
    }
};

// Handles scoring when a player passes a checkpoint.
inline void handleCheckpointPassing(const Player& player, Obstacle& obstacle, int& score) {
    if (obstacle.type == ObstacleType::Checkpoint && !obstacle.passed) {
        // Check if the player's front has passed the obstacle's back
        if (player.rect.x > obstacle.rect.x + obstacle.rect.w) {
            obstacle.passed = true;
            score += 10;
            SDL_Log("Checkpoint passed! Score: %d", score);
        }
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
inline void prepareObstacleBatches(const std::vector<Obstacle>& obstacles,
                                   std::vector<SDL_Rect>& hurt_rects,
                                   std::vector<SDL_Rect>& grow_rects,
                                   std::vector<SDL_Rect>& shrink_rects,
                                   std::vector<SDL_Rect>& checkpoint_rects) {
    hurt_rects.clear();
    grow_rects.clear();
    shrink_rects.clear();
    checkpoint_rects.clear();
    for (const auto& obstacle : obstacles) {
        switch (obstacle.type) {
            case ObstacleType::Hurt:   hurt_rects.push_back(obstacle.rect); break;
            case ObstacleType::Grow:   grow_rects.push_back(obstacle.rect); break;
            case ObstacleType::Shrink: shrink_rects.push_back(obstacle.rect); break;
            case ObstacleType::Checkpoint:
                checkpoint_rects.push_back(obstacle.rect);
                if (obstacle.rect2) {
                    checkpoint_rects.push_back(*obstacle.rect2);
                }
                break;
        }
    }
}

// Renders all obstacles in batches, which is more efficient than individual draw calls.
inline void batchRenderObstacles(SDL_Renderer* renderer, const std::vector<Obstacle>& obstacles, const Config& config,
                                 std::vector<SDL_Rect>& hurt_rects, std::vector<SDL_Rect>& grow_rects, std::vector<SDL_Rect>& shrink_rects,
                                 std::vector<SDL_Rect>& checkpoint_rects) {
    prepareObstacleBatches(obstacles, hurt_rects, grow_rects, shrink_rects, checkpoint_rects);

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
    if (!checkpoint_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Checkpoint);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, checkpoint_rects.data(), static_cast<int>(checkpoint_rects.size()));
    }
}