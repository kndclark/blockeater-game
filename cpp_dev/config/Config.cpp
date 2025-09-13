#include "Config.h"
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include "../src/Obstacle.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Helper to parse color from json
// This allows using .get<Color>() directly
void from_json(const json& j, Color& c) {
    j.at("r").get_to(c.r);
    j.at("g").get_to(c.g);
    j.at("b").get_to(c.b);
    j.at("a").get_to(c.a);
}

void from_json(const json& j, ObstacleSize& dims) {
    j.at("w").get_to(dims.w);
    j.at("h").get_to(dims.h);
}

Config::Config() {
    // Construct a path to the config file relative to the executable's location.
    // This makes the game runnable from any working directory.
    std::string config_path;
    char* base_path = SDL_GetBasePath();
    if (base_path) {
#ifdef IS_TEST_BUILD
        // The test executable is in test/build/, so we go up two directories.
        config_path = std::string(base_path) + "../../config/config.json";
#else
        // The game executable is in build/, so we go up one directory.
        config_path = std::string(base_path) + "../config/config.json";
#endif
        SDL_free(base_path);
    } else {
        // Fallback for when the base path can't be determined.
        // Assumes the executable is run from the `cpp_dev` directory.
        SDL_Log("Warning: Could not get application base path. Using relative path 'config/config.json'");
        config_path = "config/config.json";
    }
    load_from_path(config_path);
}

Config::Config(const std::string& filepath) {
    load_from_path(filepath);
}

void Config::load_from_path(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
        std::cerr << "WARNING: Failed to open config file: " << filepath << ". Using default colors." << std::endl;
        // Use default colors as a fallback
        playerColor = {100, 100, 100, 255}; // Gray
        obstacleColors[ObstacleType::Hurt] = {120, 120, 120, 255}; // Gray
        obstacleColors[ObstacleType::Grow] = {140, 140, 140, 255}; // Gray
        obstacleColors[ObstacleType::Shrink] = {160, 160, 160, 255}; // Gray
        obstacleColors[ObstacleType::Checkpoint] = {100, 100, 200, 255}; // A nice blue
        target_fps = 60;
        screen_width = 640;
        screen_height = 480;
        base_checkpoint_gap = 120;
        spawn_interval_ms = 1500;
        checkpoint_interval_ms = 10000;
        grow_chance_percent = 40;
        shrink_chance_percent = 40;
        hurt_chance_percent = 20;
        grow_dims = {40, 40};
        shrink_dims = {20, 20};
        hurt_dims = {30, 30};
        return;
    }

    try {
        json data = json::parse(f);

        // Use value() with a JSON pointer for safe access to nested keys.
        playerColor = data.value("/colors/player"_json_pointer, Color{100, 100, 100, 255});
        obstacleColors[ObstacleType::Hurt] = data.value("/colors/obstacle_hurt"_json_pointer, Color{120, 120, 120, 255});
        obstacleColors[ObstacleType::Grow] = data.value("/colors/obstacle_grow"_json_pointer, Color{140, 140, 140, 255});
        obstacleColors[ObstacleType::Shrink] = data.value("/colors/obstacle_shrink"_json_pointer, Color{160, 160, 160, 255});
        obstacleColors[ObstacleType::Checkpoint] = data.value("/colors/obstacle_checkpoint"_json_pointer, Color{100, 100, 200, 255});
        // For new settings, use value() with a JSON pointer for safe access to nested keys.
        // This will not throw if "settings" or its sub-keys are missing, using the default instead.
        target_fps = data.value("/settings/target_fps"_json_pointer, 60);
        screen_width = data.value("/settings/screen_width"_json_pointer, 640);
        screen_height = data.value("/settings/screen_height"_json_pointer, 480);
        // Game config-related (i.e. difficulty, saved state, etc.)
        base_checkpoint_gap = data.value("/game/base_checkpoint_gap"_json_pointer, 120);
        spawn_interval_ms = data.value("/game/spawn_interval_ms"_json_pointer, 1500);
        checkpoint_interval_ms = data.value("/game/checkpoint_interval_ms"_json_pointer, 10000);
        grow_chance_percent = data.value("/game/obstacle_spawn_chances/grow"_json_pointer, 40);
        shrink_chance_percent = data.value("/game/obstacle_spawn_chances/shrink"_json_pointer, 40);
        hurt_chance_percent = data.value("/game/obstacle_spawn_chances/hurt"_json_pointer, 20);
        grow_dims = data.value("/game/obstacle_dimensions/grow"_json_pointer, ObstacleSize{40, 40});
        shrink_dims = data.value("/game/obstacle_dimensions/shrink"_json_pointer, ObstacleSize{20, 20});
        hurt_dims = data.value("/game/obstacle_dimensions/hurt"_json_pointer, ObstacleSize{30, 30});

        if (grow_chance_percent + shrink_chance_percent + hurt_chance_percent != 100) {
            throw std::runtime_error("Obstacle spawn chances in config.json must sum to 100.");
        }

    } catch (const std::exception& e) {
        std::cerr << "WARNING: Error parsing " << filepath << ": " << e.what() << ". Using default configuration." << std::endl;
        // Use default configuration as a fallback
        playerColor = {100, 100, 100, 255}; // Gray
        obstacleColors[ObstacleType::Hurt] = {120, 120, 120, 255}; // Gray
        obstacleColors[ObstacleType::Grow] = {140, 140, 140, 255}; // Gray
        obstacleColors[ObstacleType::Shrink] = {160, 160, 160, 255}; // Gray
        obstacleColors[ObstacleType::Checkpoint] = {100, 100, 200, 255}; // A nice blue
        target_fps = 60;
        screen_width = 640;
        screen_height = 480;
        base_checkpoint_gap = 120;
        spawn_interval_ms = 1500;
        checkpoint_interval_ms = 10000;
        grow_chance_percent = 40;
        shrink_chance_percent = 40;
        hurt_chance_percent = 20;
        grow_dims = {40, 40};
        shrink_dims = {20, 20};
        hurt_dims = {30, 30};
    }
}

Color Config::getPlayerColor() const {
    return playerColor;
}

Color Config::getObstacleColor(ObstacleType type) const {
    // .at() will throw if key not found, which is fine since we populate all keys in constructor.
    return obstacleColors.at(type);
}

int Config::getTargetFps() const {
    return target_fps;
}

int Config::getScreenWidth() const {
    return screen_width;
}

int Config::getScreenHeight() const {
    return screen_height;
}

int Config::getBaseCheckpointGap() const {
    return base_checkpoint_gap;
}

Uint32 Config::getSpawnInterval() const {
    return spawn_interval_ms;
}

Uint32 Config::getCheckpointInterval() const {
    return checkpoint_interval_ms;
}

int Config::getGrowChance() const {
    return grow_chance_percent;
}

int Config::getShrinkChance() const {
    return shrink_chance_percent;
}

int Config::getHurtChance() const {
    return hurt_chance_percent;
}

ObstacleSize Config::getGrowDimensions() const {
    return grow_dims;
}

ObstacleSize Config::getShrinkDimensions() const {
    return shrink_dims;
}

ObstacleSize Config::getHurtDimensions() const {
    return hurt_dims;
}