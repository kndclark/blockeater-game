#include "Scoreboard.h"
#include <stdexcept> // For std::runtime_error
#include <memory>    // For std::unique_ptr

Scoreboard::Scoreboard(SDL_Renderer* renderer, const std::string& font_path, int font_size)
    : renderer_(renderer) {
    font_ = TTF_OpenFont(font_path.c_str(), font_size);
    if (!font_) {
        throw std::runtime_error("Failed to load font for scoreboard: " + std::string(TTF_GetError()));
    }
}

Scoreboard::~Scoreboard() {
    if (font_) {
        TTF_CloseFont(font_);
    }
}

void Scoreboard::render(int score, int level) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    // --- Render Score ---
    std::string score_text = "Score: " + std::to_string(score);
    SDL_Color color = {255, 255, 255, 255}; // White

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
    std::string level_text = "Level: " + std::to_string(level);
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
}