#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class Scoreboard {
public:
    Scoreboard(SDL_Renderer* renderer, const std::string& font_path, int font_size);
    ~Scoreboard();

    // Delete copy constructor and assignment operator to prevent copying.
    Scoreboard(const Scoreboard&) = delete;
    Scoreboard& operator=(const Scoreboard&) = delete;

    void render(int score, int level, int next_gap_size) const;

private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
};