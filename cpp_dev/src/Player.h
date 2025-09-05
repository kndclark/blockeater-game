#pragma once

#include <SDL2/SDL.h>

// --- Player Struct ---
// Encapsulates all data and behavior for the player character.
struct Player {
    SDL_Rect rect;
    int speed;

    Player(int x, int y, int w, int h, int s) {
        rect = {x, y, w, h};
        speed = s;
    }

    void handle_input(const Uint8* keystate) {
        if (keystate[SDL_SCANCODE_LEFT])  rect.x -= speed;
        if (keystate[SDL_SCANCODE_RIGHT]) rect.x += speed;
        if (keystate[SDL_SCANCODE_UP])    rect.y -= speed;
        if (keystate[SDL_SCANCODE_DOWN])  rect.y += speed;
    }

    void draw(SDL_Renderer* renderer) const {
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
};