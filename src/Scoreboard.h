#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "../config/Config.h"

/// @brief A data structure to hold all the information needed to render the scoreboard.
struct ScoreboardRenderData {
    int score;
    int level;
    int current_gap_size;
    int checkpoints_passed;
    int checkpoints_per_level;
    int player_size;
    bool on_cooldown;
    Uint32 cooldown_remaining;
    SizeBoostLevel last_boost_level;
    Uint32 time_since_boost;
    bool dash_boost_active;
    Uint32 time_since_dash_boost;
};

class Scoreboard {
public:
    Scoreboard(SDL_Renderer* renderer, const Config& config);
    ~Scoreboard();

    Scoreboard(const Scoreboard&) = delete;
    Scoreboard& operator=(const Scoreboard&) = delete;

    void render(const ScoreboardRenderData& data) const;

    // Generates the text for the level display. Made public for easier testing.
    std::string getLevelText(int level, int checkpoints_passed, int checkpoints_per_level) const;
    void renderDashStatus(bool on_cooldown, Uint32 cooldown_remaining, bool dash_boost_active, Uint32 time_since_dash_boost) const;
    std::string getPlayerSizeText(int player_size, int gap_size) const;
    std::string getDashStatusText(bool on_cooldown, Uint32 cooldown_remaining) const;

    // Public for testing
    Color getColorForSizeBoostTier(int player_width, int gap_size) const;
    Color getFlashColorForBoostMessage(SizeBoostLevel level, Uint32 time_since_boost) const;

    void drawCooldownCircle(float progress, int x, int y, int radius, SDL_Color color, std::vector<SDL_Point>* out_points = nullptr) const;

private:
    SDL_Renderer* renderer_;
    const Config& config_;

    struct SdlFontDeleter {
        void operator()(TTF_Font* f) const { if (f) TTF_CloseFont(f); }
    };

    std::unique_ptr<TTF_Font, SdlFontDeleter> font_;
};