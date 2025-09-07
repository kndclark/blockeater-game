#include "Config.h"
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "Color.h"

using json = nlohmann::json;

// Helper to parse color from json
// This allows using .get<Color>() directly
void from_json(const json& j, Color& c) {
    j.at("r").get_to(c.r);
    j.at("g").get_to(c.g);
    j.at("b").get_to(c.b);
    j.at("a").get_to(c.a);
}

Config::Config(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
        std::cerr << "WARNING: Failed to open config file: " << filepath << ". Using default colors." << std::endl;
        // Use default colors as a fallback
        playerColor = {100, 100, 100, 255}; // Gray
        obstacleColors[ObstacleType::Hurt] = {120, 120, 120, 255}; // Gray
        obstacleColors[ObstacleType::Grow] = {140, 140, 140, 255}; // Gray
        obstacleColors[ObstacleType::Shrink] = {160, 160, 160, 255}; // Gray
        return;
    }

    try {
        json data = json::parse(f);
        playerColor = data.at("player").get<Color>();
        obstacleColors[ObstacleType::Hurt] = data.at("obstacle_hurt").get<Color>();
        obstacleColors[ObstacleType::Grow] = data.at("obstacle_grow").get<Color>();
        obstacleColors[ObstacleType::Shrink] = data.at("obstacle_shrink").get<Color>();
    } catch (const std::exception& e) {
        std::cerr << "WARNING: Error parsing " << filepath << ": " << e.what() << ". Using default colors." << std::endl;
        // Use default colors as a fallback
        playerColor = {100, 100, 100, 255}; // Gray
        obstacleColors[ObstacleType::Hurt] = {120, 120, 120, 255}; // Gray
        obstacleColors[ObstacleType::Grow] = {140, 140, 140, 255}; // Gray
        obstacleColors[ObstacleType::Shrink] = {160, 160, 160, 255}; // Gray
    }
}

Color Config::getPlayerColor() const {
    return playerColor;
}

Color Config::getObstacleColor(ObstacleType type) const {
    // .at() will throw if key not found, which is fine since we populate all keys in constructor.
    return obstacleColors.at(type);
}