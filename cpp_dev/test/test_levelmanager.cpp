#include <gtest/gtest.h>
#include <fstream>
#include "../config/Config.h"
#include "../src/LevelManager.h"
#include "test_helpers.h"

class LevelManagerTest : public ConfigFileTest {
protected:
    void SetUp() override {
        ConfigFileTest::SetUp();
        // Create a base config file
        std::ofstream base_config_file("base_config.json");
        base_config_file << R"({
            "game": {
                "spawn_interval_ms": 1000,
                "obstacle_speed": 3,
                "base_checkpoint_gap": 100
            }
        })";
        base_config_file.close();

        // Create a levels config file
        std::ofstream levels_file(test_levels_filename);
        levels_file << R"({
            "levels": {
                "1": {
                    "obstacle_speed": 5,
                    "spawn_interval_ms": 800
                },
                "2": {
                    "obstacle_speed": 10
                }
            }
        })";
        levels_file.close();
    }

    void TearDown() override {
        std::remove("base_config.json");
        ConfigFileTest::TearDown();
    }
};

TEST_F(LevelManagerTest, LoadsLevelSpecificConfig) {
    Config config("base_config.json");
    LevelManager level_manager(config);

    // Level 1 should have its own values
    EXPECT_EQ(level_manager.getObstacleSpeed(), 5);
    EXPECT_EQ(level_manager.getSpawnInterval(), 800);
    // base_checkpoint_gap is not in level 1 config, so it should use the base config value
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);

    // Update to level 2
    level_manager.updateForLevel(2);
    EXPECT_EQ(level_manager.getObstacleSpeed(), 10);
    // spawn_interval_ms is not in level 2 config, so it should use the base config value
    EXPECT_EQ(level_manager.getSpawnInterval(), 1000);
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);

    // Update to level 3 (not in config)
    level_manager.updateForLevel(3);
    // Values should remain from the last valid level config (level 2)
    EXPECT_EQ(level_manager.getObstacleSpeed(), 10);
    EXPECT_EQ(level_manager.getSpawnInterval(), 1000);
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);
}