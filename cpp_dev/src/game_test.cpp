// Simple 2D game window using SDL2
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Squareboi",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Add player variables
    int player_x = 320, player_y = 240;
    int player_w = 40, player_h = 40;
    int player_speed = 5;

    // Add obstacle variables
    SDL_Rect obstacle = {200, 150, 60, 60};

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Keyboard state for movement
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_LEFT])  player_x -= player_speed;
        if (keystate[SDL_SCANCODE_RIGHT]) player_x += player_speed;
        if (keystate[SDL_SCANCODE_UP])    player_y -= player_speed;
        if (keystate[SDL_SCANCODE_DOWN])  player_y += player_speed;

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Dark gray
        SDL_RenderClear(renderer);

        // Draw obstacle
        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_RenderFillRect(renderer, &obstacle);

        // Draw player as a red rectangle
        SDL_Rect player_rect = {player_x, player_y, player_w, player_h};
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &player_rect);

        // Collision detection
        if (SDL_HasIntersection(&player_rect, &obstacle)) {
            // Simple collision response: move player back
            if (keystate[SDL_SCANCODE_LEFT])  player_x += player_speed;
            if (keystate[SDL_SCANCODE_RIGHT]) player_x -= player_speed;
            if (keystate[SDL_SCANCODE_UP])    player_y += player_speed;
            if (keystate[SDL_SCANCODE_DOWN])  player_y -= player_speed;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}