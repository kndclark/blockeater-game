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

    // Dash ability state
    bool is_dashing = false;
    bool on_cooldown = false;
    Uint32 dash_start_time = 0;
    Uint32 dash_cooldown_start_time = 0;

    // Dash ability constants TODO move these to config.json (may shouldn't be consts? could modify via powerups, etc.)
    static constexpr float DASH_SPEED_MULTIPLIER = 2.5f;
    static constexpr Uint32 DASH_DURATION_MS = 500; // .5 seconds
    static constexpr Uint32 DASH_COOLDOWN_MS = 2000; // 2 seconds

    Player(int x, int y, int w, int h, int s, Color c) : rect{x, y, w, h}, speed(s), color(c), default_w(w), default_h(h) {}

    // Updates the player's state, like managing dash timers.
    void update() {
        // End the dash after its duration has passed
        if (is_dashing && (SDL_GetTicks() - dash_start_time >= DASH_DURATION_MS)) {
            is_dashing = false;
            on_cooldown = true;
            dash_cooldown_start_time = SDL_GetTicks();
        }
        // End the cooldown after its duration has passed
        if (on_cooldown && (SDL_GetTicks() - dash_cooldown_start_time >= DASH_COOLDOWN_MS)) {
            on_cooldown = false;
        }
    }

    void handle_input(const Uint8* keystate, int screen_width, int screen_height) {
        // Check for dash input. Can't dash if already dashing or on cooldown.
        if ((keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT]) && !is_dashing && !on_cooldown) {
            is_dashing = true;
            dash_start_time = SDL_GetTicks();
        }

        float current_speed = is_dashing ? static_cast<float>(speed) * DASH_SPEED_MULTIPLIER : static_cast<float>(speed);

        bool any_direction_pressed = keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_RIGHT] ||
                                     keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_DOWN];

        if (is_dashing && !any_direction_pressed) {
            // If dashing with no directional input, move forward.
            rect.x += static_cast<int>(current_speed);
        }

        // Regular movement or directional dash.
        if (keystate[SDL_SCANCODE_LEFT])  rect.x -= static_cast<int>(current_speed);
        if (keystate[SDL_SCANCODE_RIGHT]) rect.x += static_cast<int>(current_speed);
        if (keystate[SDL_SCANCODE_UP])    rect.y -= static_cast<int>(current_speed);
        if (keystate[SDL_SCANCODE_DOWN])  rect.y += static_cast<int>(current_speed);

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