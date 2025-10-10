#pragma once

#include <SDL2/SDL_stdinc.h> // For Uint32
#include <string>
#include <optional>
#include <map>
#include "../src/Color.h"
#include "../src/Obstacle.h" // For ObstacleType, and dimension structs

/// @brief Holds configuration that can be overridden on a per-level basis.
struct LevelConfig {
    std::optional<Uint32> spawn_interval_ms;
    std::optional<Uint32> checkpoint_interval_ms;
    std::optional<int> obstacle_speed;
    std::optional<int> grow_chance_percent;
    std::optional<int> shrink_chance_percent;
    std::optional<int> hurt_chance_percent;
    std::optional<int> base_checkpoint_gap;
    std::optional<int> checkpoints_per_level;
};

class Config {
public:
    // Default constructor: loads from a standard path relative to the executable.
    Config();

    // Loads configuration from the given JSON file.
    // If loading fails, it will use hardcoded default colors.
    explicit Config(const std::string& filepath);

    Color getPlayerColor() const;
    Color getObstacleColor(ObstacleType type) const;
    int getTargetFps() const;
    int getScreenWidth() const;
    int getScreenHeight() const;
    int getObstacleSpeed() const;
    int getBaseCheckpointGap() const;
    Uint32 getSpawnInterval() const;
    int getPlayerSizeChangeAmount() const;
    int getScorePerCheckpoint() const;
    int getCheckpointsPerLevel() const;
    Uint32 getCheckpointInterval() const;
    Uint32 getCheckpointSafeZoneDuration() const;
    int getGrowChance() const;
    int getShrinkChance() const;
    int getHurtChance() const;
    ObstacleSize getGrowDimensions() const;
    ObstacleSize getShrinkDimensions() const;
    ObstacleSize getHurtDimensions() const;
    int getPlayerInitialX() const;
    int getPlayerWidth() const;
    int getPlayerHeight() const;
    int getPlayerSpeed() const;
    const std::string& getScorePrefix() const;
    virtual const std::string& getFontPath() const;
    int getFontSize() const;
    const std::string& getLevelPrefix() const;
    const std::string& getLevelProgressPrefix() const;
    const std::string& getLevelProgressSuffix() const;
    const LevelConfig* getLevelConfig(int level) const;

private:
    void load_from_path(const std::string& filepath);
    void load_ui_texts(const std::string& base_path);
    void load_levels(const std::string& filepath);
    void load_defaults();

    Color playerColor;
    std::map<int, LevelConfig> level_configs_;
    std::map<ObstacleType, Color> obstacleColors;
    int target_fps;
    int screen_width;
    int screen_height;
    int obstacle_speed;
    int base_checkpoint_gap;
    int player_size_change_amount;
    int score_per_checkpoint;
    int checkpoints_per_level;
    Uint32 spawn_interval_ms;
    Uint32 checkpoint_interval_ms;
    Uint32 checkpoint_safe_zone_duration_ms;
    int grow_chance_percent;
    int shrink_chance_percent;
    int hurt_chance_percent;
    ObstacleSize grow_dims;
    ObstacleSize shrink_dims;
    ObstacleSize hurt_dims;
    int player_initial_x;
    int player_width;
    int player_height;
    int player_speed;

    // UI Text
    std::string score_prefix_;
    std::string level_prefix_;
    std::string level_progress_prefix_;
    std::string level_progress_suffix_;
    std::string font_path_;
    int font_size_;
};