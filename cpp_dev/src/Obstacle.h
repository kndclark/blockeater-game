#pragma once

#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include <cstdlib> // For rand()
#include <algorithm> // For std::remove_if

// Define the different types of obstacles that can exist in the game.
enum class ObstacleType {
    Hurt,   // Standard obstacle that ends the game on collision.
    Grow,   // Power-up that makes the player bigger.
    Shrink, // Power-down that makes the player smaller.
    Checkpoint // A wall with a gap to pass through.
};

// This function takes a roll (0-99) and returns an obstacle type based on percentages.
inline ObstacleType determineObstacleType(int percent_grow, int percent_shrink, int roll) {
    // Assumes the sum of percentages is <= 100.
    // The remainder is the chance for the Hurt obstacle.
    if (roll < percent_grow) {
        return ObstacleType::Grow;
    } else if (roll < percent_grow + percent_shrink) {
        return ObstacleType::Shrink;
    } else {
        return ObstacleType::Hurt;
    }
}

// --- Obstacle Struct ---
// Encapsulates all data and behavior for a single obstacle.
struct Obstacle {
    SDL_Rect rect;
    // Checkpoints are composed of two rectangles.
    // We use std::optional to avoid allocating memory for non-checkpoint obstacles.
    std::optional<SDL_Rect> rect2;
    int speed;
    ObstacleType type;
    bool passed = false; // for checkpoints

    Obstacle(int x, int y, int w, int h, int s, ObstacleType t) {
        rect = {x, y, w, h};
        rect2 = std::nullopt;
        speed = s;
        type = t;
    }

    // Constructor for a checkpoint obstacle with two rectangles
    Obstacle(SDL_Rect r1, SDL_Rect r2, int s) {
        rect = r1;
        rect2 = r2;
        speed = s;
        type = ObstacleType::Checkpoint;
    }

    void update() {
        rect.x -= speed;
        if (rect2) {
            rect2->x -= speed;
        }
    }

    bool is_offscreen() const {
        return rect.x + rect.w <= 0;
    }

    static Obstacle createCheckpoint(int screen_width, int screen_height, int speed, int gap_height) {
        // Spawn a checkpoint (a wall with a gap)
        const int gap_y = rand() % (screen_height - gap_height);
        const int checkpoint_width = 30;

        SDL_Rect top_rect = {screen_width, 0, checkpoint_width, gap_y};
        SDL_Rect bottom_rect = {screen_width, gap_y + gap_height, checkpoint_width, screen_height - (gap_y + gap_height)};

        return Obstacle(top_rect, bottom_rect, speed);
    }

    static Obstacle createGrowBlock(int screen_width, int screen_height, int speed) {
        int w = 40; // random width
        int h = 40; // random height
        int y = rand() % (screen_height - h); // random y position
        return Obstacle(screen_width, y, w, h, speed, ObstacleType::Grow);
    }

    static Obstacle createShrinkBlock(int screen_width, int screen_height, int speed) {
        int w = 20; // random width
        int h = 20; // random height
        int y = rand() % (screen_height - h); // random y position
        return Obstacle(screen_width, y, w, h, speed, ObstacleType::Shrink);
    }

    static Obstacle createHurtBlock(int screen_width, int screen_height, int speed) {
        int w = 20 + (rand() % 40); // random width
        int h = 20 + (rand() % 40); // random height
        int y = rand() % (screen_height - h); // random y position
        return Obstacle(screen_width, y, w, h, speed, ObstacleType::Hurt);
    }

    static Obstacle createRegular(int screen_width, int screen_height, int speed, int grow_chance, int shrink_chance) {
        int type_roll = rand() % 100; // Roll a number between 0 and 99
        ObstacleType type = determineObstacleType(grow_chance, shrink_chance, type_roll);

        switch (type) {
            case ObstacleType::Grow:   return createGrowBlock(screen_width, screen_height, speed);
            case ObstacleType::Shrink: return createShrinkBlock(screen_width, screen_height, speed);
            default:                   return createHurtBlock(screen_width, screen_height, speed);
        }
    }

    static void updateAndRemove(std::vector<Obstacle>& obstacles) {
        // This is more efficient than the erase-remove idiom as it avoids
        // shifting elements in the vector. It has O(N) complexity for one
        // pass, whereas erase-remove can be O(N^2) in the worst case if
        // many elements are removed.
        for (size_t i = 0; i < obstacles.size();) {
            obstacles[i].update();
            if (obstacles[i].is_offscreen()) {
                // Swap with the last element and pop back (O(1) on average)
                std::swap(obstacles[i], obstacles.back());
                obstacles.pop_back();
                // Do not increment i, as we need to process the new element at index i
            } else {
                // Move to the next obstacle
                ++i;
            }
        }
    }
};