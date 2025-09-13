#pragma once

#include <SDL2/SDL_stdinc.h> // For Uint32
#include <string>
#include <map>
#include "../src/Color.h"
#include "../src/Obstacle.h" // For ObstacleType, and dimension structs

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
    int getBaseCheckpointGap() const;
    Uint32 getSpawnInterval() const;
    Uint32 getCheckpointInterval() const;
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

private:
    void load_from_path(const std::string& filepath);
    void load_defaults();

    Color playerColor;
    std::map<ObstacleType, Color> obstacleColors;
    int target_fps;
    int screen_width;
    int screen_height;
    int base_checkpoint_gap;
    Uint32 spawn_interval_ms;
    Uint32 checkpoint_interval_ms;
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
};