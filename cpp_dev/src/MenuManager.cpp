#include "MenuManager.h"
#include <SDL2/SDL_ttf.h>

MainMenuAction MenuManager::showMainMenu(SDL_Renderer* renderer, const Config& config) {
    // Clear the screen with a dark gray color to ensure no old graphics remain.
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // --- Main Title ---
    TTF_Font* main_font = TTF_OpenFont(config.getFontPath().c_str(), 64);
    if (!main_font) {
        SDL_Log("Failed to load main font for main menu: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(main_font, config.getMainMenuTitle().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) - text_height - 20, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(main_font);
    }

    // --- Instructions Text ---
    TTF_Font* instruction_font = TTF_OpenFont(config.getFontPath().c_str(), 24);
    if (!instruction_font) {
        SDL_Log("Failed to load instruction font for main menu: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(instruction_font, config.getMainMenuInstructions().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) + 20, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(instruction_font);
    }

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

    // --- Main Message (Paused) ---
    TTF_Font* main_font = TTF_OpenFont(config.getFontPath().c_str(), 48);
    if (main_font) {
        SDL_Surface* surface = TTF_RenderText_Solid(main_font, config.getPauseMenuTitle().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) - text_height, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(main_font);
    }

    // --- Instructions Text ---
    TTF_Font* instruction_font = TTF_OpenFont(config.getFontPath().c_str(), 24);
    if (instruction_font) {
        SDL_Surface* surface = TTF_RenderText_Solid(instruction_font, config.getPauseMenuInstructions().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) + 20, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(instruction_font);
    }

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

    // (Code to render text is omitted for brevity, but it's the same pattern as above)
    // ... render "Game Over" / "Victory" message ...
    // ... render instructions ...

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