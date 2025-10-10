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

void Scoreboard::render(int score) const {
    // Custom deleters for SDL resources. These are simple structs that define
    // how to properly destroy a Surface or a Texture.
    struct SdlSurfaceDeleter {
        void operator()(SDL_Surface* s) const { if (s) SDL_FreeSurface(s); }
    };
    struct SdlTextureDeleter {
        void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
    };

    std::string score_text = "Score: " + std::to_string(score);
    SDL_Color color = {255, 255, 255, 255}; // White

    // The unique_ptr now owns the surface. It will be automatically freed
    // when 'surface' goes out of scope, even if the function exits early.
    std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> surface(TTF_RenderText_Solid(font_, score_text.c_str(), color));
    if (!surface) {
        SDL_Log("Unable to create text surface for score: %s", TTF_GetError());
        return;
    }

    std::unique_ptr<SDL_Texture, SdlTextureDeleter> texture(SDL_CreateTextureFromSurface(renderer_, surface.get()));
    if (!texture) {
        SDL_Log("Unable to create texture from surface for score: %s", SDL_GetError());
        // No need to free the surface manually, the unique_ptr will handle it
        return;
    }

    SDL_Rect destRect = {10, 10, surface->w, surface->h};
    SDL_RenderCopy(renderer_, texture.get(), nullptr, &destRect);
}