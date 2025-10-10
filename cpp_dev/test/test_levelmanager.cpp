#include <gtest/gtest.h>
#include <fstream>
#include "../config/Config.h"
#include "../src/LevelManager.h"
#include "test_helpers.h"

class LevelManagerTest : public ConfigFileTest {
protected:
    void SetUp() override {
        ConfigFileTest::SetUp();
        // Create a dummy directory for the test config files
        std::system("mkdir -p test_config_dir/json");

        // Create a base config file
        std::ofstream base_config_file("test_config_dir/json/base_config.json");
        base_config_file << R"({
            "game": {
                "spawn_interval_ms": 1000,
                "obstacle_speed": 3,
                "base_checkpoint_gap": 100,
                "checkpoints_per_level": 1
            }
        })";
        base_config_file.close();

        // Create a levels config file
        std::ofstream levels_file("test_config_dir/json/levels.json");
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
        std::system("rm -rf test_config_dir");
        ConfigFileTest::TearDown();
    }
};

TEST_F(LevelManagerTest, LoadsLevelSpecificConfig) {
    // Config will automatically look for levels.json in the same directory.
    Config config = Config::fromFile("test_config_dir/json/base_config.json");
    LevelManager level_manager(config);

    // Level 1 should have its own values
    EXPECT_EQ(level_manager.getObstacleSpeed(), 5);
    EXPECT_EQ(level_manager.getSpawnInterval(), 800);
    // base_checkpoint_gap is not in level 1 config, so it should use the base config value
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);

    // Update to level 2
    level_manager.updateForLevel(2);
    EXPECT_EQ(level_manager.getObstacleSpeed(), 10);
    // spawn_interval_ms is not in level 2 config, so it should persist from level 1
    EXPECT_EQ(level_manager.getSpawnInterval(), 800);
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);

    // Update to level 3 (not in config)
    level_manager.updateForLevel(3);
    // Values should remain from the last valid level config (level 2)
    EXPECT_EQ(level_manager.getObstacleSpeed(), 10);
    EXPECT_EQ(level_manager.getSpawnInterval(), 800);
    EXPECT_EQ(level_manager.getBaseCheckpointGap(), 100);
}