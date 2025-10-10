#include <gtest/gtest.h>
#include "../config/Config.h"
#include "../src/GameState.h"

#include "test_helpers.h"

// Test suite for the GameState struct
class GameStateTest : public SdlTest {};

TEST_F(GameStateTest, Initialization) {
    const int screen_width = 800;
    const int screen_height = 600;
    Config config = Config::fromFile(kTestConfigPath); // This will call SDL_GetDesktopDisplayMode

    GameState game_state(config, screen_width, screen_height); // This will call SDL_GetTicks

    // Check player initialization
    EXPECT_EQ(game_state.player.rect.x, config.getPlayerInitialX());
    EXPECT_EQ(game_state.player.rect.y, screen_height / 2 - config.getPlayerHeight() / 2);
    EXPECT_EQ(game_state.player.rect.w, config.getPlayerWidth());
    EXPECT_EQ(game_state.player.rect.h, config.getPlayerHeight());
    EXPECT_EQ(game_state.player.speed, config.getPlayerSpeed());
    Color expected_player_color = config.getPlayerColor();
    EXPECT_EQ(game_state.player.color.r, expected_player_color.r);
    EXPECT_EQ(game_state.player.color.g, expected_player_color.g);
    EXPECT_EQ(game_state.player.color.b, expected_player_color.b);

    // Check initial state variables
    EXPECT_TRUE(game_state.obstacles.empty());
    EXPECT_EQ(game_state.score, 0);
    EXPECT_TRUE(game_state.running);
    EXPECT_EQ(game_state.frame_count, 0);
    EXPECT_NE(game_state.last_fps_update_time, 0);

    // GameState initializes with level 1 config, so we test against that.
    const LevelConfig* level1_config = config.getLevelConfig(1);
    ASSERT_NE(level1_config, nullptr);

    // Check spawner initialization
    EXPECT_EQ(game_state.level_manager.getSpawnInterval(), level1_config->spawn_interval_ms.value());
    EXPECT_EQ(game_state.spawner.checkpoint_spawn_interval, config.getCheckpointInterval());
    EXPECT_EQ(game_state.spawner.screen_width, screen_width);
    EXPECT_EQ(game_state.spawner.screen_height, screen_height);
    EXPECT_EQ(game_state.spawner.player_size_change_amount, config.getPlayerSizeChangeAmount());
    EXPECT_EQ(game_state.level_manager.getObstacleSpeed(), level1_config->obstacle_speed.value());
    EXPECT_EQ(game_state.level_manager.getGrowChance(), level1_config->grow_chance_percent.value());
    EXPECT_EQ(game_state.level_manager.getShrinkChance(), level1_config->shrink_chance_percent.value());
    EXPECT_EQ(game_state.level_manager.getBaseCheckpointGap(), level1_config->base_checkpoint_gap.value());
    EXPECT_EQ(game_state.spawner.grow_dims, config.getGrowDimensions());
    EXPECT_EQ(game_state.spawner.shrink_dims, config.getShrinkDimensions());
    EXPECT_EQ(game_state.spawner.hurt_dims, config.getHurtDimensions());
}