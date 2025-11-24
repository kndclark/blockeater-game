#include "Obstacle.h"
#include "../config/Config.h"

Obstacle Obstacle::createCheckpoint(int screen_width, int screen_height, int speed, int gap_height, int points, const std::vector<Obstacle>& nearby_obstacles, int& out_gap_y) {
    // Let calculateSafeY do the heavy lifting of finding a safe spot for the whole structure.
    // We pass the gap_height to signal that we are placing a checkpoint.
    out_gap_y = calculateSafeY(screen_height, 0, nearby_obstacles, gap_height);
    
    const int checkpoint_width = 30;
    SDL_Rect top_wall = {screen_width, 0, checkpoint_width, out_gap_y};
    SDL_Rect bottom_wall = {screen_width, out_gap_y + gap_height, checkpoint_width, screen_height - (out_gap_y + gap_height)};
    return Obstacle(top_wall, bottom_wall, speed, points);
}

int Obstacle::calculateSafeY(int screen_height, int entity_height, const std::vector<Obstacle>& nearby_obstacles, std::optional<int> gap_height) {
    const int clearance = 50; // Minimum vertical pixels between obstacles.

    std::vector<std::pair<int, int>> cleared_forbidden_zones;
    for (const auto& obs : nearby_obstacles) {
        // Apply clearance to the physical bodies of all obstacles.
        if (obs.type == ObstacleType::Checkpoint && obs.rect2.has_value()) {
            int top_end = obs.rect.y + obs.rect.h;
            int bottom_start = obs.rect2->y;
            cleared_forbidden_zones.push_back({std::max(0, obs.rect.y - clearance), std::min(screen_height, top_end + clearance) - std::max(0, obs.rect.y - clearance)});
            cleared_forbidden_zones.push_back({std::max(0, bottom_start - clearance), std::min(screen_height, obs.rect2->y + obs.rect2->h + clearance) - std::max(0, bottom_start - clearance)});

            // If placing a regular obstacle, add the checkpoint's gap to the forbidden zones, but WITHOUT clearance.
            if (!gap_height.has_value()) {
                int gap_size = bottom_start - top_end;
                if (gap_size > 0) cleared_forbidden_zones.push_back({top_end, gap_size});
            }
        } else {
            // It's a regular obstacle, apply clearance normally.
            int end = obs.rect.y + obs.rect.h;
            int cleared_start = std::max(0, obs.rect.y - clearance);
            int cleared_end = std::min(screen_height, end + clearance);
            cleared_forbidden_zones.push_back({cleared_start, cleared_end - cleared_start});
        }
    }

    // Sort and merge overlapping forbidden zones
    std::sort(cleared_forbidden_zones.begin(), cleared_forbidden_zones.end());
    std::vector<std::pair<int, int>> merged_forbidden;
    for (const auto& zone : cleared_forbidden_zones) {
        if (merged_forbidden.empty() || zone.first > merged_forbidden.back().first + merged_forbidden.back().second) {
            if (zone.second > 0) merged_forbidden.push_back(zone);
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
    // If we are placing a checkpoint, the "entity" we need to fit is the gap.
    // Otherwise, it's the regular obstacle's height.
    int height_to_fit = gap_height.has_value() ? gap_height.value() : entity_height;
    safe_zones.erase(std::remove_if(safe_zones.begin(), safe_zones.end(),
                                    [height_to_fit](const auto& zone) { return zone.second < height_to_fit; }),
                     safe_zones.end());

    if (safe_zones.empty()) {
        // Fallback if no safe spot is possible (very unlikely)
        return rand() % (screen_height - height_to_fit);
    }

    // Pick a random safe zone and a random position within it
    const auto& chosen_zone = safe_zones[rand() % safe_zones.size()];
    if (gap_height.has_value()) {
        return chosen_zone.first + (rand() % (chosen_zone.second - gap_height.value() + 1));
    } else {
        return chosen_zone.first + (rand() % (chosen_zone.second - height_to_fit + 1));
    }
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
    int y = calculateSafeY(screen_height, dims.h, nearby_obstacles, std::nullopt);
    return Obstacle(screen_width, y, dims.w, dims.h, speed, type, points);
}