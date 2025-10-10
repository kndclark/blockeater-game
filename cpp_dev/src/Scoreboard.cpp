#include "Scoreboard.h"
#include <stdexcept> // For std::runtime_error
#include <memory>    // For std::unique_ptr

Scoreboard::Scoreboard(SDL_Renderer* renderer, const Config& config)
    : renderer_(renderer), config_(config) {
    font_ = TTF_OpenFont(config.getFontPath().c_str(), config.getFontSize());
    if (!font_) {
        throw std::runtime_error("Failed to load font for scoreboard: " + std::string(TTF_GetError()));
    }
}

Scoreboard::~Scoreboard() {
    if (font_) {
        TTF_CloseFont(font_);
    }
}

void Scoreboard::render(int score, int level, int current_gap_size, int checkpoints_passed, int checkpoints_per_level, int player_size) const {
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
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> score_surface(TTF_RenderText_Solid(font_, score_text.c_str(), color));
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
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> level_surface(TTF_RenderText_Solid(font_, level_text.c_str(), color));
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
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> gap_surface(TTF_RenderText_Solid(font_, gap_text.c_str(), color));
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
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> player_size_surface(TTF_RenderText_Solid(font_, player_size_text.c_str(), color));
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