#pragma once

#include <SDL2/SDL.h>
#include <algorithm> // For std::max
#include "Color.h"

// TODO: give ability to deflect blocks (do this in a separate feature)

// --- Player Struct ---
// Encapsulates all data and behavior for the player character.
struct Player {
    SDL_Rect rect;
    int speed;
    Color color;
    const int default_w;
    const int default_h;
    static constexpr int MIN_SIZE = 20;

    Player(int x, int y, int w, int h, int s, Color c) : rect{x, y, w, h}, speed(s), color(c), default_w(w), default_h(h) {}

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
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    // Increases the player's size.
    void grow(int amount) {
        rect.w += amount;
        rect.h += amount;
    }

    // Decreases the player's size, but not below a minimum threshold.
    void shrink(int amount) {
        rect.w = std::max(MIN_SIZE, rect.w - amount);
        rect.h = std::max(MIN_SIZE, rect.h - amount);
    }

    // Resets the player to its original size.
    void resetSize() {
        rect.w = default_w;
        rect.h = default_h;
    }
};