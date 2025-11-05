#include "MenuManager.h"
#include <SDL2/SDL_ttf.h>

void MenuManager::renderText(SDL_Renderer* renderer, const std::string& font_path, int font_size, const std::string& text,
                           SDL_Color color, int x, int y, bool center_on_x) {
    TTF_Font* font = TTF_OpenFont(font_path.c_str(), font_size);
    if (!font) {
        SDL_Log("Failed to load font '%s': %s", font_path.c_str(), TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        SDL_Log("Failed to render text '%s': %s", text.c_str(), TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int text_width = surface->w;
    int text_height = surface->h;
    SDL_Rect dest_rect = { center_on_x ? (x - text_width / 2) : x, y, text_width, text_height };
    SDL_RenderCopy(renderer, texture, nullptr, &dest_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}

MainMenuAction MenuManager::showMainMenu(SDL_Renderer* renderer, const Config& config) {
    // Clear the screen with a dark gray color to ensure no old graphics remain.
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // Render title and instructions using the helper
    renderText(renderer, config.getFontPath(), 64, config.getMainMenuTitle(), color, screen_width / 2, screen_height / 2 - 84);
    renderText(renderer, config.getFontPath(), 24, config.getMainMenuInstructions(), color, screen_width / 2, screen_height / 2 + 20);

    SDL_RenderPresent(renderer);

    // --- Main Menu Event Loop ---
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return MainMenuAction::Quit;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_s: return MainMenuAction::StartGame;
                    case SDLK_q: case SDLK_ESCAPE: return MainMenuAction::Quit;
                    default: break;
                }
            }
        }
        SDL_Delay(100);
    }
}

PauseMenuAction MenuManager::showPauseMenu(SDL_Renderer* renderer, const Config& config) {
    // Draw a semi-transparent overlay to dim the background game state.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // ~60% opacity black
    SDL_Rect overlay_rect = {0, 0, config.getScreenWidth(), config.getScreenHeight()};
    SDL_RenderFillRect(renderer, &overlay_rect);

    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // Render title and instructions using the helper
    renderText(renderer, config.getFontPath(), 48, config.getPauseMenuTitle(), color, screen_width / 2, screen_height / 2 - 48);
    renderText(renderer, config.getFontPath(), 24, config.getPauseMenuInstructions(), color, screen_width / 2, screen_height / 2 + 20);

    SDL_RenderPresent(renderer);

    // --- Pause Menu Event Loop ---
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return PauseMenuAction::Quit;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: return PauseMenuAction::Resume;
                    case SDLK_r: return PauseMenuAction::Restart;
                    case SDLK_m: return PauseMenuAction::MainMenu;
                    case SDLK_q: return PauseMenuAction::Quit;
                    default: break;
                }
            }
        }
        SDL_Delay(100);
    }
}

GameOverAction MenuManager::showGameOverScreen(SDL_Renderer* renderer, const Config& config, const std::string& message) {
    // Draw a semi-transparent overlay to dim the background game state.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // ~60% opacity black
    SDL_Rect overlay_rect = {0, 0, config.getScreenWidth(), config.getScreenHeight()};
    SDL_RenderFillRect(renderer, &overlay_rect);

    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // Render "Game Over" / "Victory" message and instructions using the helper
    renderText(renderer, config.getFontPath(), 48, message, color, screen_width / 2, screen_height / 2 - 48);
    renderText(renderer, config.getFontPath(), 24, config.getGameOverInstructions(), color, screen_width / 2, screen_height / 2 + 20);


    SDL_RenderPresent(renderer);

    // --- Game Over Event Loop ---
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return GameOverAction::Quit;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_r: return GameOverAction::Restart;
                    case SDLK_m: return GameOverAction::MainMenu;
                    case SDLK_q: case SDLK_ESCAPE: return GameOverAction::Quit;
                    default: break;
                }
            }
        }
        SDL_Delay(100);
    }
}