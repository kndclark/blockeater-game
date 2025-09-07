#pragma once

#include <SDL2/SDL.h>
#include <optional>

// Define the different types of obstacles that can exist in the game.
enum class ObstacleType {
    Hurt,   // Standard obstacle that ends the game on collision.
    Grow,   // Power-up that makes the player bigger.
    Shrink, // Power-down that makes the player smaller.
    Checkpoint // A wall with a gap to pass through.
};

// --- Obstacle Struct ---
// Encapsulates all data and behavior for a single obstacle.
struct Obstacle {
    SDL_Rect rect;
    // Checkpoints are composed of two rectangles.
    // We use std::optional to avoid allocating memory for non-checkpoint obstacles.
    std::optional<SDL_Rect> rect2;
    int speed;
    ObstacleType type;
    bool passed = false; // for checkpoints

    Obstacle(int x, int y, int w, int h, int s, ObstacleType t) {
        rect = {x, y, w, h};
        rect2 = std::nullopt;
        speed = s;
        type = t;
    }

    // Constructor for a checkpoint obstacle with two rectangles
    Obstacle(SDL_Rect r1, SDL_Rect r2, int s) {
        rect = r1;
        rect2 = r2;
        speed = s;
        type = ObstacleType::Checkpoint;
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
};