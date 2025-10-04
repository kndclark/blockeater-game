#pragma once

#include <vector>
#include <SDL2/SDL.h> // For SDL_Log and Uint32
#include <optional>   // For std::optional
#include <numeric>    // For std::accumulate
#include <cstdlib> // For rand()

#include "Obstacle.h" // For ObstacleType
#include "Player.h"   // For Player
#include "LevelManager.h"

struct GameState; // Forward declaration

/// Handles the game logic for a collision between the player and an obstacle.
/// @return The iterator to the next element to be processed.
std::vector<Obstacle>::iterator handleCollision(Player& player, std::vector<Obstacle>::iterator it, std::vector<Obstacle>& obstacles, bool& running, int player_size_change_amount);

// --- Obstacle Spawner ---
// Manages the logic and state for spawning obstacles over time.
struct ObstacleSpawner {
    const LevelManager& level_manager;
    Uint32 last_spawn_time = 0;
    Uint32 last_checkpoint_spawn_time = 0;
    const Uint32 checkpoint_spawn_interval;

    const int screen_width;
    const int screen_height;
    const int player_size_change_amount;
    const ObstacleSize grow_dims;
    const ObstacleSize shrink_dims;
    const ObstacleSize hurt_dims;
    // Track power-ups to influence checkpoint gap size. The values are dummy
    // values; only the count of elements matters.
    std::vector<int> shrink_powerups_since_checkpoint;
    ObstacleSpawner(const LevelManager& lm, Uint32 checkpoint_int, int width, int height, int size_change,
                      ObstacleSize gd, ObstacleSize sd, ObstacleSize hd)
        : level_manager(lm), checkpoint_spawn_interval(checkpoint_int),
          screen_width(width), screen_height(height),
          player_size_change_amount(size_change),
          grow_dims(gd), shrink_dims(sd), hurt_dims(hd) {}
    
    // Calculates the gap size for the next checkpoint based on power-ups collected.
    int calculateCheckpointGapSize() const;

    // Checks the current time and spawns obstacles if their respective intervals have passed.
    void spawn_obstacles(Uint32 current_time, std::vector<Obstacle>& obstacles);
};

// Handles scoring when a player passes a checkpoint.
void handleCheckpointPassing(Player& player, Obstacle& obstacle, GameState& game_state);

// Calculates FPS when a second has passed.
// Returns the FPS value if an update is due, otherwise returns std::nullopt.
// Manages frame_count and last_fps_update_time by reference.
std::optional<float> calculateFps(Uint32& frame_count, Uint32& last_fps_update_time, Uint32 current_time);

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
void batchRenderObstacles(SDL_Renderer* renderer, const std::vector<Obstacle>& obstacles, const Config& config,
                                 std::vector<SDL_Rect>& hurt_rects, std::vector<SDL_Rect>& grow_rects, std::vector<SDL_Rect>& shrink_rects,
                                 std::vector<SDL_Rect>& checkpoint_rects);