// Simple 2D game window using SDL2
#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <algorithm>
#include "Player.h"
#include "Obstacle.h"
#include "../config/Config.h"
#include "GameLogic.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Load configuration. The Config class will find the default config file.
    Config config;

    const int SCREEN_WIDTH = config.getScreenWidth();
    const int SCREEN_HEIGHT = config.getScreenHeight();

    SDL_Window* window = SDL_CreateWindow(
        "THE BLOCKEATER",                  // const char* title: The title of the window
        SDL_WINDOWPOS_CENTERED,       // int x: Initial x position
        SDL_WINDOWPOS_CENTERED,       // int y: Initial y position
        SCREEN_WIDTH,                 // int w: Width of the window, in pixels
        SCREEN_HEIGHT,                // int h: Height of the window, in pixels
        SDL_WINDOW_SHOWN              // Uint32 flags: Window state flags (e.g., shown, fullscreen)
    );
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Creates context for 2D drawing operations (renderer) to be shown in the window.
    // Uses a back-buffer system: clear the screen, draw all your objects to a hidden
    // buffer, then "present" that buffer to the screen all at once to prevent flickering.
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,                       // The window to render to.
        -1,                           // The index of the rendering driver to initialize. -1 means to use the first one supporting the requested flags.
        SDL_RENDERER_ACCELERATED      // Flags: Use hardware-accelerated rendering (the GPU), which is much faster.
    );

    // Check if the renderer was created successfully.
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        // If renderer creation fails, clean up the window we already created before quitting.
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Seed for random numbers
    srand(time(NULL));

    // Create the player object (defined in Player.h)
    Player player(100, SCREEN_HEIGHT / 2 - 20, 40, 40, 5, config.getPlayerColor());

    // Obstacle variables
    std::vector<Obstacle> obstacles;
    int obstacle_speed = 3;
    // Create vectors to hold rectangles for batch drawing. Reusing these vectors
    // each frame avoids repeated memory allocations.
    std::vector<SDL_Rect> hurt_rects;
    std::vector<SDL_Rect> grow_rects;
    std::vector<SDL_Rect> shrink_rects;
    std::vector<SDL_Rect> checkpoint_rects;

    // --- Game State ---
    int score = 0;

    // --- Obstacle Spawn Chances ---
    const int GROW_CHANCE_PERCENT = 40; 
    const int SHRINK_CHANCE_PERCENT = 40; 
    const int HURT_CHANCE_PERCENT = 20; 
    // Ensure that the chances for all obstacle types sum to 100%.
    static_assert(GROW_CHANCE_PERCENT + SHRINK_CHANCE_PERCENT + HURT_CHANCE_PERCENT == 100, "The sum of obstacle spawn chances must be 100.");

    // --- Spawner Setup ---
    const Uint32 SPAWN_INTERVAL = 1500; // milliseconds for regular obstacles
    const Uint32 CHECKPOINT_SPAWN_INTERVAL = 10000; // 10 seconds
    ObstacleSpawner spawner(SPAWN_INTERVAL, CHECKPOINT_SPAWN_INTERVAL, SCREEN_WIDTH, SCREEN_HEIGHT, obstacle_speed, GROW_CHANCE_PERCENT, SHRINK_CHANCE_PERCENT);

    // --- Framerate Control ---
    const int TARGET_FPS = config.getTargetFps();
    const Uint32 FRAME_DELAY = (TARGET_FPS > 0) ? 1000 / TARGET_FPS : 0;
    Uint32 frame_start_time;
    Uint32 frame_count = 0; // For FPS calculation
    Uint32 last_fps_update_time = SDL_GetTicks(); // For FPS calculation

    // --- Main Game Loop ---
    bool running = true; // This flag controls the main game loop.
    SDL_Event event;     // A variable to store event data (e.g., keyboard, mouse, window events).

    // The game will continue to run as long as this 'running' flag is true.
    while (running) {
        // This inner loop processes all pending events in SDL's event queue.
        while (SDL_PollEvent(&event)) {
            // Check if the event is a request to quit the application.
            if (event.type == SDL_QUIT) {
                // If the user closes the window, set 'running' to false to exit the main loop.
                running = false;
            }
        }

        frame_start_time = SDL_GetTicks();

        // Handle player input
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        player.handle_input(keystate, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Spawn new obstacles based on time
        Uint32 current_time = SDL_GetTicks();
        spawner.spawn_obstacles(current_time, obstacles);

        // Update obstacle positions and remove off-screen ones
        Obstacle::updateAndRemove(obstacles);

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Dark gray
        SDL_RenderClear(renderer);

        batchRenderObstacles(renderer, obstacles, config, hurt_rects, grow_rects, shrink_rects, checkpoint_rects);

        // Draw player
        player.draw(renderer);

        // Collision detection
        // We use an iterator-based loop so we can safely remove obstacles after collision.
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            bool collision_detected = SDL_HasIntersection(&player.rect, &it->rect);
            // For checkpoints, check collision with the second rectangle as well.
            if (it->rect2) {
                collision_detected = collision_detected || SDL_HasIntersection(&player.rect, &*(it->rect2));
            }

            if (collision_detected) {
                handleCollision(player, it, obstacles, running);
            } else {
                handleCheckpointPassing(player, *it, score);
                ++it;
            }
            if (!running) break; // Exit loop immediately if game is over
        }

        SDL_RenderPresent(renderer);

        // --- FPS Calculation and Capping ---
        Uint32 current_frametime = SDL_GetTicks();
        if (auto fps_opt = calculateFps(frame_count, last_fps_update_time, current_frametime)) {
            SDL_Log("FPS: %.2f", *fps_opt);
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start_time;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}