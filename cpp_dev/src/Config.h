#pragma once

#include <string>
#include <map>
#include "Color.h"
#include "Obstacle.h" // For ObstacleType

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

private:
    void load_from_path(const std::string& filepath);

    Color playerColor;
    std::map<ObstacleType, Color> obstacleColors;
    int target_fps;
    int screen_width;
    int screen_height;
};