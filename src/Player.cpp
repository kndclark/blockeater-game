#include "Player.h"

Player::Player(int x, int y, int w, int h, int s, Color c, float dash_mult, Uint32 dash_dur, Uint32 dash_cd)
    : rect{x, y, w, h}, speed(s), color(c),
      default_w(w), default_h(h),
      dash_speed_multiplier(dash_mult),
      dash_duration_ms(dash_dur),
      dash_cooldown_ms(dash_cd)
{}

void Player::update(Uint32 current_time) {
    switch (state) {
        case PlayerState::Ready:
            updateReady(current_time);
            break;
        case PlayerState::Dashing:
            updateDashing(current_time);
            break;
        case PlayerState::Cooldown:
            updateCooldown(current_time);
            break;
    }

    update_ghosts(current_time);
}

void Player::updateReady(Uint32 current_time) {
    // Nothing to do in the Ready state for now.
}

void Player::updateDashing(Uint32 current_time) {
    if (current_time - dash_start_time >= dash_duration_ms) {
        state = PlayerState::Cooldown;
        dash_cooldown_start_time = current_time;
        ghosts.clear(); // Clear any remaining ghosts when dash ends
    }
}

void Player::updateCooldown(Uint32 current_time) {
    if (current_time - dash_cooldown_start_time >= dash_cooldown_ms) {
        state = PlayerState::Ready;
    }
}

void Player::update_ghosts(Uint32 current_time) {
    // Spawn new ghosts if dashing
    if (state == PlayerState::Dashing && (current_time - last_ghost_spawn_time > GHOST_SPAWN_INTERVAL_MS)) {
        ghosts.push_back({rect, current_time});
        last_ghost_spawn_time = current_time;
    }

    // Remove old ghosts that have faded out
    ghosts.erase(
        std::remove_if(ghosts.begin(), ghosts.end(), [current_time](const Ghost& g) {
            return current_time - g.creation_time > GHOST_LIFETIME_MS;
        }),
        ghosts.end()
    );

    // If not dashing, ensure ghosts fade out and are eventually cleared.
    if (state != PlayerState::Dashing && ghosts.empty()) {
        // All ghosts have faded out, nothing more to do.
    }
}

void Player::handle_input(const Uint8* keystate, int screen_width, int screen_height) {
    // Check for dash input. Can't dash if already dashing or on cooldown.
    if ((keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT]) && state == PlayerState::Ready) {
        state = PlayerState::Dashing;
        dash_start_time = SDL_GetTicks();
    }

    float current_speed = (state == PlayerState::Dashing) ? static_cast<float>(speed) * dash_speed_multiplier : static_cast<float>(speed);

    bool any_direction_pressed = keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_RIGHT] ||
                                 keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_DOWN];

    if (state == PlayerState::Dashing && !any_direction_pressed) {
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

    // --- Draw Ghosts ---
    if (!ghosts.empty()) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Enable alpha blending
        Uint32 current_time = SDL_GetTicks();
        for (const auto& ghost : ghosts) {
            float age_ratio = static_cast<float>(current_time - ghost.creation_time) / GHOST_LIFETIME_MS;
            Uint8 alpha = static_cast<Uint8>(GHOST_INITIAL_ALPHA * (1.0f - std::min(1.0f, age_ratio)));
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
            SDL_RenderFillRect(renderer, &ghost.rect);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Disable alpha blending
    }

    // --- Draw Player ---
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
    if (state != PlayerState::Cooldown) {
        return 0;
    }
    Uint32 elapsed = SDL_GetTicks() - dash_cooldown_start_time;
    if (elapsed >= dash_cooldown_ms) {
        return 0;
    }
    return dash_cooldown_ms - elapsed;
}