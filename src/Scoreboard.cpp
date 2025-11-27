#include "Scoreboard.h"
#include <stdexcept> // For std::runtime_error
#include <iomanip>   // For std::fixed, std::setprecision
#include <algorithm> // For std::find_if
#include <memory>    // For std::unique_ptr
#include "Player.h"  // For Player::DASH_COOLDOWN_MS

Scoreboard::Scoreboard(SDL_Renderer* renderer, const Config& config) : renderer_(renderer), config_(config) {
    font_.reset(TTF_OpenFont(config.getFontPath().c_str(), config.getFontSize()));
    if (!font_) {
        throw std::runtime_error("Failed to load font for scoreboard: " + std::string(TTF_GetError()));
    }
}

Scoreboard::~Scoreboard() = default;

void Scoreboard::render(const ScoreboardRenderData& data) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    // --- Render Score ---
    std::string score_text = config_.getScorePrefix() + std::to_string(data.score);
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

    // --- Render Dash Boost Message ---
    if (data.dash_boost_active) {
        Color boost_color_struct = getFlashColorForBoostMessage(SizeBoostLevel::Perfect, data.time_since_dash_boost); // Reuse rainbow flash
        SDL_Color boost_color = {boost_color_struct.r, boost_color_struct.g, boost_color_struct.b, boost_color_struct.a};

        std::string boost_text = "Dash boost!";
        std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> boost_surface(TTF_RenderText_Solid(font_.get(), boost_text.c_str(), boost_color));
        if (!boost_surface) {
            SDL_Log("Unable to create text surface for dash boost message: %s", TTF_GetError());
            return;
        }

        std::unique_ptr<SDL_Texture, SdlTextureDeleter> boost_texture(SDL_CreateTextureFromSurface(renderer_, boost_surface.get()));
        if (!boost_texture) {
            SDL_Log("Unable to create texture from surface for dash boost message: %s", SDL_GetError());
            return;
        }

        // Position the dash boost message to the right of the score.
        SDL_Rect boost_dest_rect = {score_dest_rect.x + score_dest_rect.w + 10, score_dest_rect.y, boost_surface->w, boost_surface->h};
        SDL_RenderCopy(renderer_, boost_texture.get(), nullptr, &boost_dest_rect);
    }

    // --- Render Level ---
    std::string level_text = getLevelText(data.level, data.checkpoints_passed, data.checkpoints_per_level);
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
    // - renderer_: game's main renderer.
    // - level_texture.get(): The texture we just made from the level text surface.
    // - nullptr: Use the entire texture as the source (no clipping).
    // - &level_dest_rect: The destination rectangle on the screen.
    SDL_RenderCopy(renderer_, level_texture.get(), nullptr, &level_dest_rect);

    // --- Render Next Gap Size ---
    // This now displays the player's size as a percentage of the upcoming gap.
    Color gap_text_color_struct = getColorForSizeBoostTier(data.player_size, data.current_gap_size);
    SDL_Color gap_text_color = {gap_text_color_struct.r, gap_text_color_struct.g, gap_text_color_struct.b, gap_text_color_struct.a};

    std::string gap_text = getPlayerSizeText(data.player_size, data.current_gap_size);
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> gap_surface(TTF_RenderText_Solid(font_.get(), gap_text.c_str(), gap_text_color));
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
    SDL_RenderCopy(renderer_, gap_texture.get(), nullptr, &gap_dest_rect);

    renderDashStatus(data.on_cooldown, data.cooldown_remaining, data.dash_boost_active, data.time_since_dash_boost);

    // --- Render Size Boost Message ---
    if (data.last_boost_level != SizeBoostLevel::None) {
        Color boost_color_struct = getFlashColorForBoostMessage(data.last_boost_level, data.time_since_boost);
        SDL_Color boost_color = {boost_color_struct.r, boost_color_struct.g, boost_color_struct.b, boost_color_struct.a};

        std::string boost_text = config_.getSizeBoostText(data.last_boost_level);
        std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> boost_surface(TTF_RenderText_Solid(font_.get(), boost_text.c_str(), boost_color));
        if (!boost_surface) {
            SDL_Log("Unable to create text surface for boost message: %s", TTF_GetError());
            return;
        }

        std::unique_ptr<SDL_Texture, SdlTextureDeleter> boost_texture(SDL_CreateTextureFromSurface(renderer_, boost_surface.get()));
        if (!boost_texture) {
            SDL_Log("Unable to create texture from surface for boost message: %s", SDL_GetError());
            return;
        }

        // Position the boost text to the right of the gap size text.
        SDL_Rect boost_dest_rect = {gap_dest_rect.x + gap_dest_rect.w + 10, gap_dest_rect.y, boost_surface->w, boost_surface->h};
        SDL_RenderCopy(renderer_, boost_texture.get(), nullptr, &boost_dest_rect);
    }

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
        return config_.getGapSizePrefix() + "N/A";
    }
    int percentage = static_cast<int>(std::round((static_cast<double>(player_size) / gap_size) * 100.0));
    return config_.getGapSizePrefix() + std::to_string(percentage) +
           config_.getGapSizeSuffix();
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

Color Scoreboard::getColorForSizeBoostTier(int player_width, int gap_size) const {
    if (gap_size <= 0) {
        return config_.getUiTextColor();
    }

    const float raw_size_percentage = (static_cast<float>(player_width) / gap_size) * 100.0f;
    const int rounded_percentage = static_cast<int>(std::round(raw_size_percentage));
    const auto& tiers = config_.getSizeBoostTiers();

    auto tier_it = std::find_if(tiers.cbegin(), tiers.cend(), [rounded_percentage](const auto& tier) {
        return rounded_percentage >= tier.threshold_percent;
    });

    if (tier_it != tiers.cend()) {
        SizeBoostLevel level = SizeBoostLevel::None;
        if (tier_it->tier == "Perfect") level = SizeBoostLevel::Perfect;
        else if (tier_it->tier == "Great") level = SizeBoostLevel::Great;
        else if (tier_it->tier == "Good") level = SizeBoostLevel::Good;

        if (level != SizeBoostLevel::None) {
            return config_.getSizeBoostTierColor(level);
        }
    }

    return config_.getUiTextColor();
}

Color Scoreboard::getFlashColorForBoostMessage(SizeBoostLevel level, Uint32 time_since_boost) const {
    if (level == SizeBoostLevel::Perfect) {
        const auto& rainbow_colors = config_.getRainbowColors();
        if (rainbow_colors.empty()) return config_.getUiTextColor();
        const Uint32 rainbow_cycle_time = 50; // ms per color
        int color_index = ((time_since_boost - 1) / rainbow_cycle_time) % rainbow_colors.size();
        return rainbow_colors[color_index];
    }

    // For Good and Great, flash between tier color and default UI color
    const Uint32 flash_interval = 150; // ms
    bool use_tier_color = (time_since_boost / flash_interval) % 2 == 0;

    if (use_tier_color) {
        return config_.getSizeBoostTierColor(level);
    } else {
        return config_.getUiTextColor();
    }
}

void Scoreboard::renderDashStatus(bool on_cooldown, Uint32 cooldown_remaining, bool dash_boost_active, Uint32 time_since_dash_boost) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    const int cooldown_indicator_padding = 8;
    int dash_text_x_offset = 10;

    if (on_cooldown && cooldown_remaining > 0) {
        // Calculate cooldown progress (0.0 to 1.0)
        float progress = 1.0f - (static_cast<float>(cooldown_remaining) / static_cast<float>(config_.getDashCooldownMs()));

        const int radius = config_.getCooldownIndicatorRadius();
        Color c = config_.getCooldownIndicatorColor();
        SDL_Color circle_color = {c.r, c.g, c.b, c.a};

        // Position the circle in the bottom-left corner
        int circle_center_x = 10 + radius;
        int circle_center_y = config_.getScreenHeight() - 10 - radius;

        // Draw the circular loading bar
        drawCooldownCircle(progress, circle_center_x, circle_center_y, radius, circle_color, nullptr);

        // Offset the text to be to the right of the circle
        dash_text_x_offset = circle_center_x + radius + cooldown_indicator_padding;
    }
    std::string dash_text = getDashStatusText(on_cooldown, cooldown_remaining);
    
    Color text_color_struct = config_.getUiTextColor();
    if (dash_boost_active) {
        text_color_struct = getFlashColorForBoostMessage(SizeBoostLevel::Perfect, time_since_dash_boost); // Reuse rainbow flash
    }
    SDL_Color color = {text_color_struct.r, text_color_struct.g, text_color_struct.b, text_color_struct.a};

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

void Scoreboard::drawCooldownCircle(float progress, int x, int y, int radius, SDL_Color color, std::vector<SDL_Point>* out_points) const {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);

    // The number of segments to draw for the circle. More segments = smoother circle.
    const int segments = 30;
    const int num_points_to_draw = static_cast<int>(segments * progress) + 1;

    std::vector<SDL_Point> points;
    if (out_points) {
        out_points->clear();
        points.reserve(num_points_to_draw);
    }

    // Calculate points on the arc and draw them.
    for (int i = 0; i < num_points_to_draw; ++i) {
        // Start at -90 degrees (12 o'clock) and sweep clockwise.
        float angle = -M_PI / 2.0f + (static_cast<float>(i) / segments) * 2.0f * M_PI;
        points.push_back({x + static_cast<int>(radius * std::cos(angle)),
                          y + static_cast<int>(radius * std::sin(angle))});
    }

    SDL_RenderDrawPoints(renderer_, points.data(), static_cast<int>(points.size()));

    if (out_points) {
        *out_points = points;
    }
}