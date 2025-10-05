// Simple 2D game window using SDL2
#include <SDL2/SDL.h>
#include <memory>  // For std::unique_ptr
#include <ctime>   // For time()
#include "../config/Config.h"
#include "GameState.h"
#include "GameLogic.h"

int main(int argc, char* argv[]) {
    struct SdlDeleter {
        void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
        void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    };

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Load configuration. The Config class will find the default config file.
    Config config;

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

    // Seed for random numbers
    srand(time(NULL));

    // --- Game State Setup ---
    GameState game_state(config, SCREEN_WIDTH, SCREEN_HEIGHT);

    // --- Framerate Control ---
    const int TARGET_FPS = config.getTargetFps();
    const Uint32 FRAME_DELAY = (TARGET_FPS > 0) ? 1000 / TARGET_FPS : 0;
    Uint32 frame_start_time;

    // --- Main Game Loop ---
    // The game will continue to run as long as this 'running' flag is true.
    while (game_state.running) {
        frame_start_time = SDL_GetTicks();

        gameLoopIteration(game_state, config);

        // Only render if the game is still running after the update phase
        if (game_state.running) {
            renderGame(renderer.get(), game_state, config);
        }

        // --- FPS Calculation and Capping ---
        Uint32 current_frametime = SDL_GetTicks();
        if (auto fps_opt = calculateFps(game_state.frame_count, game_state.last_fps_update_time, current_frametime)) {
            SDL_Log("FPS: %.2f", *fps_opt);
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    SDL_Quit();
    return 0;
}