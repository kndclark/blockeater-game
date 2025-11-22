#include "Player.h"

Player::Player(int x, int y, int w, int h, int s, Color c, float dash_mult, Uint32 dash_dur, Uint32 dash_cd)
    : rect{x, y, w, h}, speed(s), color(c), default_w(w), default_h(h),
      dash_speed_multiplier(dash_mult),
      dash_duration_ms(dash_dur),
      dash_cooldown_ms(dash_cd)
{}

void Player::update(Uint32 current_time) {
    // End the dash after its duration has passed
    if (is_dashing && (current_time - dash_start_time >= dash_duration_ms)) {
        is_dashing = false;
        on_cooldown = true;
        dash_cooldown_start_time = current_time;
    }
    // End the cooldown after its duration has passed
    if (on_cooldown && (current_time - dash_cooldown_start_time >= dash_cooldown_ms)) {
        on_cooldown = false;
    }
}

void Player::handle_input(const Uint8* keystate, int screen_width, int screen_height) {
    // Check for dash input. Can't dash if already dashing or on cooldown.
    if ((keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT]) && !is_dashing && !on_cooldown) {
        is_dashing = true;
        dash_start_time = SDL_GetTicks();
    }

    float current_speed = is_dashing ? static_cast<float>(speed) * dash_speed_multiplier : static_cast<float>(speed);

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
    rect.x = std::max(0, std::min(rect.x, screen_width - rect.w));
    rect.y = std::max(0, std::min(rect.y, screen_height - rect.h));
}

void Player::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Player::grow(int amount) {
    rect.w += amount;
    rect.h += amount;
}

void Player::shrink(int amount) {
    rect.w = std::max(MIN_SIZE, rect.w - amount);
    rect.h = std::max(MIN_SIZE, rect.h - amount);
}

void Player::resetSize() {
    rect.w = default_w;
    rect.h = default_h;
}

Uint32 Player::getDashCooldownRemaining() const {
    if (!on_cooldown) {
        return 0;
    }
    Uint32 elapsed = SDL_GetTicks() - dash_cooldown_start_time;
    if (elapsed >= dash_cooldown_ms) {
        return 0;
    }
    return dash_cooldown_ms - elapsed;
}