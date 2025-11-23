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

    // Dash ability parameters
    const float dash_speed_multiplier;
    const Uint32 dash_duration_ms;
    const Uint32 dash_cooldown_ms;

    Player(int x, int y, int w, int h, int s, Color c, float dash_mult, Uint32 dash_dur, Uint32 dash_cd);

    // Updates the player's state, like managing dash timers.
    void update(Uint32 current_time);
    void handle_input(const Uint8* keystate, int screen_width, int screen_height);
    void draw(SDL_Renderer* renderer) const;

    // Increases the player's size.
    void grow(int amount);

    // Decreases the player's size, but not below a minimum threshold.
    void shrink(int amount);

    // Resets the player to its original size.
    void resetSize();

    // Returns the remaining dash cooldown in milliseconds.
    Uint32 getDashCooldownRemaining() const;
};