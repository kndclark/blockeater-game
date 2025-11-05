// Simple 2D game window using SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept> // For std::runtime_error
#include <memory>  // For std::unique_ptr
#include <ctime>   // For time()
#include <string>
#include "../config/Config.h"
#include "GameState.h"
#include "GameLogic.h"
#include "MenuManager.h"
#include "Scoreboard.h"
#include <sstream>
#include "Player.h"

int main(int argc, char* argv[]) {
    // RAII wrapper for SDL and TTF initialization
    struct SdlInitializer {
        SdlInitializer() {
            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                throw std::runtime_error(std::string("Unable to initialize SDL: ") + SDL_GetError());
            }
            if (TTF_Init() == -1) {
                throw std::runtime_error(std::string("Unable to initialize SDL_ttf: ") + TTF_GetError());
            }
        }
        ~SdlInitializer() {
            TTF_Quit();
            SDL_Quit();
        }
    };

    struct SdlDeleter {
        void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
        void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    };

    // Determine the project root path. The game executable is in `build/`.
    std::string root_path;
    char* base_path = SDL_GetBasePath();
    if (base_path) {
        root_path = std::string(base_path) + "../";
        SDL_free(base_path);
    } else {
        SDL_Log("Warning: Could not get application base path. Asset paths may be incorrect.");
        // Fallback to an empty root path.
    }

    try {
        SdlInitializer sdl_initializer;
        
    // Load configuration. The Config class will find the default config file.
    Config config(root_path);

    const int SCREEN_WIDTH = config.getScreenWidth();
    const int SCREEN_HEIGHT = config.getScreenHeight();

    std::unique_ptr<SDL_Window, SdlDeleter> window(SDL_CreateWindow(
        "THE BLOCKEATER",                  // const char* title: The title of the window
        SDL_WINDOWPOS_CENTERED,       // int x: Initial x position
        SDL_WINDOWPOS_CENTERED,       // int y: Initial y position
        SCREEN_WIDTH,                 // int w: Width of the window, in pixels
        SCREEN_HEIGHT,                // int h: Height of the window, in pixels
        SDL_WINDOW_FULLSCREEN_DESKTOP // Create a fullscreen window at the desktop resolution
    ));
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Creates context for 2D drawing operations (renderer) to be shown in the window.
    // Uses a back-buffer system: clear the screen, draw all your objects to a hidden
    // buffer, then "present" that buffer to the screen all at once to prevent flickering.
    std::unique_ptr<SDL_Renderer, SdlDeleter> renderer(SDL_CreateRenderer(
        window.get(),                 // The window to render to.
        -1,                           // The index of the rendering driver to initialize. -1 means to use the first one supporting the requested flags.
        SDL_RENDERER_ACCELERATED      // Flags: Use hardware-accelerated rendering (the GPU), which is much faster.
    ));

    // Check if the renderer was created successfully.
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    std::unique_ptr<Scoreboard> scoreboard;
    try {
        scoreboard = std::make_unique<Scoreboard>(renderer.get(), config);
    } catch (const std::exception& e) {
        SDL_Log("Error creating scoreboard: %s", e.what());
        return 1;
    }

    AppStatus app_status = AppStatus::ShowingMainMenu;
    while (app_status != AppStatus::Quitting) {
        switch (app_status) {
            case AppStatus::ShowingMainMenu: {
                MainMenuAction action = MenuManager::showMainMenu(renderer.get(), config);
                if (action == MainMenuAction::StartGame) {
                    app_status = AppStatus::Running;
                } else {
                    app_status = AppStatus::Quitting;
                }
                break;
            }
            case AppStatus::Running: {
                // Use a dedicated scope for game objects that depend on the config.
                // This ensures they are destroyed and re-created on restart.
                {
                    // Seed for random numbers
                    srand(time(NULL));
        
                    // --- Game State Setup --- (Use a unique_ptr to control lifetime)
                    auto game_state = std::make_unique<GameState>(config, SCREEN_WIDTH, SCREEN_HEIGHT);
        
                    // --- Main Game Loop ---
                    while (game_state->running) {
                        if (game_state->paused) {
                            PauseMenuAction action = MenuManager::showPauseMenu(renderer.get(), config);
                            handlePauseMenuAction(action, *game_state, app_status);
                        } else {
                            handleGameLoop(renderer.get(), *game_state, *scoreboard, config);
        
                            // Only check for victory if the game is still running after the update phase
                            if (game_state->running) {
                                checkVictoryCondition(*game_state);
                            }
                        }
                    }
                    // Only show the game over/victory screen if we haven't already decided to quit from the pause menu.
                    if (app_status == AppStatus::Running) { // If not quitting or restarting
                        // Display either the game over or victory screen, and wait for user action.
                        GameOverAction action = MenuManager::showGameOverScreen(renderer.get(), config, game_state->victory ? config.getVictoryText() : config.getGameOverText());
                        handleGameOverAction(action, app_status);
                    }
                }
                break;
            }
            case AppStatus::Restarting: {
                // Simply transition back to Running to start a new game loop.
                app_status = AppStatus::Running;
                break;
            }
            case AppStatus::Quitting:
                // Loop will terminate
                break;
        }
    }
    } catch (const std::exception& e) {
        SDL_Log("An error occurred: %s", e.what());
        return 1;
    }

    return 0;
}