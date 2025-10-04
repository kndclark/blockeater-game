#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include "../src/GameState.h"
#include "../src/GameLogic.h"
#include "../src/LevelManager.h"
#include "../config/Config.h"
#include "test_helpers.h"

// Test fixture for game logic tests
class GameLogicTest : public SdlTest {
protected:
    Config config;
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
};

TEST_F(GameLogicTest, VictoryConditionIsMetAtMaxLevel) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Set up the game state to be on the final level, about to pass the last checkpoint.
    gameState.level = LevelManager::MAX_LEVEL;
    gameState.checkpoints_passed = gameState.level_manager.getCheckpointsPerLevel() - 1;
    
    // Add a checkpoint obstacle that the player is about to pass (but not collide with).
    // The player starts at (400, 550). A checkpoint far away ensures no collision.
    Obstacle checkpoint(0, 0, 10, 10, ObstacleType::Checkpoint, 1);
    checkpoint.passed = false; // Ensure it can be "passed"
    gameState.obstacles.push_back(checkpoint);

    // Pre-condition check
    ASSERT_TRUE(gameState.running);

    // Act: Run the game logic update. This should trigger the victory condition.
    updateGame(gameState);

    // Assert: The game should no longer be running.
    EXPECT_FALSE(gameState.running);
}

TEST_F(GameLogicTest, ProcessInputSetsRunningFalseOnQuit) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    ASSERT_TRUE(gameState.running);

    // Simulate a quit event
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    processInput(gameState, SCREEN_WIDTH, SCREEN_HEIGHT);

    EXPECT_FALSE(gameState.running);
}

TEST_F(GameLogicTest, ProcessInput_NoQuitEvent_GameContinuesRunning) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    ASSERT_TRUE(gameState.running);

    // No event pushed to the queue
    processInput(gameState, SCREEN_WIDTH, SCREEN_HEIGHT);

    EXPECT_TRUE(gameState.running);
}

TEST_F(GameLogicTest, UpdateGame_PlayerCollidesWithHurtObstacle_EndsGame) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Place a "Hurt" obstacle directly on the player
    gameState.obstacles.emplace_back(gameState.player.rect.x, gameState.player.rect.y, 20, 20, 1, ObstacleType::Hurt);
    
    ASSERT_TRUE(gameState.running);

    updateGame(gameState);

    EXPECT_FALSE(gameState.running);
}

TEST_F(GameLogicTest, UpdateGame_PlayerCollidesWithGrowObstacle_PlayerGrows) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    int initial_width = gameState.player.rect.w;
    // Place a "Grow" obstacle directly on the player
    gameState.obstacles.emplace_back(gameState.player.rect.x, gameState.player.rect.y, 20, 20, 1, ObstacleType::Grow);
    
    ASSERT_EQ(gameState.obstacles.size(), 1);

    updateGame(gameState);

    EXPECT_TRUE(gameState.running);
    EXPECT_GT(gameState.player.rect.w, initial_width);
    EXPECT_EQ(gameState.obstacles.size(), 0); // Obstacle should be removed
}

TEST_F(GameLogicTest, UpdateGame_PlayerCollidesWithShrinkObstacle_PlayerShrinks) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    int initial_width = gameState.player.rect.w;
    // Place a "Shrink" obstacle directly on the player
    gameState.obstacles.emplace_back(gameState.player.rect.x, gameState.player.rect.y, 20, 20, 1, ObstacleType::Shrink);
    
    ASSERT_EQ(gameState.obstacles.size(), 1);

    updateGame(gameState);

    EXPECT_TRUE(gameState.running);
    EXPECT_LT(gameState.player.rect.w, initial_width);
    EXPECT_EQ(gameState.obstacles.size(), 0); // Obstacle should be removed
}

TEST_F(GameLogicTest, UpdateGame_ObstaclesAreUpdatedAndRemoved) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Add one obstacle that will move off-screen and one that will stay on-screen
    gameState.obstacles.emplace_back(1, 100, 20, 20, 5, ObstacleType::Hurt); // Will move to x=-4
    gameState.obstacles.emplace_back(100, 100, 20, 20, 5, ObstacleType::Hurt); // Will move to x=95

    ASSERT_EQ(gameState.obstacles.size(), 2);

    updateGame(gameState);

    ASSERT_EQ(gameState.obstacles.size(), 1);
    EXPECT_EQ(gameState.obstacles[0].rect.x, 95);
}