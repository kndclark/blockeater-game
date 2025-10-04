#include <gtest/gtest.h>
#include <fstream> // For std::ofstream in ConfigTest
#include "../config/Config.h"
#include "test_helpers.h"

// Test suite for the Config class
TEST_F(ConfigFileTest, LoadsGameConfigFromFile) {
    // The test executable runs from the `test/build` directory, so we navigate up.
    Config config(kTestConfigPath);

    // Check colors
    EXPECT_EQ(config.getPlayerColor().r, 128);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Hurt).r, 255);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Grow).g, 200);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Shrink).b, 0);

    // Check game settings
    EXPECT_EQ(config.getBaseCheckpointGap(), 200);
    EXPECT_EQ(config.getSpawnInterval(), 1500);
    EXPECT_EQ(config.getCheckpointInterval(), 10000);
    EXPECT_EQ(config.getGrowChance(), 40);
    EXPECT_EQ(config.getShrinkChance(), 40);
    EXPECT_EQ(config.getHurtChance(), 20);

    // Check obstacle dimensions
    ObstacleSize grow_dims = config.getGrowDimensions();
    EXPECT_EQ(grow_dims.w, 50);
    EXPECT_EQ(grow_dims.h, 50);

    ObstacleSize shrink_dims = config.getShrinkDimensions();
    EXPECT_EQ(shrink_dims.w, 10);
    EXPECT_EQ(shrink_dims.h, 10);

    ObstacleSize hurt_dims = config.getHurtDimensions();
    EXPECT_EQ(hurt_dims.w, 30);
    EXPECT_EQ(hurt_dims.h, 30);

    // Check player settings
    EXPECT_EQ(config.getPlayerInitialX(), 100);
    EXPECT_EQ(config.getPlayerWidth(), 40);
    EXPECT_EQ(config.getPlayerHeight(), 40);
    EXPECT_EQ(config.getPlayerSpeed(), 5);

    checkConfigScreenResolution(config);
}

TEST_F(SdlTest, FallbackOnMissingFile) {
    Config config("nonexistent_file.json");

    // Should fall back to the hardcoded defaults defined in Config.cpp
    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 100);
    EXPECT_EQ(player_color.g, 100);
    EXPECT_EQ(player_color.b, 100);

    Color hurt_color = config.getObstacleColor(ObstacleType::Hurt);
    EXPECT_EQ(hurt_color.r, 120);

    checkConfigScreenResolution(config);
}

TEST_F(ConfigFileTest, FallbackOnMalformedFile) {
    // Create a temporary malformed JSON file for the test.
    // This file will be created in the `test/build` directory.
    std::ofstream malformed_file(malformed_filename);
    malformed_file << "{ \"player\": { \"r\": 10, "; // Intentionally broken JSON
    malformed_file.close();

    Config config(malformed_filename);
    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 100); // Should be the default gray, not 10 from the broken file.
    EXPECT_EQ(player_color.g, 100);
    EXPECT_EQ(player_color.b, 100);

    checkConfigScreenResolution(config);
}

TEST_F(ConfigFileTest, FallbackOnPartiallyMissingKeys) {
    std::ofstream partial_file(partial_filename);
    partial_file << R"({
        "colors": {
            "player": { "r": 1, "g": 2, "b": 3, "a": 255 }
        },
        "game": {
            "base_checkpoint_gap": 99
        }
    })";
    partial_file.close();

    Config config(partial_filename);

    // Check that the specified values are loaded
    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 1);
    EXPECT_EQ(config.getBaseCheckpointGap(), 99);

    // Check that other values fall back to defaults
    Color hurt_color = config.getObstacleColor(ObstacleType::Hurt);
    EXPECT_EQ(hurt_color.r, 120); // Default value
    EXPECT_EQ(config.getSpawnInterval(), 1500); // Default value

    checkConfigScreenResolution(config);
}