// Simple 2D game window using SDL2
#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <algorithm>
#include "Player.h"
#include "Obstacle.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;

    SDL_Window* window = SDL_CreateWindow(
        "Squareboi",                  // const char* title: The title of the window
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
    Player player(100, SCREEN_HEIGHT / 2 - 20, 40, 40, 5);

    // Obstacle variables (defined in Obstacles.h)
    std::vector<Obstacle> obstacles;
    int obstacle_speed = 3;
    Uint32 last_spawn_time = 0;
    Uint32 spawn_interval = 1500; // milliseconds

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

        // Handle player input
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        player.handle_input(keystate, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Periodically spawn new obstacles
        Uint32 current_time = SDL_GetTicks();
        if (current_time > last_spawn_time + spawn_interval) {
            last_spawn_time = current_time;
            int w = 20 + (rand() % 40); // random width
            int h = 20 + (rand() % 40); // random height
            int y = rand() % (SCREEN_HEIGHT - h); // random y position

            // Randomly determine the type of obstacle to spawn
            int type_roll = rand() % 10; // 0-9
            ObstacleType type = ObstacleType::Hurt; // Default to Hurt
            if (type_roll < 2) { // 20% chance for Grow
                type = ObstacleType::Grow;
            } else if (type_roll < 4) { // 20% chance for Shrink
                type = ObstacleType::Shrink;
            } // 60% chance for Hurt
            obstacles.emplace_back(SCREEN_WIDTH, y, w, h, obstacle_speed, type);
        }

        // Update obstacle positions and remove off-screen ones
        for (auto& obstacle : obstacles) {
            obstacle.update();
        }
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(), 
                [](const Obstacle& o) { return o.is_offscreen(); }),
            obstacles.end());

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Dark gray
        SDL_RenderClear(renderer);

        // Draw all obstacles
        for (const auto& obstacle : obstacles) {
            obstacle.draw(renderer);
        }

        // Draw player
        player.draw(renderer);

        // Collision detection
        // We use an iterator-based loop so we can safely remove obstacles after collision.
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            if (SDL_HasIntersection(&player.rect, &it->rect)) {
                switch (it->type) {
                    case ObstacleType::Hurt:
                        SDL_Log("Collision with Hurt obstacle! Game Over.");
                        running = false; // End the game
                        ++it; // Advance iterator before breaking the outer loop
                        break;
                    case ObstacleType::Grow:
                        SDL_Log("Collision with Grow obstacle! Player grows.");
                        player.grow(10);
                        it = obstacles.erase(it); // Erase and get next iterator
                        break;
                    case ObstacleType::Shrink:
                        SDL_Log("Collision with Shrink obstacle! Player shrinks.");
                        player.shrink(10);
                        it = obstacles.erase(it); // Erase and get next iterator
                        break;
                }
            } else {
                ++it;
            }
            if (!running) break; // Exit loop immediately if game is over
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}