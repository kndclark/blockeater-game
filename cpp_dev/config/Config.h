#pragma once

#include <SDL2/SDL_stdinc.h> // For Uint32
#include <string>
#include <map>
#include "../src/Color.h"
#include "../src/Obstacle.h" // For ObstacleType

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

private:
    void load_from_path(const std::string& filepath);

    Color playerColor;
    std::map<ObstacleType, Color> obstacleColors;
    int target_fps;
    int screen_width;
    int screen_height;
    int base_checkpoint_gap;
    Uint32 spawn_interval_ms;
    Uint32 checkpoint_interval_ms;
};