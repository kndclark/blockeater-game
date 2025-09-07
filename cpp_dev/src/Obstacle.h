#pragma once

#include <SDL2/SDL.h>
#include "Color.h"

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
    Color color;
    ObstacleType type;

    Obstacle(int x, int y, int w, int h, int s, ObstacleType t, Color c) {
        rect = {x, y, w, h};
        speed = s;
        type = t;
        color = c;
    }

    void update() {
        rect.x -= speed;
    }

    // Draws the obstacle with a color corresponding to its type.
    void draw(SDL_Renderer* renderer) const {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    bool is_offscreen() const {
        return rect.x + rect.w <= 0;
    }
};