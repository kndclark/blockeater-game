#include <gtest/gtest.h>
#include <fstream> // For std::ofstream in ConfigTest
#include "../config/Config.h"
#include "test_helpers.h"

// Test suite for the Config class
TEST_F(ConfigFileTest, LoadsGameConfigFromFile) {
    // This constructor loads config.json and associated files like levels.json
    Config config(kTestRootPath);

    // Check colors
    EXPECT_EQ(config.getPlayerColor().r, 128);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Hurt).r, 255);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Grow).g, 200);
    EXPECT_EQ(config.getObstacleColor(ObstacleType::Shrink).b, 0);

    // Check UI text color
    Color ui_color = config.getUiTextColor();
    EXPECT_EQ(ui_color.r, 255);
    EXPECT_EQ(ui_color.g, 255);
    EXPECT_EQ(ui_color.b, 255);

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

TEST_F(ConfigFileTest, ThrowsOnInvalidSpawnChances) {
    // Create a temporary config file with spawn chances that don't sum to 100.
    std::ofstream invalid_chances_file(invalid_chances_filename);
    invalid_chances_file << R"({
        "game": { "obstacle_spawn_chances": { "grow": 10, "shrink": 10, "hurt": 10 } }
    })";
    invalid_chances_file.close();

    // Expect a std::runtime_error to be thrown.
    EXPECT_THROW(TestConfig(invalid_chances_filename, true), std::runtime_error); // This now correctly throws
}

TEST_F(ConfigFileTest, FallbackOnMalformedFile) {
    // Create a temporary malformed JSON file for the test.
    // This file will be created in the `test/build` directory.
    std::ofstream malformed_file(malformed_filename);
    malformed_file << "{ \"player\": { \"r\": 10, "; // Intentionally broken JSON
    malformed_file.close();

    TestConfig config(malformed_filename, true);
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

    TestConfig config(partial_filename, true);

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

TEST_F(ConfigFileTest, LoadsLevelsConfig) {
    // This test relies on the main config file being present at kTestConfigPath
    // and the associated levels.json being in the same directory.
    Config config(kTestRootPath);

    // Check level 1 config from levels.json
    const LevelConfig* level1_config = config.getLevelConfig(1);
    ASSERT_NE(level1_config, nullptr);
    EXPECT_EQ(level1_config->obstacle_speed.value(), 3);
    EXPECT_EQ(level1_config->spawn_interval_ms.value(), 2000);
    EXPECT_EQ(level1_config->base_checkpoint_gap.value(), 200);

    // Check level 2 config from levels.json
    const LevelConfig* level2_config = config.getLevelConfig(2);
    ASSERT_NE(level2_config, nullptr);
    EXPECT_EQ(level2_config->obstacle_speed.value(), 4);
    EXPECT_EQ(level2_config->spawn_interval_ms.value(), 1850);

    // Check a level that doesn't exist in levels.json
    const LevelConfig* non_existent_level_config = config.getLevelConfig(99);
    EXPECT_EQ(non_existent_level_config, nullptr);
}