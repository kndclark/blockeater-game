#pragma once

#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include <cstdlib> // For rand()
#include <algorithm> // For std::remove_if
#include <tuple>

struct ObstacleSize {
    int w, h;
    bool operator==(const ObstacleSize& other) const {
        return w == other.w && h == other.h;
    }
};

// Define the different types of obstacles that can exist in the game.
enum class ObstacleType {
    Hurt,   // Standard obstacle that ends the game on collision.
    Grow,   // Power-up that makes the player bigger.
    Shrink, // Power-down that makes the player smaller.
    Checkpoint // A wall with a gap to pass through.
};

// This function takes a roll (0-99) and returns an obstacle type based on percentages.
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

class Config; // Forward declaration

// --- Obstacle Struct ---
// Encapsulates all data and behavior for a single obstacle.
struct Obstacle {
    SDL_Rect rect;
    // Checkpoints are composed of two rectangles.
    // We use std::optional to avoid allocating memory for non-checkpoint obstacles.
    std::optional<SDL_Rect> rect2;
    int speed;
    ObstacleType type;
    int points = 0;
    bool passed = false; // for checkpoints

    Obstacle(int x, int y, int w, int h, int s, ObstacleType t, int p = 0) {
        rect = {x, y, w, h};
        rect2 = std::nullopt;
        speed = s;
        type = t;
        points = p;
    }

    // Constructor for a checkpoint obstacle with two rectangles
    Obstacle(SDL_Rect r1, SDL_Rect r2, int s, int p = 0) {
        rect = r1;
        rect2 = r2;
        speed = s;
        type = ObstacleType::Checkpoint;
        points = p;
    }

    void update() {
        rect.x -= speed;
        if (rect2) {
            rect2->x -= speed;
        }
    }

    bool is_offscreen() const {
        return rect.x + rect.w <= 0;
    }

    static void updateAndRemove(std::vector<Obstacle>& obstacles) {
        for (auto& obstacle : obstacles) {
            obstacle.update();
        }
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(), [](const Obstacle& o) {
                return o.is_offscreen();
            }),
            obstacles.end()
        );
    }

    static Obstacle createCheckpoint(int screen_width, int screen_height, int speed, int gap_height, int points, const std::vector<Obstacle>& nearby_obstacles, int& out_gap_y);
    static int calculateSafeY(int screen_height, int entity_height, const std::vector<Obstacle>& nearby_obstacles, std::optional<int> gap_height = std::nullopt);

private:
    // Helper to select obstacle properties based on a random roll.
    // Kept private as it's an implementation detail of createRegular.
    static std::tuple<ObstacleType, ObstacleSize, int> getObstacleTypeAndSize(const Config& config);

public:
    static Obstacle createRegular(int screen_width, int screen_height, int speed,
                                  const Config& config,
                                  const std::vector<Obstacle>& nearby_obstacles);

};
