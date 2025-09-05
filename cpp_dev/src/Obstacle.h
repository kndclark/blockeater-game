#pragma once

#include <SDL2/SDL.h>

// --- Obstacle Struct ---
// Encapsulates all data and behavior for a single obstacle.
struct Obstacle {
    SDL_Rect rect;
    int speed;

    Obstacle(int x, int y, int w, int h, int s) {
        rect = {x, y, w, h};
        speed = s;
    }

    void update() {
        rect.x -= speed;
    }

    void draw(SDL_Renderer* renderer) const {
        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    bool is_offscreen() const {
        return rect.x + rect.w <= 0;
    }
};