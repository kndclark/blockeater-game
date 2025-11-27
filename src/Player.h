#pragma once

#include <SDL2/SDL.h>
#include <algorithm> // For std::max
#include <vector>
#include "Color.h"

// TODO: give ability to deflect blocks (do this in a separate feature)

// --- Player Struct ---

struct Ghost {
    SDL_Rect rect;
    Uint32 creation_time;
};

enum class PlayerState {
    Ready,
    Dashing,
    Cooldown
};

/// @brief A data structure to hold all the information needed to create a Player.
struct PlayerDef {
    int x, y, w, h, speed;
    Color color;
    float dash_speed_multiplier;
    Uint32 dash_duration_ms;
    Uint32 dash_cooldown_ms;
};

// Encapsulates all data and behavior for the player character.
struct Player {
    SDL_Rect rect;
    int speed;
    Color color;
    static constexpr int MIN_SIZE = 20;
    PlayerState state = PlayerState::Ready;

    // Dash ability state
    Uint32 dash_start_time = 0;
    Uint32 dash_cooldown_start_time = 0;

    // Dash ability parameters
    const int default_w;
    const int default_h;
    const float dash_speed_multiplier;
    const Uint32 dash_duration_ms;
    const Uint32 dash_cooldown_ms;

    // Ghosting effect state
    std::vector<Ghost> ghosts;
    Uint32 last_ghost_spawn_time = 0;

    // Ghosting effect parameters
    static constexpr Uint32 GHOST_LIFETIME_MS = 300;
    static constexpr Uint32 GHOST_SPAWN_INTERVAL_MS = 30;
    static constexpr Uint8 GHOST_INITIAL_ALPHA = 100;

    explicit Player(const PlayerDef& def);

    // Updates the player's state, like managing dash timers.
    void update(Uint32 current_time);
private:
    void updateReady(Uint32 current_time);
    void updateDashing(Uint32 current_time);
    void updateCooldown(Uint32 current_time);
public:
    void update_ghosts(Uint32 current_time);
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