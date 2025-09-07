#pragma once

#include <SDL2/SDL.h>
#include <algorithm> // For std::max

// --- Player Struct ---
// Encapsulates all data and behavior for the player character.
struct Player {
    SDL_Rect rect;
    int speed;

    Player(int x, int y, int w, int h, int s) {
        rect = {x, y, w, h};
        speed = s;
    }

    void handle_input(const Uint8* keystate, int screen_width, int screen_height) {
        if (keystate[SDL_SCANCODE_LEFT])  rect.x -= speed;
        if (keystate[SDL_SCANCODE_RIGHT]) rect.x += speed;
        if (keystate[SDL_SCANCODE_UP])    rect.y -= speed;
        if (keystate[SDL_SCANCODE_DOWN])  rect.y += speed;

        // Clamp player position to stay within screen bounds.
        // The player's x position cannot be less than 0.
        rect.x = std::max(0, rect.x);
        // The player's right edge cannot be past the screen width.
        rect.x = std::min(rect.x, screen_width - rect.w);
        // The player's y position cannot be less than 0.
        rect.y = std::max(0, rect.y);
        // The player's bottom edge cannot be past the screen height.
        rect.y = std::min(rect.y, screen_height - rect.h);
    }

    void draw(SDL_Renderer* renderer) const {
        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255); // Purple
        SDL_RenderFillRect(renderer, &rect);
    }

    // Increases the player's size.
    void grow(int amount) {
        rect.w += amount;
        rect.h += amount;
    }

    // Decreases the player's size, but not below a minimum threshold.
    void shrink(int amount) {
        const int min_size = 10;
        rect.w = std::max(min_size, rect.w - amount);
        rect.h = std::max(min_size, rect.h - amount);
    }
};