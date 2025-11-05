#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "../config/Config.h"

enum class AppStatus {
    ShowingMainMenu,
    Running,
    Restarting,
    Quitting
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
    Quit
};

class MenuManager {
public:
    static MainMenuAction showMainMenu(SDL_Renderer* renderer, const Config& config);
    static PauseMenuAction showPauseMenu(SDL_Renderer* renderer, const Config& config);
    static GameOverAction showGameOverScreen(SDL_Renderer* renderer, const Config& config, const std::string& message);
};