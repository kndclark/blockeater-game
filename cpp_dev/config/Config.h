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

/// @brief Holds all configuration related to regular obstacle creation.
struct ObstacleConfig {
    int grow_chance;
    int shrink_chance;
    ObstacleSize grow_dims;
    ObstacleSize shrink_dims;
    ObstacleSize hurt_dims;
    int grow_points;
    int shrink_points;
    int checkpoint_points;
};



class Config {
public:
    // Default constructor: loads from a standard path relative to the executable,
    // optionally taking a root_path to locate assets.
    explicit Config(const std::string& root_path = "");

    Color getPlayerColor() const;
    Color getObstacleColor(ObstacleType type) const;
    int getTargetFps() const;
    int getScreenWidth() const;
    int getScreenHeight() const;
    int getMaxLevel() const;
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
    int getScorePerGrow() const;
    int getScorePerShrink() const;
    int getScorePerHurt() const;
    ObstacleConfig getObstacleConfig() const;
    ObstacleSize getShrinkDimensions() const;
    float getDashBoostMultiplier() const;
    int getSizeBoostThreshold() const;
    float getSizeBoostMultiplier() const;
    ObstacleSize getHurtDimensions() const;
    int getPlayerInitialX() const;
    int getPlayerWidth() const;
    int getPlayerHeight() const;
    int getPlayerSpeed() const;
    float getDashSpeedMultiplier() const;
    Uint32 getDashDurationMs() const;
    Uint32 getDashCooldownMs() const;
    const std::string& getScorePrefix() const;
    virtual const std::string& getFontPath() const;
    int getFontSize() const;
    const std::string& getLevelPrefix() const;
    const std::string& getGapSizePrefix() const;
    const std::string& getLevelProgressPrefix() const;
    const std::string& getPlayerSizePrefix() const;
    const std::string& getPlayerSizeSuffix() const;
    Color getUiTextColor() const;
    const std::string& getGameOverText() const;
    const std::string& getVictoryText() const;
    const std::string& getGameOverInstructions() const;
    const std::string& getPauseMenuTitle() const;
    const std::string& getMainMenuTitle() const;
    const std::string& getMainMenuInstructions() const;
    const std::string& getPauseMenuInstructions() const;
    const std::string& getLevelProgressSuffix() const;
    const std::string& getDashReadyText() const;
    const std::string& getDashCooldownPrefix() const;
    const std::string& getDashCooldownSuffix() const;
    int getCooldownIndicatorRadius() const;
    Color getCooldownIndicatorColor() const;
    const LevelConfig* getLevelConfig(int level) const;

protected:
    void load_levels(const std::string& filepath);

private:
    void load_from_path(const std::string& filepath);
    void load_ui_texts(const std::string& base_path);
    void load_defaults();

    std::string root_path_;
    Color playerColor;
    std::map<int, LevelConfig> level_configs_;
    std::map<ObstacleType, Color> obstacleColors;
    int target_fps;
    int screen_width;
    int max_level_;
    int screen_height;
    int obstacle_speed;
    int base_checkpoint_gap;
    int player_size_change_amount;
    int score_per_checkpoint;
    int score_per_grow;
    int score_per_shrink;
    int score_per_hurt;
    float dash_boost_multiplier_;
    int size_boost_threshold_;
    float size_boost_multiplier_;
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
    float dash_speed_multiplier_;
    Uint32 dash_duration_ms_;
    Uint32 dash_cooldown_ms_;

    // UI Text
    std::string score_prefix_;
    std::string level_prefix_;
    std::string level_progress_prefix_;
    std::string level_progress_suffix_;
    std::string dash_ready_text_;
    std::string dash_cooldown_prefix_;
    std::string dash_cooldown_suffix_;
    int cooldown_indicator_radius_;
    Color cooldown_indicator_color_;
    std::string gap_size_prefix_;
    std::string player_size_prefix_;
    std::string player_size_suffix_;
    std::string game_over_text_;
    std::string victory_text_;
    std::string game_over_instructions_;
    std::string pause_menu_title_;
    std::string main_menu_title_;
    std::string main_menu_instructions_;
    std::string pause_menu_instructions_;
    std::string font_path_;
    Color ui_text_color_;
    int font_size_;
};