#pragma once

#include <string>
#include <map>
#include "Color.h"
#include "Obstacle.h" // For ObstacleType

class Config {
public:
    // Loads configuration from the given JSON file.
    // If loading fails, it will use hardcoded default colors.
    explicit Config(const std::string& filepath);

    Color getPlayerColor() const;
    Color getObstacleColor(ObstacleType type) const;

private:
    Color playerColor;
    std::map<ObstacleType, Color> obstacleColors;
};