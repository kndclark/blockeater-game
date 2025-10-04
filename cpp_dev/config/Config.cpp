#include "Config.h"
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include "../src/Obstacle.h"
#include "../src/LevelManager.h"
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

// Helper to parse LevelConfig from json. `value()` is used for optional fields.
void from_json(const json& j, LevelConfig& lc) {
    if (j.contains("spawn_interval_ms")) {
        lc.spawn_interval_ms = j.at("spawn_interval_ms").get<Uint32>();
    }
    if (j.contains("checkpoint_interval_ms")) {
        lc.checkpoint_interval_ms = j.at("checkpoint_interval_ms").get<Uint32>();
    }
    if (j.contains("obstacle_speed")) {
        lc.obstacle_speed = j.at("obstacle_speed").get<int>();
    }
    // Add other level-specific parameters here...
    if (j.contains("base_checkpoint_gap")) {
        lc.base_checkpoint_gap = j.at("base_checkpoint_gap").get<int>();
    }
}

void Config::load_defaults() {
    playerColor = {100, 100, 100, 255}; // Gray
    obstacleColors[ObstacleType::Hurt] = {120, 120, 120, 255}; // Gray
    obstacleColors[ObstacleType::Grow] = {140, 140, 140, 255}; // Gray
    obstacleColors[ObstacleType::Shrink] = {160, 160, 160, 255}; // Gray
    obstacleColors[ObstacleType::Checkpoint] = {100, 100, 200, 255}; // A nice blue
    target_fps = 60;
    screen_width = 640;
    screen_height = 480;
    obstacle_speed = 3;
    base_checkpoint_gap = 120;
    player_size_change_amount = 10;
    score_per_checkpoint = 10;
    checkpoints_per_level = 10;
    spawn_interval_ms = 1500;
    checkpoint_interval_ms = 10000;
    grow_chance_percent = 40;
    shrink_chance_percent = 40;
    hurt_chance_percent = 20;
    grow_dims = {40, 40};
    shrink_dims = {20, 20};
    hurt_dims = {30, 30};
    player_initial_x = 100;
    player_width = 40;
    player_height = 40;
    player_speed = 5;
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
    load_from_path(config_path); // This will also load levels.json
}

Config::Config(const std::string& filepath) {
    load_from_path(filepath);
}

void Config::load_levels(const std::string& filepath) {
    std::ifstream f(filepath);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            if (data.contains("levels")) {
                for (auto& [level_str, level_data] : data["levels"].items()) {
                    int level = std::stoi(level_str);
                    level_configs_[level] = level_data.get<LevelConfig>();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "WARNING: Error parsing " << filepath << ": " << e.what() << ". Using default level configuration." << std::endl;
        }
    } else {
        // This is not a critical error, as level-specific configs are optional.
        // In tests, this might be expected if no levels.json is created.
    }
}

void Config::load_from_path(const std::string& filepath) {
    std::ifstream f(filepath);
    if (f.is_open()) {
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
            player_size_change_amount = data.value("/game/player_size_change_amount"_json_pointer, 10);
            score_per_checkpoint = data.value("/game/score_per_checkpoint"_json_pointer, 10);
            checkpoints_per_level = data.value("/game/checkpoints_per_level"_json_pointer, 10);
            obstacle_speed = data.value("/game/obstacle_speed"_json_pointer, 3);
            spawn_interval_ms = data.value("/game/spawn_interval_ms"_json_pointer, 1500);
            checkpoint_interval_ms = data.value("/game/checkpoint_interval_ms"_json_pointer, 10000);
            grow_chance_percent = data.value("/game/obstacle_spawn_chances/grow"_json_pointer, 40);
            shrink_chance_percent = data.value("/game/obstacle_spawn_chances/shrink"_json_pointer, 40);
            hurt_chance_percent = data.value("/game/obstacle_spawn_chances/hurt"_json_pointer, 20);
            grow_dims = data.value("/game/obstacle_dimensions/grow"_json_pointer, ObstacleSize{40, 40});
            shrink_dims = data.value("/game/obstacle_dimensions/shrink"_json_pointer, ObstacleSize{20, 20});
            hurt_dims = data.value("/game/obstacle_dimensions/hurt"_json_pointer, ObstacleSize{30, 30});
            player_initial_x = data.value("/game/player/initial_x"_json_pointer, 100);
            player_width = data.value("/game/player/width"_json_pointer, 40);
            player_height = data.value("/game/player/height"_json_pointer, 40);
            player_speed = data.value("/game/player/speed"_json_pointer, 5);

            if (grow_chance_percent + shrink_chance_percent + hurt_chance_percent != 100) {
                throw std::runtime_error("Obstacle spawn chances in config.json must sum to 100.");
            }

        } catch (const std::exception& e) {
            std::cerr << "WARNING: Error parsing " << filepath << ": " << e.what() << ". Using default configuration." << std::endl;
            load_defaults();
        }
    } else {
        std::cerr << "WARNING: Failed to open config file: " << filepath << ". Using default configuration." << std::endl;
        load_defaults();
    }

    // Also load level configurations from a file with the same path but with ".levels.json" suffix.
    load_levels(filepath + ".levels.json");


    // Override screen dimensions with native resolution for fullscreen mode.
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("Warning: Could not get display mode: %s. Using configured resolution.", SDL_GetError());
    } else {
        screen_width = dm.w;
        screen_height = dm.h;
        SDL_Log("Using native screen resolution: %d x %d", screen_width, screen_height);
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

int Config::getObstacleSpeed() const {
    return obstacle_speed;
}

int Config::getBaseCheckpointGap() const {
    return base_checkpoint_gap;
}

int Config::getPlayerSizeChangeAmount() const {
    return player_size_change_amount;
}

int Config::getScorePerCheckpoint() const {
    return score_per_checkpoint;
}

int Config::getCheckpointsPerLevel() const {
    return checkpoints_per_level;
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

int Config::getPlayerInitialX() const {
    return player_initial_x;
}

int Config::getPlayerWidth() const {
    return player_width;
}

int Config::getPlayerHeight() const {
    return player_height;
}

int Config::getPlayerSpeed() const {
    return player_speed;
}

const LevelConfig* Config::getLevelConfig(int level) const {
    auto it = level_configs_.find(level);
    if (it != level_configs_.end()) {
        return &it->second;
    }
    return nullptr;
}