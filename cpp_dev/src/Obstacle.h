#pragma once

#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include <cstdlib> // For rand()
#include <algorithm> // For std::remove_if

struct ObstacleSize {
    int w, h;
    bool operator==(const ObstacleSize& other) const {
        return w == other.w && h == other.h;
    }
};

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

    static void updateAndRemove(std::vector<Obstacle>& obstacles) {
        for (auto& obstacle : obstacles) {
            obstacle.update();
        }
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(), [](const Obstacle& o) {
                return o.is_offscreen();
            }),
            obstacles.end()
        );
    }

    static Obstacle createCheckpoint(int screen_width, int screen_height, int speed, int gap_height, int& out_gap_y) {
        // Spawn a checkpoint (a wall with a gap)
        out_gap_y = rand() % (screen_height - gap_height);
        const int checkpoint_width = 30;

        SDL_Rect top_rect = {screen_width, 0, checkpoint_width, out_gap_y};
        SDL_Rect bottom_rect = {screen_width, out_gap_y + gap_height, checkpoint_width, screen_height - (out_gap_y + gap_height)};

        return Obstacle(top_rect, bottom_rect, speed);
    }

    static int calculateSafeY(int screen_height, int obstacle_height, const std::optional<std::pair<int, int>>& gap) {
        if (!gap.has_value()) {
            return rand() % (screen_height - obstacle_height);
        }

        const auto& [gap_top, gap_h] = gap.value();
        int gap_bottom = gap_top + gap_h;

        // Define the two possible spawn areas: above the gap and below the gap
        int top_area_height = gap_top;
        int bottom_area_height = screen_height - gap_bottom;

        // If there's no space to spawn, return a default value (edge case)
        if (top_area_height < obstacle_height && bottom_area_height < obstacle_height) {
            return rand() % (screen_height - obstacle_height);
        }

        // Decide whether to spawn in the top or bottom area
        if (top_area_height >= obstacle_height && (bottom_area_height < obstacle_height || (rand() % 2 == 0))) {
            // Spawn in the top area
            return rand() % (top_area_height - obstacle_height + 1);
        }
        
        if (bottom_area_height >= obstacle_height) {
            // Spawn in the bottom area
            return gap_bottom + (rand() % (bottom_area_height - obstacle_height + 1));
        }

        // Fallback, should not be reached if logic is correct
        return rand() % (screen_height - obstacle_height);
    }

private:
    // Helper to select obstacle properties based on a random roll.
    // Kept private as it's an implementation detail of createRegular.
    static std::pair<ObstacleType, ObstacleSize> getObstacleTypeAndSize(
        int grow_chance, int shrink_chance, int type_roll,
        const ObstacleSize& grow_dims, const ObstacleSize& shrink_dims, const ObstacleSize& hurt_dims) {

        ObstacleType type = determineObstacleType(grow_chance, shrink_chance, type_roll);
        switch (type) {
            case ObstacleType::Grow:   return {type, grow_dims};
            case ObstacleType::Shrink: return {type, shrink_dims};
            default:                   return {type, hurt_dims};
        }
    }

public:
    static Obstacle createRegular(int screen_width, int screen_height, int speed, int grow_chance, int shrink_chance,
                                  ObstacleSize grow_dims, ObstacleSize shrink_dims, ObstacleSize hurt_dims,
                                  const std::optional<std::pair<int, int>>& gap) {
        int type_roll = rand() % 100; // Roll a number between 0 and 99
        auto [type, dims] = getObstacleTypeAndSize(grow_chance, shrink_chance, type_roll, grow_dims, shrink_dims, hurt_dims);

        int y = calculateSafeY(screen_height, dims.h, gap);
        return Obstacle(screen_width, y, dims.w, dims.h, speed, type);
    }

};
