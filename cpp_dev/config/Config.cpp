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
    if (j.contains("grow_chance_percent")) {
        lc.grow_chance_percent = j.at("grow_chance_percent").get<int>();
    }
    if (j.contains("shrink_chance_percent")) {
        lc.shrink_chance_percent = j.at("shrink_chance_percent").get<int>();
    }
    if (j.contains("hurt_chance_percent")) {
        lc.hurt_chance_percent = j.at("hurt_chance_percent").get<int>();
    }
    if (j.contains("checkpoints_per_level")) {
        lc.checkpoints_per_level = j.at("checkpoints_per_level").get<int>();
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
    score_per_grow = 200;
    score_per_shrink = 100;
    score_per_hurt = -500;
    checkpoints_per_level = 10;
    spawn_interval_ms = 1500;
    checkpoint_interval_ms = 10000;
    checkpoint_safe_zone_duration_ms = 500;
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
    score_prefix_ = "Score: ";
    level_prefix_ = "Level: ";
    level_progress_prefix_ = " (";
    level_progress_suffix_ = " checkpoints to next level)";
    gap_size_prefix_ = "Gap Size: ";
    player_size_prefix_ = "Player Size: ";
    player_size_suffix_ = "% of gap size";
    font_path_ = "assets/font.ttf";
    font_size_ = 24;
    ui_text_color_ = {255, 255, 255, 255}; // Default white
}

Config::Config(const std::string& root_path) : root_path_(root_path) {
    std::string config_filepath;
    // if the provided path ends with .json, consider it a full path.
    // otherwise, treat it as a root path to which we append the default config path.
    if (root_path_.length() >= 5 && root_path_.substr(root_path_.length() - 5) == ".json") {
        config_filepath = root_path_;
        // This is a test-only path. We should not load associated files.
        load_from_path(config_filepath);
    } else if (root_path_.empty()) {
        // Fallback for when the base path can't be determined.
        // Assumes the executable is run from the `cpp_dev` directory.
        SDL_Log("Warning: Could not get application base path. Using relative path 'config/json/config.json'");
        config_filepath = "config/json/config.json";
        load_from_path(config_filepath);
        // Then, load the associated levels and UI text files from the same directory.
        std::string config_dir = config_filepath.substr(0, config_filepath.find_last_of("/\\") + 1);
        load_levels(config_dir + "levels.json");
        load_ui_texts(config_dir);
    } else {
        config_filepath = root_path_ + "config/json/config.json";
        load_from_path(config_filepath);
        load_levels(root_path_ + "config/json/levels.json");
        load_ui_texts(root_path_ + "config/json/");
    }

    // Final validation after all files are loaded.
    if (grow_chance_percent + shrink_chance_percent + hurt_chance_percent != 100) {
        throw std::runtime_error("Obstacle spawn chances in config must sum to 100.");
    }
}

void Config::load_ui_texts(const std::string& base_path) {
    std::ifstream f(base_path + "ui_texts.json");
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            score_prefix_ = data.value("/ui_text/score_prefix"_json_pointer, score_prefix_);
            level_prefix_ = data.value("/ui_text/level_prefix"_json_pointer, level_prefix_);
            level_progress_prefix_ = data.value("/ui_text/level_progress_prefix"_json_pointer, level_progress_prefix_);
            level_progress_suffix_ = data.value("/ui_text/level_progress_suffix"_json_pointer, level_progress_suffix_);
            gap_size_prefix_ = data.value("/ui_text/gap_size_prefix"_json_pointer, gap_size_prefix_);
            player_size_prefix_ = data.value("/ui_text/player_size_prefix"_json_pointer, player_size_prefix_);
            player_size_suffix_ = data.value("/ui_text/player_size_suffix"_json_pointer, player_size_suffix_);
            font_path_ = data.value("/ui_text/font/path"_json_pointer, font_path_);
            font_size_ = data.value("/ui_text/font/size"_json_pointer, font_size_);
            ui_text_color_ = data.value("/ui_text/text_color"_json_pointer, ui_text_color_);

            // Prepend the base path to the font path if it's not absolute.
            // This makes the font path relative to the project root.
            if (!root_path_.empty()) {
                font_path_ = root_path_ + font_path_;
            }
        } catch (const std::exception& e) {
            std::cerr << "WARNING: Error parsing ui_texts.json: " << e.what() << ". Using default UI text." << std::endl;
        }
    } else {
        // This is not a critical error, as default values are set.
        SDL_Log("Info: ui_texts.json not found. Using default UI text.");
    }
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
            score_per_grow = data.value("/game/score_per_grow"_json_pointer, 200);
            score_per_shrink = data.value("/game/score_per_shrink"_json_pointer, 100);
            dash_boost_multiplier_ = data.value("/game/score_boosts/dash_multiplier"_json_pointer, 1.0f);
            size_boost_threshold_ = data.value("/game/score_boosts/size_threshold_percent"_json_pointer, 80);
            size_boost_multiplier_ = data.value("/game/score_boosts/size_multiplier"_json_pointer, 1.0f);
            score_per_hurt = data.value("/game/score_per_hurt"_json_pointer, -500);
            checkpoints_per_level = data.value("/game/checkpoints_per_level"_json_pointer, 10);
            obstacle_speed = data.value("/game/obstacle_speed"_json_pointer, 3);
            spawn_interval_ms = data.value("/game/spawn_interval_ms"_json_pointer, 1500);
            checkpoint_safe_zone_duration_ms = data.value("/game/checkpoint_safe_zone_duration_ms"_json_pointer, 500);
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

        } catch (const std::exception& e) {
            std::cerr << "WARNING: Error parsing " << filepath << ": " << e.what() << ". Using default configuration." << std::endl;
            load_defaults();
        }
    } else {
        std::cerr << "WARNING: Failed to open config file: " << filepath << ". Using default configuration." << std::endl;
        load_defaults();
    }

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

Uint32 Config::getCheckpointSafeZoneDuration() const {
    return checkpoint_safe_zone_duration_ms;
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

int Config::getScorePerGrow() const {
    return score_per_grow;
}

int Config::getScorePerShrink() const {
    return score_per_shrink;
}

int Config::getScorePerHurt() const {
    return score_per_hurt;
}

float Config::getDashBoostMultiplier() const {
    return dash_boost_multiplier_;
}

int Config::getSizeBoostThreshold() const {
    return size_boost_threshold_;
}

float Config::getSizeBoostMultiplier() const {
    return size_boost_multiplier_;
}

ObstacleConfig Config::getObstacleConfig() const {
    return {
        getGrowChance(),
        getShrinkChance(),
        getGrowDimensions(),
        getShrinkDimensions(),
        getHurtDimensions(),
        getScorePerGrow(),
        getScorePerShrink(),
        getScorePerCheckpoint()
    };
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

const std::string& Config::getScorePrefix() const {
    return score_prefix_;
}

const std::string& Config::getFontPath() const {
    return font_path_;
}

int Config::getFontSize() const {
    return font_size_;
}

const std::string& Config::getLevelPrefix() const {
    return level_prefix_;
}

Color Config::getUiTextColor() const {
    return ui_text_color_;
}

const std::string& Config::getGapSizePrefix() const {
    return gap_size_prefix_;
}

const std::string& Config::getPlayerSizePrefix() const {
    return player_size_prefix_;
}

const std::string& Config::getPlayerSizeSuffix() const {
    return player_size_suffix_;
}

const std::string& Config::getLevelProgressPrefix() const {
    return level_progress_prefix_;
}

const std::string& Config::getLevelProgressSuffix() const {
    return level_progress_suffix_;
}