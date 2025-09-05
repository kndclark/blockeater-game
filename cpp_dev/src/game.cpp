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
        "Squareboi",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Seed for random numbers
    srand(time(NULL));

    // Create the player object
    Player player(100, SCREEN_HEIGHT / 2 - 20, 40, 40, 5);

    // Obstacle variables
    std::vector<Obstacle> obstacles;
    int obstacle_speed = 3;
    Uint32 last_spawn_time = 0;
    Uint32 spawn_interval = 1500; // milliseconds

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Handle player input
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        player.handle_input(keystate);

        // Periodically spawn new obstacles
        Uint32 current_time = SDL_GetTicks();
        if (current_time > last_spawn_time + spawn_interval) {
            last_spawn_time = current_time;
            int w = 20 + (rand() % 60); // random width
            int h = 20 + (rand() % 60); // random height
            int y = rand() % (SCREEN_HEIGHT - h); // random y position
            obstacles.emplace_back(SCREEN_WIDTH, y, w, h, obstacle_speed);
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
        for (const auto& obstacle : obstacles) {
            if (SDL_HasIntersection(&player.rect, &obstacle.rect)) {
                SDL_Log("Collision detected! Game Over.");
                running = false; // End the game on collision
                break;
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}