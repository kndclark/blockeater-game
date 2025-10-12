#include "Obstacle.h"
#include "../config/Config.h"

Obstacle Obstacle::createCheckpoint(int screen_width, int screen_height, int speed, int gap_height, int points, const std::vector<Obstacle>& nearby_obstacles, int& out_gap_y) {
    const int checkpoint_width = 30;
    const int max_retries = 10;

    for (int i = 0; i < max_retries; ++i) {
        // Find a safe Y position for the gap itself.
        out_gap_y = calculateSafeY(screen_height, gap_height, nearby_obstacles);

        SDL_Rect top_wall = {screen_width, 0, checkpoint_width, out_gap_y};
        SDL_Rect bottom_wall = {screen_width, out_gap_y + gap_height, checkpoint_width, screen_height - (out_gap_y + gap_height)};

        bool overlaps = false;
        for (const auto& obstacle : nearby_obstacles) {
            if (SDL_HasIntersection(&top_wall, &obstacle.rect) || (obstacle.rect2.has_value() && SDL_HasIntersection(&top_wall, &obstacle.rect2.value())) ||
                SDL_HasIntersection(&bottom_wall, &obstacle.rect) || (obstacle.rect2.has_value() && SDL_HasIntersection(&bottom_wall, &obstacle.rect2.value()))) {
                overlaps = true;
                break;
            }
        }

        if (!overlaps) {
            return Obstacle(top_wall, bottom_wall, speed, points);
        }
    }

    // Fallback: if we can't find a safe spot after several tries, just create it at a random spot.
    out_gap_y = rand() % (screen_height - gap_height);
    SDL_Rect top_wall = {screen_width, 0, checkpoint_width, out_gap_y};
    SDL_Rect bottom_wall = {screen_width, out_gap_y + gap_height, checkpoint_width, screen_height - (out_gap_y + gap_height)};
    return Obstacle(top_wall, bottom_wall, speed, points);
}

int Obstacle::calculateSafeY(int screen_height, int obstacle_height, const std::vector<Obstacle>& nearby_obstacles) {
    const int clearance = 50; // Minimum vertical pixels between obstacles.

    std::vector<std::pair<int, int>> forbidden_zones;

    for (const auto& obs : nearby_obstacles) {
        // For checkpoints, the forbidden zones are the walls, not the gap.
        if (obs.type == ObstacleType::Checkpoint && obs.rect2.has_value()) {
            forbidden_zones.push_back({0, obs.rect.h}); // Top wall
            forbidden_zones.push_back({obs.rect2->y, screen_height - obs.rect2->y}); // Bottom wall
        } else {
            // For regular obstacles, forbid the area around them.
            int start_y = std::max(0, obs.rect.y - clearance);
            int end_y = std::min(screen_height, obs.rect.y + obs.rect.h + clearance);
            forbidden_zones.push_back({start_y, end_y - start_y});
        }
    }

    // Sort and merge overlapping forbidden zones
    std::sort(forbidden_zones.begin(), forbidden_zones.end());
    std::vector<std::pair<int, int>> merged_forbidden;
    for (const auto& zone : forbidden_zones) {
        if (merged_forbidden.empty() || zone.first > merged_forbidden.back().first + merged_forbidden.back().second) {
            merged_forbidden.push_back(zone);
        } else {
            merged_forbidden.back().second = std::max(merged_forbidden.back().second, zone.first + zone.second - merged_forbidden.back().first);
        }
    }

    // Determine the valid spawn zones
    std::vector<std::pair<int, int>> safe_zones;
    int last_y = 0;
    for (const auto& zone : merged_forbidden) {
        if (zone.first > last_y) {
            safe_zones.push_back({last_y, zone.first - last_y});
        }
        last_y = zone.first + zone.second;
    }
    if (last_y < screen_height) {
        safe_zones.push_back({last_y, screen_height - last_y});
    }

    // Filter out safe zones that are too small for the obstacle
    safe_zones.erase(std::remove_if(safe_zones.begin(), safe_zones.end(),
                                    [obstacle_height](const auto& zone) { return zone.second < obstacle_height; }),
                     safe_zones.end());

    if (safe_zones.empty()) {
        // Fallback if no safe spot is possible (very unlikely)
        return rand() % (screen_height - obstacle_height);
    }

    // Pick a random safe zone and a random position within it
    const auto& chosen_zone = safe_zones[rand() % safe_zones.size()];
    return chosen_zone.first + (rand() % (chosen_zone.second - obstacle_height + 1));
}

std::tuple<ObstacleType, ObstacleSize, int> Obstacle::getObstacleTypeAndSize(const ObstacleConfig& obs_cfg) {
    int type_roll = rand() % 100; // Roll a number between 0 and 99
    ObstacleType type = determineObstacleType(obs_cfg.grow_chance, obs_cfg.shrink_chance, type_roll);
    switch (type) {
        case ObstacleType::Grow:   return {type, obs_cfg.grow_dims, obs_cfg.grow_points};
        case ObstacleType::Shrink: return {type, obs_cfg.shrink_dims, obs_cfg.shrink_points};
        default:                   return {type, obs_cfg.hurt_dims, 0};
    }
}

Obstacle Obstacle::createRegular(int screen_width, int screen_height, int speed,
                              const ObstacleConfig& obs_cfg,
                              const std::vector<Obstacle>& nearby_obstacles) {
    auto [type, dims, points] = getObstacleTypeAndSize(obs_cfg);
    int y = calculateSafeY(screen_height, dims.h, nearby_obstacles);
    return Obstacle(screen_width, y, dims.w, dims.h, speed, type, points);
}