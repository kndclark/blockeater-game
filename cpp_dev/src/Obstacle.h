#pragma once

#include <SDL2/SDL.h>

// Define the different types of obstacles that can exist in the game.
enum class ObstacleType {
    Hurt,   // Standard obstacle that ends the game on collision.
    Grow,   // Power-up that makes the player bigger.
    Shrink  // Power-down that makes the player smaller.
};

// --- Obstacle Struct ---
// Encapsulates all data and behavior for a single obstacle.
struct Obstacle {
    SDL_Rect rect;
    int speed;
    ObstacleType type;

    Obstacle(int x, int y, int w, int h, int s, ObstacleType t) {
        rect = {x, y, w, h};
        speed = s;
        type = t;
    }

    void update() {
        rect.x -= speed;
    }

    // Draws the obstacle with a color corresponding to its type.
    void draw(SDL_Renderer* renderer) const {
        switch (type) {
            case ObstacleType::Hurt:
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // Red for Hurt
                break;
            case ObstacleType::Grow:
                SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255); // Green for Grow
                break;
            case ObstacleType::Shrink:
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange for Shrink
                break;
        }
        SDL_RenderFillRect(renderer, &rect);
    }

    bool is_offscreen() const {
        return rect.x + rect.w <= 0;
    }
};