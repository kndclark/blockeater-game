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

    void render(int score, int level, int current_gap_size, int checkpoints_passed, int checkpoints_per_level) const;

    // Generates the text for the level display. Made public for easier testing.
    std::string getLevelText(int level, int checkpoints_passed, int checkpoints_per_level) const;

private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
};