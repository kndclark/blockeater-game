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

    bool app_is_running = true;
    while (app_is_running) {
        // Use a dedicated scope for game objects that depend on the config.
        // This ensures they are destroyed and re-created on restart.
        {
            // Seed for random numbers
            srand(time(NULL));

            // --- Framerate Control ---
            const int TARGET_FPS = config.getTargetFps();
            const Uint32 FRAME_DELAY = (TARGET_FPS > 0) ? 1000 / TARGET_FPS : 0;
            Uint32 frame_start_time;

            // --- Game State Setup --- (Use a unique_ptr to control lifetime)
            auto game_state = std::make_unique<GameState>(config, SCREEN_WIDTH, SCREEN_HEIGHT);

            // --- Main Game Loop ---
            while (game_state->running) {
                frame_start_time = SDL_GetTicks();

                processInput(*game_state, SCREEN_WIDTH, SCREEN_HEIGHT);

                if (game_state->paused) {
                    PauseMenuAction action = showPauseMenu(renderer.get(), config);
                    switch (action) {
                        case PauseMenuAction::Resume:
                            game_state->paused = false;
                            break;
                        case PauseMenuAction::MainMenu:
                            SDL_Log("Main Menu selected. Exiting for now.");
                            game_state->running = false; // This will break the inner loop
                            app_is_running = false;    // This will break the outer loop
                            break;
                        case PauseMenuAction::Quit:
                            game_state->running = false;
                            app_is_running = false;
                            break;
                    }
                } else {
                    updateGame(*game_state);
                }

                // Only render if the game is still running after the update phase
                if (game_state->running) {
                    renderGame(renderer.get(), *game_state, config);
                    scoreboard->render(game_state->score, game_state->level, game_state->ui_next_checkpoint_gap_size,
                                       game_state->checkpoints_passed_in_level, game_state->level_manager.getCheckpointsPerLevel(),
                                       game_state->player.rect.w, game_state->player.on_cooldown, game_state->player.getDashCooldownRemaining());
                    SDL_RenderPresent(renderer.get());
                }

                // Only check for victory if the game is still running after the update phase
                if (game_state->running && !game_state->paused) {
                    if (game_state->level > game_state->level_manager.getMaxLevel()) {
                        game_state->victory = true;
                        game_state->running = false; // End the game
                    }
                }
                // --- FPS Calculation and Capping ---
                if (auto fps_opt = calculateFps(game_state->frame_count, game_state->last_fps_update_time, SDL_GetTicks())) {
                    SDL_Log("FPS: %.2f", *fps_opt);
                }

                Uint32 frame_time = SDL_GetTicks() - frame_start_time;
                if (frame_time < FRAME_DELAY) {
                    SDL_Delay(FRAME_DELAY - frame_time);
                                }
            }
            // Only show the game over/victory screen if we haven't already decided to quit from the pause menu.
            if (app_is_running) {
                // Display either the game over or victory screen, and wait for user action.
                GameOverAction action = showGameOverScreen(renderer.get(), config, game_state->victory ? config.getVictoryText() : config.getGameOverText());

                if (action == GameOverAction::Quit) {
                    app_is_running = false;
                } else if (action == GameOverAction::MainMenu) {
                    // For now, main menu also quits.
                    SDL_Log("Main Menu selected. Exiting for now.");
                    app_is_running = false;
                }
                // If action is Restart, the outer while loop will simply continue, re-creating the GameState.
            }
        }
    }
    } catch (const std::exception& e) {
        SDL_Log("An error occurred: %s", e.what());
        return 1;
    }

    return 0;
}