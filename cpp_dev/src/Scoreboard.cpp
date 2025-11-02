#include "Scoreboard.h"
#include <stdexcept> // For std::runtime_error
#include <iomanip>   // For std::fixed, std::setprecision
#include <memory>    // For std::unique_ptr
#include <cmath>     // For std::round
#include "Player.h"  // For Player::DASH_COOLDOWN_MS

Scoreboard::Scoreboard(SDL_Renderer* renderer, const Config& config) : renderer_(renderer), config_(config) {
    font_.reset(TTF_OpenFont(config.getFontPath().c_str(), config.getFontSize()));
    if (!font_) {
        throw std::runtime_error("Failed to load font for scoreboard: " + std::string(TTF_GetError()));
    }
}

Scoreboard::~Scoreboard() = default;

void Scoreboard::render(int score, int level, int current_gap_size, int checkpoints_passed, int checkpoints_per_level, int player_size, bool on_cooldown, Uint32 cooldown_remaining) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    // --- Render Score ---
    std::string score_text = config_.getScorePrefix() + std::to_string(score);
    Color c = config_.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};

    // Create a temporary surface for the score text. A unique_ptr handles cleanup for us.
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> score_surface(TTF_RenderText_Solid(font_.get(), score_text.c_str(), color));
    if (!score_surface) {
        SDL_Log("Unable to create text surface for score: %s", TTF_GetError());
        return;
    }

    // Now create a texture from that surface. Textures are GPU-optimized.
    std::unique_ptr<SDL_Texture, SdlTextureDeleter> score_texture(SDL_CreateTextureFromSurface(renderer_, score_surface.get()));
    if (!score_texture) {
        SDL_Log("Unable to create texture from surface for score: %s", SDL_GetError());
        return;
    }

    // Define where on the screen to draw the score text.
    SDL_Rect score_dest_rect = {10, 10, score_surface->w, score_surface->h};
    SDL_RenderCopy(renderer_, score_texture.get(), nullptr, &score_dest_rect);

    // --- Render Level ---
    std::string level_text = getLevelText(level, checkpoints_passed, checkpoints_per_level);
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> level_surface(TTF_RenderText_Solid(font_.get(), level_text.c_str(), color));
    if (!level_surface) {
        SDL_Log("Unable to create text surface for level: %s", TTF_GetError());
        return;
    }

    std::unique_ptr<SDL_Texture, SdlTextureDeleter> level_texture(SDL_CreateTextureFromSurface(renderer_, level_surface.get()));
    if (!level_texture) {
        SDL_Log("Unable to create texture from surface for HUD: %s", SDL_GetError());
        return;
    }

    // Position the level text just below the score text.
    SDL_Rect level_dest_rect = {10, 10 + score_dest_rect.h + 5, level_surface->w, level_surface->h};
    // Copy the texture to the renderer.
    // - renderer_: Our game's main renderer.
    // - level_texture.get(): The texture we just made from the level text surface.
    // - nullptr: Use the entire texture as the source (no clipping).
    // - &level_dest_rect: The destination rectangle on the screen.
    SDL_RenderCopy(renderer_, level_texture.get(), nullptr, &level_dest_rect);

    // --- Render Next Gap Size ---
    std::string gap_text = config_.getGapSizePrefix() + std::to_string(current_gap_size);
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> gap_surface(TTF_RenderText_Solid(font_.get(), gap_text.c_str(), color));
    if (!gap_surface) {
        SDL_Log("Unable to create text surface for gap size: %s", TTF_GetError());
        return;
    }

    std::unique_ptr<SDL_Texture, SdlTextureDeleter> gap_texture(SDL_CreateTextureFromSurface(renderer_, gap_surface.get()));
    if (!gap_texture) {
        SDL_Log("Unable to create texture from surface for gap size: %s", SDL_GetError());
        return;
    }

    // Position the gap text just below the level text.
    SDL_Rect gap_dest_rect = {10, level_dest_rect.y + level_dest_rect.h + 5, gap_surface->w, gap_surface->h};
    // Copy the texture to the renderer.
    // - renderer_: Our game's main renderer.
    // - gap_texture.get(): The texture we just made from the gap text surface.
    // - nullptr: Use the entire texture as the source (no clipping).
    // - &gap_dest_rect: The destination rectangle on the screen.
    SDL_RenderCopy(renderer_, gap_texture.get(), nullptr, &gap_dest_rect);

    // --- Render Player Size ---
    std::string player_size_text = getPlayerSizeText(player_size, current_gap_size);
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> player_size_surface(TTF_RenderText_Solid(font_.get(), player_size_text.c_str(), color));
    if (!player_size_surface) {
        SDL_Log("Unable to create text surface for player size: %s", TTF_GetError());
        return;
    }

    std::unique_ptr<SDL_Texture, SdlTextureDeleter> player_size_texture(SDL_CreateTextureFromSurface(renderer_, player_size_surface.get()));
    if (!player_size_texture) {
        SDL_Log("Unable to create texture from surface for player size: %s", SDL_GetError());
        return;
    }

    // Position the player size text just below the gap text.
    SDL_Rect player_size_dest_rect = {10, gap_dest_rect.y + gap_dest_rect.h + 5, player_size_surface->w, player_size_surface->h};
    // Copy the texture to the renderer.
    SDL_RenderCopy(renderer_, player_size_texture.get(), nullptr, &player_size_dest_rect);

    renderDashStatus(on_cooldown, cooldown_remaining);

}

std::string Scoreboard::getLevelText(int level, int checkpoints_passed, int checkpoints_per_level) const {
    if (checkpoints_per_level <= 0) {
        return config_.getLevelPrefix() + std::to_string(level);
    }
    // If 5 checkpoints are needed, passing #4 means you are on 4/5, with 1 to go.
    // (4 % 5) = 4. 5 - 4 = 1.
    int checkpoints_in_level = checkpoints_passed % checkpoints_per_level;
    int checkpoints_to_next_level = checkpoints_per_level - checkpoints_in_level;
    return config_.getLevelPrefix() + std::to_string(level) +
           config_.getLevelProgressPrefix() + std::to_string(checkpoints_to_next_level) + config_.getLevelProgressSuffix();
}

std::string Scoreboard::getPlayerSizeText(int player_size, int gap_size) const {
    if (gap_size <= 0) {
        return config_.getPlayerSizePrefix() + "N/A";
    }
    int percentage = static_cast<int>((static_cast<double>(player_size) / gap_size) * 100.0);
    return config_.getPlayerSizePrefix() + std::to_string(percentage) +
           config_.getPlayerSizeSuffix();
}

std::string Scoreboard::getDashStatusText(bool on_cooldown, Uint32 cooldown_remaining) const {
    if (on_cooldown && cooldown_remaining > 0) {
        // Convert milliseconds to seconds with one decimal place.
        float seconds = static_cast<float>(cooldown_remaining) / 1000.0f;
        // Round to the nearest tenth of a second.
        int seconds_int = static_cast<int>(std::round(seconds * 10.0f));
        std::string time_str = std::to_string(seconds_int / 10) + "." + std::to_string(seconds_int % 10);
        return config_.getDashCooldownPrefix() + time_str + config_.getDashCooldownSuffix();
    } else {
        // Show "ready" if not on cooldown or if the cooldown has just expired.
        return config_.getDashReadyText();
    }
}

void Scoreboard::renderDashStatus(bool on_cooldown, Uint32 cooldown_remaining) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    Color c = config_.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};

    const int cooldown_indicator_radius = 12;
    const int cooldown_indicator_padding = 8;
    int dash_text_x_offset = 10;

    if (on_cooldown && cooldown_remaining > 0) {
        // Calculate cooldown progress (0.0 to 1.0)
        float progress = 1.0f - (static_cast<float>(cooldown_remaining) / static_cast<float>(Player::DASH_COOLDOWN_MS));

        // Position the circle in the bottom-left corner
        int circle_center_x = 10 + cooldown_indicator_radius;
        int circle_center_y = config_.getScreenHeight() - 10 - cooldown_indicator_radius;

        // Draw the circular loading bar
        drawCooldownCircle(progress, circle_center_x, circle_center_y, cooldown_indicator_radius, color);

        // Offset the text to be to the right of the circle
        dash_text_x_offset = circle_center_x + cooldown_indicator_radius + cooldown_indicator_padding;
    }
    std::string dash_text = getDashStatusText(on_cooldown, cooldown_remaining);
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> dash_surface(TTF_RenderText_Solid(font_.get(), dash_text.c_str(), color));
    if (!dash_surface) {
        SDL_Log("Unable to create text surface for dash status: %s", TTF_GetError());
        return;
    }

    std::unique_ptr<SDL_Texture, SdlTextureDeleter> dash_texture(SDL_CreateTextureFromSurface(renderer_, dash_surface.get()));
    if (!dash_texture) {
        SDL_Log("Unable to create texture from surface for dash status: %s", SDL_GetError());
        return;
    }

    SDL_Rect dash_dest_rect = {dash_text_x_offset, config_.getScreenHeight() - dash_surface->h - 10, dash_surface->w, dash_surface->h};
    SDL_RenderCopy(renderer_, dash_texture.get(), nullptr, &dash_dest_rect);
}

void Scoreboard::drawCooldownCircle(float progress, int x, int y, int radius, SDL_Color color) const {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    // The number of segments to draw for the circle. More segments = smoother circle.
    const int segments = 30;

    // Calculate points on the arc and draw them.
    // We draw points instead of lines to keep it simple and avoid visual artifacts
    // when the progress is very small or very large.
    for (int i = 0; i <= static_cast<int>(segments * progress); ++i) {
        // Start at -90 degrees (12 o'clock) and sweep clockwise.
        float angle = -M_PI / 2.0f + (static_cast<float>(i) / segments) * 2.0f * M_PI;
        int point_x = x + static_cast<int>(radius * std::cos(angle));
        int point_y = y + static_cast<int>(radius * std::sin(angle));
        SDL_RenderDrawPoint(renderer_, point_x, point_y);
    }
}