#include "MenuManager.h"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <memory>

// A RAII-based helper class to manage font and texture rendering for menus.
// This avoids repeatedly opening/closing the font file.
class TextRenderer {
public:
    TextRenderer(SDL_Renderer* renderer, const std::string& font_path, int font_size)
        : renderer_(renderer) {
        font_.reset(TTF_OpenFont(font_path.c_str(), font_size));
        if (!font_) {
            SDL_Log("Failed to load font '%s': %s", font_path.c_str(), TTF_GetError());
            // Depending on how critical the font is, you might throw an exception here.
        }
    }

    void render(const std::string& text, SDL_Color color, int x, int y, bool center_on_x = true) {
        if (!font_) return; // Don't try to render if font failed to load.

        SDL_Surface* surface = TTF_RenderText_Solid(font_.get(), text.c_str(), color);
        if (!surface) {
            SDL_Log("Failed to render text '%s': %s", text.c_str(), TTF_GetError());
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (!texture) {
            SDL_Log("Failed to create texture for text '%s': %s", text.c_str(), SDL_GetError());
            SDL_FreeSurface(surface);
            return;
        }

        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { center_on_x ? (x - text_width / 2) : x, y, text_width, text_height };
        SDL_RenderCopy(renderer_, texture, nullptr, &dest_rect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

private:
    struct SdlFontDeleter {
        void operator()(TTF_Font* f) const { if (f) TTF_CloseFont(f); }
    };

    SDL_Renderer* renderer_;
    std::unique_ptr<TTF_Font, SdlFontDeleter> font_;
};

void MenuManager::renderText(SDL_Renderer* renderer, const std::string& font_path, int font_size, const std::string& text,
                           SDL_Color color, int x, int y, bool center_on_x) {
    // This function is now a simple wrapper. For more complex menus,
    // it would be better to create one TextRenderer and pass it around.
    TextRenderer(renderer, font_path, font_size).render(text, color, x, y, center_on_x);
}

void MenuManager::renderColorPicker(SDL_Renderer* renderer, int x, int y, const std::vector<Color>& colors, int selection) {
    const int box_size = 40;
    const int padding = 10;
    const int total_width = colors.size() * (box_size + padding) - padding;
    int start_x = x - total_width / 2;

    for (size_t i = 0; i < colors.size(); ++i) {
        SDL_Rect color_box = {start_x + static_cast<int>(i) * (box_size + padding), y, box_size, box_size};
        const auto& c = colors[i];
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &color_box);

        if (static_cast<int>(i) == selection) {
            // Draw a white border around the selected color
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &color_box);
        }
    }
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
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) return MainMenuAction::Quit;
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_s: return MainMenuAction::StartGame;
                case SDLK_e: return MainMenuAction::Settings;
                case SDLK_q: case SDLK_ESCAPE: return MainMenuAction::Quit;
                default: break;
            }
        }
    }
    // If SDL_WaitEvent fails, the loop will exit. Return a safe default.
    SDL_Log("SDL_WaitEvent failed in main menu: %s", SDL_GetError());
    return MainMenuAction::Quit;
}

SettingsMenuAction MenuManager::showSettingsMenu(SDL_Renderer* renderer, Config& config) {
    SDL_Event event;
    bool in_color_picker = false;
    int color_selection = 0;

    while (true) { // This loop is more complex, we'll adjust it carefully.
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        Color c = config.getUiTextColor();
        SDL_Color color = {c.r, c.g, c.b, c.a};
        const int screen_width = config.getScreenWidth();
        const int screen_height = config.getScreenHeight();

        if (in_color_picker) {
            renderText(renderer, config.getFontPath(), 48, "Choose Color", color, screen_width / 2, screen_height / 2 - 120);
            renderColorPicker(renderer, screen_width / 2, screen_height / 2 - 40, config.getPlayerColorChoices(), color_selection);
            renderText(renderer, config.getFontPath(), 24, "Left/Right to select, Enter to confirm, ESC to cancel", color, screen_width / 2, screen_height / 2 + 60);
        } else {
            renderText(renderer, config.getFontPath(), 48, config.getSettingsMenuTitle(), color, screen_width / 2, screen_height / 2 - 48);
            renderText(renderer, config.getFontPath(), 24, config.getSettingsMenuInstructions(), color, screen_width / 2, screen_height / 2 + 20);
        }

        SDL_RenderPresent(renderer);

        // Use SDL_WaitEvent to pause until an event is available.
        if (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) return SettingsMenuAction::Back;
            if (event.type == SDL_KEYDOWN) {
                if (in_color_picker) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (color_selection > 0) {
                                color_selection--;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (color_selection < static_cast<int>(config.getPlayerColorChoices().size()) - 1) {
                                color_selection++;
                            }
                            break;
                        case SDLK_RETURN: {
                            const auto& choices = config.getPlayerColorChoices();
                            if (!choices.empty() && color_selection < static_cast<int>(choices.size())) {
                                config.setPlayerColor(choices[color_selection]);
                            }
                            in_color_picker = false;
                            break;
                        }
                        case SDLK_ESCAPE: {
                            in_color_picker = false;
                            break;
                        }
                        default: break;
                        }
                } else {
                    switch (event.key.keysym.sym) {
                        case SDLK_c: in_color_picker = true; break;
                        case SDLK_t: return SettingsMenuAction::ToggleFullscreen;
                        case SDLK_b: case SDLK_ESCAPE: return SettingsMenuAction::Back;
                        default: break;
                    }
                }
            }
        } else {
            // SDL_WaitEvent failed, break the loop.
            break;
        }
    }
    // If the loop is broken (e.g., SDL_WaitEvent fails), return a safe default.
    SDL_Log("Event loop broke in settings menu. Returning to main menu.");
    return SettingsMenuAction::Back;
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
    while (SDL_WaitEvent(&event)) {
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
    // If SDL_WaitEvent fails, the loop will exit. Return a safe default.
    SDL_Log("SDL_WaitEvent failed in pause menu: %s", SDL_GetError());
    return PauseMenuAction::Quit;
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
    while (SDL_WaitEvent(&event)) {
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
    // If SDL_WaitEvent fails, the loop will exit. Return a safe default.
    SDL_Log("SDL_WaitEvent failed in game over screen: %s", SDL_GetError());
    return GameOverAction::Quit;
}