#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "../config/Config.h"

enum class AppStatus {
    ShowingMainMenu,
    Running,
    Restarting,
    Quitting,
    ShowingSettingsMenu
};

enum class GameOverAction {
    Restart,
    MainMenu,
    Quit
};

enum class PauseMenuAction {
    Resume,
    Restart,
    MainMenu,
    Quit
};

enum class MainMenuAction {
    StartGame,
    Settings,
    Quit
};

enum class SettingsMenuAction {
    ChangePlayerColor,
    ToggleFullscreen,
    Back
};

class MenuManager {
public:
    static MainMenuAction showMainMenu(SDL_Renderer* renderer, const Config& config);
    static SettingsMenuAction showSettingsMenu(SDL_Renderer* renderer, Config& config);
    static PauseMenuAction showPauseMenu(SDL_Renderer* renderer, const Config& config);
    static GameOverAction showGameOverScreen(SDL_Renderer* renderer, const Config& config, const std::string& message);
private:
    static void renderText(SDL_Renderer* renderer, const std::string& font_path, int font_size, const std::string& text,
                           SDL_Color color, int x, int y, bool center_on_x = true);
    static void renderColorPicker(SDL_Renderer* renderer, int x, int y, const std::vector<Color>& colors, int selection);
};