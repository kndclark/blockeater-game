#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <stdexcept>

class Scoreboard {
public:
    Scoreboard(SDL_Renderer* renderer, const std::string& font_path, int font_size)
        : renderer_(renderer) {
        font_ = TTF_OpenFont(font_path.c_str(), font_size);
        if (!font_) {
            throw std::runtime_error("Failed to load font for scoreboard: " + std::string(TTF_GetError()));
        }
    }

    ~Scoreboard() {
        if (font_) {
            TTF_CloseFont(font_);
        }
    }

    // Delete copy constructor and assignment operator to prevent copying.
    Scoreboard(const Scoreboard&) = delete;
    Scoreboard& operator=(const Scoreboard&) = delete;

    void render(int score) const {
        std::string score_text = "Score: " + std::to_string(score);
        SDL_Color color = {255, 255, 255, 255}; // White

        SDL_Surface* surface = TTF_RenderText_Solid(font_, score_text.c_str(), color);
        if (!surface) {
            SDL_Log("Unable to create text surface for score: %s", TTF_GetError());
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_Rect destRect = {10, 10, surface->w, surface->h}; // Position at (10, 10)
        SDL_RenderCopy(renderer_, texture, NULL, &destRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
};