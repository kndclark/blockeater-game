#pragma once

#include <vector>
#include <SDL2/SDL.h> // For SDL_Log and Uint32
#include <optional>   // For std::optional
#include <numeric>    // For std::accumulate
#include <cstdlib> // For rand()

#include "Obstacle.h" // For ObstacleType
#include "Player.h"   // For Player
#include "LevelManager.h"
#include "Scoreboard.h"

struct GameState; // Forward declaration needed for handleCheckpointPassing
class Config;     // Forward declaration for batchRenderObstacles
/// Handles the game logic for a collision between the player and an obstacle.
std::vector<Obstacle>::iterator handleCollision(GameState& game_state, std::vector<Obstacle>::iterator it, std::vector<Obstacle>& obstacles);

// --- Obstacle Spawner ---
// Manages the logic and state for spawning obstacles over time.
struct ObstacleSpawner {
    const LevelManager& level_manager;
    Uint32 last_spawn_time = 0;
    Uint32 last_checkpoint_spawn_time = 0;
    const Uint32 checkpoint_safe_zone_duration;
    std::vector<Obstacle> nearby_obstacles;
    Obstacle* last_checkpoint = nullptr;
    // Track power-ups to influence checkpoint gap size. The values are dummy
    // values; only the count of elements matters.
    std::vector<int> shrink_powerups_since_checkpoint;
    ObstacleSpawner(const LevelManager& lm, Uint32 safe_zone_duration, int width, int height, int size_change);
    
    // Calculates the gap size for the next checkpoint based on power-ups collected.
    int calculateCheckpointGapSize() const;

    // Checks the current time and spawns obstacles if their respective intervals have passed.
    void spawn_obstacles(Uint32 current_time, GameState& game_state);

private:
    const int screen_width;
    const int screen_height;
    const int player_size_change_amount;
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

/// @brief Processes all pending SDL events and player keyboard input.
void processInput(GameState& game_state, const int SCREEN_WIDTH, const int SCREEN_HEIGHT);

/// @brief Updates the state of all game objects and handles game logic.
void updateGame(GameState& game_state);

/// @brief Renders all game objects to the screen.
void renderGame(SDL_Renderer* renderer, const GameState& game_state, const Config& config);

enum class GameOverAction {
    Restart,
    MainMenu,
    Quit
};

enum class PauseMenuAction {
    Resume,
    Restart,
    MainMenu,
    Quit
};

/// @brief Displays the game over screen and waits for user input.
/// @return The action selected by the user.
GameOverAction showGameOverScreen(SDL_Renderer* renderer, const Config& config, const std::string& message);

/// @brief Displays the pause menu and waits for user input.
/// @return The action selected by the user.
PauseMenuAction showPauseMenu(SDL_Renderer* renderer, const Config& config);

/// @brief Handles the action selected from the pause menu, updating game state flags.
void handlePauseMenuAction(PauseMenuAction action, GameState& game_state, bool& restart_requested, bool& app_is_running);

/// @brief Checks if the victory condition has been met and updates the game state accordingly.
void checkVictoryCondition(GameState& game_state);

/// @brief Runs a single iteration of the main game loop, handling input, updates, rendering, and frame capping.
void handleGameLoop(SDL_Renderer* renderer, GameState& game_state, Scoreboard& scoreboard, const Config& config);