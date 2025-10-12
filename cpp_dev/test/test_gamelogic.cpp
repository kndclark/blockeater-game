#include <gtest/gtest.h>
#include <vector>
#include <optional>
#include <numeric>
#include <set>
#include "../src/Player.h"
#include "../src/Obstacle.h"
#include "../config/Config.h"
#include "../src/GameState.h"
#include "../src/GameLogic.h" 
#include "test_helpers.h"

// --- FPS Calculation Test ---
struct FpsCalculationParams {
    // Inputs
    Uint32 initial_frame_count;
    Uint32 initial_last_fps_update_time;
    Uint32 current_time;

    // Expected outputs
    bool expect_update;
    std::optional<float> expected_fps;
    Uint32 expected_final_frame_count;
    Uint32 expected_final_last_fps_update_time;
    std::string description;
};

void PrintTo(const FpsCalculationParams& params, std::ostream* os) {
    *os << params.description;
}

class FpsCalculationTest : public ::testing::TestWithParam<FpsCalculationParams> {};

TEST_P(FpsCalculationTest, CorrectlyCalculatesFps) {
    auto params = GetParam();
    Uint32 frame_count = params.initial_frame_count;
    Uint32 last_fps_update_time = params.initial_last_fps_update_time;

    auto fps_opt = calculateFps(frame_count, last_fps_update_time, params.current_time);

    EXPECT_EQ(fps_opt.has_value(), params.expect_update);
    if (params.expected_fps.has_value()) {
        ASSERT_TRUE(fps_opt.has_value());
        EXPECT_NEAR(*fps_opt, *params.expected_fps, 0.01f);
    }

    EXPECT_EQ(frame_count, params.expected_final_frame_count);
    EXPECT_EQ(last_fps_update_time, params.expected_final_last_fps_update_time);
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    FpsCalculationTest,
    ::testing::Values(
        FpsCalculationParams{0, 0, 999, false, std::nullopt, 1, 0, "NoUpdateBeforeOneSecond"},
        FpsCalculationParams{59, 0, 1000, true, 60.0f, 0, 1000, "UpdateAfterExactlyOneSecond"},
        FpsCalculationParams{119, 1000, 2005, true, 120.0f / 1.005f, 0, 2005, "UpdateAfterMoreThanOneSecond"}
    ),
    [](const testing::TestParamInfo<FpsCalculationTest::ParamType>& info) {
        return info.param.description;
    }
);

// --- Collision Logic Test ---
enum class PlayerSizeChange { EQUAL, GREATER, LESS };

struct CollisionLogicParams {
    ObstacleType obstacle_type;
    bool expected_running;
    size_t expected_obstacle_count;
    PlayerSizeChange player_size_change;
    int expected_score;
    std::string description;
};

void PrintTo(const CollisionLogicParams& params, std::ostream* os) {
    *os << params.description;
}

class CollisionLogicTest : public SdlTest, public ::testing::WithParamInterface<CollisionLogicParams> {};

TEST_P(CollisionLogicTest, HandlesCollisions) {
    auto params = GetParam();
    Config config(kTestRootPath);
    GameState game_state(config, 800, 600);
    game_state.player.rect.x = 100;
    game_state.player.rect.y = 100;

    int points = 0;
    if (params.obstacle_type == ObstacleType::Grow) {
        points = config.getScorePerGrow();
    } else if (params.obstacle_type == ObstacleType::Shrink) {
        points = config.getScorePerShrink();
    }

    game_state.obstacles.emplace_back(100, 100, 20, 20, 1, params.obstacle_type, points);

    auto it = game_state.obstacles.begin();
    int initial_width = game_state.player.rect.w;

    it = handleCollision(game_state, it, game_state.obstacles);

    EXPECT_EQ(game_state.running, params.expected_running);
    EXPECT_EQ(game_state.obstacles.size(), params.expected_obstacle_count);
    EXPECT_EQ(game_state.score, params.expected_score);

    switch (params.player_size_change) {
        case PlayerSizeChange::EQUAL:   EXPECT_EQ(game_state.player.rect.w, initial_width); break;
        case PlayerSizeChange::GREATER: EXPECT_GT(game_state.player.rect.w, initial_width); break;
        case PlayerSizeChange::LESS:    EXPECT_LT(game_state.player.rect.w, initial_width); break;
    }
}

INSTANTIATE_TEST_SUITE_P(
    CollisionTests,
    CollisionLogicTest,
    ::testing::Values(
        CollisionLogicParams{ObstacleType::Hurt, false, 1, PlayerSizeChange::EQUAL, 0, "HurtCollision"},
        CollisionLogicParams{ObstacleType::Grow, true, 0, PlayerSizeChange::GREATER, 200, "GrowCollision"},
        CollisionLogicParams{ObstacleType::Shrink, true, 0, PlayerSizeChange::LESS, 100, "ShrinkCollision"},
        CollisionLogicParams{ObstacleType::Checkpoint, false, 1, PlayerSizeChange::EQUAL, 0, "CheckpointCollision"} // A collision with a checkpoint wall should be fatal.
    ),
    [](const testing::TestParamInfo<CollisionLogicTest::ParamType>& info) {
        return info.param.description;
    }
);

// --- Checkpoint Passing Logic Test ---
struct CheckpointPassingParams {
    int player_x;
    int obstacle_x;
    bool obstacle_initially_passed;
    int initial_score;
    int expected_score;
    bool expected_passed_state;
    ObstacleType obstacle_type;
    std::string description;
};

void PrintTo(const CheckpointPassingParams& params, std::ostream* os) {
    *os << params.description;
}

class CheckpointPassingTest : public ::testing::TestWithParam<CheckpointPassingParams> {};

TEST_P(CheckpointPassingTest, HandlesPassingCorrectly) {
    auto params = GetParam();
    Config config(kTestRootPath);
    GameState game_state(config, 800, 600);
    game_state.player.rect.x = params.player_x;
    game_state.score = params.initial_score;

    const int initial_w = 40;
    // Grow the player to check if size is reset on checkpoint pass
    game_state.player.grow(20);
    ASSERT_EQ(game_state.player.rect.w, initial_w + 20);

    Obstacle obstacle(params.obstacle_x, 100, 20, 20, 3, params.obstacle_type);
    if (params.obstacle_type == ObstacleType::Checkpoint) {
        obstacle = Obstacle({params.obstacle_x, 0, 20, 100}, {params.obstacle_x, 200, 20, 100}, 3, 10);
    }
    obstacle.passed = params.obstacle_initially_passed;

    handleCheckpointPassing(game_state.player, obstacle, game_state);

    EXPECT_EQ(game_state.score, params.expected_score);
    EXPECT_EQ(obstacle.passed, params.expected_passed_state);

    bool should_reset = params.obstacle_type == ObstacleType::Checkpoint &&
                        !params.obstacle_initially_passed &&
                        game_state.player.rect.x > obstacle.rect.x + obstacle.rect.w;

    EXPECT_EQ(game_state.player.rect.w, should_reset ? initial_w : initial_w + 20);
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    CheckpointPassingTest,
    ::testing::Values(
        CheckpointPassingParams{100, 121, false, 0, 0, false, ObstacleType::Checkpoint, "PlayerBeforeCheckpoint"},
        CheckpointPassingParams{121, 100, false, 0, 500, true, ObstacleType::Checkpoint, "PlayerPassesCheckpoint"}, // Score becomes 10
        CheckpointPassingParams{122, 100, false, 500, 1000, true, ObstacleType::Checkpoint, "PlayerPassesCheckpointWithScore"},
        CheckpointPassingParams{121, 100, true, 500, 500, true, ObstacleType::Checkpoint, "PlayerPassesAlreadyPassedCheckpoint"},
        CheckpointPassingParams{100, 100, false, 0, 0, false, ObstacleType::Checkpoint, "PlayerAtCheckpointEdge"},
        CheckpointPassingParams{121, 100, false, 0, 0, false, ObstacleType::Hurt, "DoesNotAffectNonCheckpoints"}
    ),
    [](const testing::TestParamInfo<CheckpointPassingTest::ParamType>& info) {
        return info.param.description;
    }
);

Obstacle createPlacedCheckpoint(int x_pos) {
    int dummy_gap_y;
    std::vector<Obstacle> nearby;
    Obstacle checkpoint = Obstacle::createCheckpoint(0, 600, 3, 150, 10, nearby, dummy_gap_y); // NOLINT(readability-magic-numbers)
    checkpoint.rect.x = x_pos;
    if(checkpoint.rect2) checkpoint.rect2->x = x_pos;
    return checkpoint;
}

TEST(GameLogicTest, LevelUp) {
    Config config(kTestRootPath);
    GameState game_state(config, 800, 600);
    
    // --- Test Level 1 -> 2 ---
    const int checkpoints_for_lvl1 = game_state.level_manager.getCheckpointsPerLevel();
    game_state.checkpoints_passed_in_level = checkpoints_for_lvl1 - 1;

    // Pass a checkpoint. This should trigger a level up to 2.
    Obstacle checkpoint1 = createPlacedCheckpoint(50); // Place it behind the player
    handleCheckpointPassing(game_state.player, checkpoint1, game_state);
    
    EXPECT_EQ(game_state.checkpoints_passed_in_level, 0); // Counter should reset
    EXPECT_EQ(game_state.level, 2);
    EXPECT_TRUE(checkpoint1.passed);

    // --- Test within Level 2 ---
    // After leveling up, LevelManager should be using level 2's config.
    
    // Pass another checkpoint. This should NOT trigger a level up.
    Obstacle checkpoint2 = createPlacedCheckpoint(50);
    handleCheckpointPassing(game_state.player, checkpoint2, game_state);
    
    EXPECT_EQ(game_state.checkpoints_passed_in_level, 1); // Counter should increment
    EXPECT_EQ(game_state.level, 2);
    EXPECT_TRUE(checkpoint2.passed);
}


// --- Obstacle Spawner Test ---
struct ObstacleSpawnerParams {
    Uint32 regular_interval;
    Uint32 checkpoint_interval;
    std::vector<Uint32> spawn_times;
    size_t expected_regular;
    size_t expected_checkpoints;
    std::string description;
};

void PrintTo(const ObstacleSpawnerParams& params, std::ostream* os) {
    *os << params.description;
}

class ObstacleSpawnerTest : public SdlTest, public ::testing::WithParamInterface<ObstacleSpawnerParams> {};

TEST_P(ObstacleSpawnerTest, SpawnsCorrectlyOverTime) {
    auto params = GetParam();
    Config config(kTestRootPath); // Use default config
    TestLevelManager test_level_manager = createTestLevelManager(config, params.regular_interval, params.checkpoint_interval);
    ObstacleSpawner test_spawner(test_level_manager, config.getCheckpointSafeZoneDuration(), 800, 600, config.getPlayerSizeChangeAmount());
    
    GameState game_state(config, 800, 600);
    injectSpawnerForTest(game_state, std::move(test_spawner));
    for (const auto& time : params.spawn_times) {
        game_state.spawner.spawn_obstacles(time, game_state);
    }
    size_t regular_count = std::count_if(game_state.obstacles.begin(), game_state.obstacles.end(), [](const Obstacle& o){ return o.type != ObstacleType::Checkpoint; });
    size_t checkpoint_count = game_state.obstacles.size() - regular_count;

    EXPECT_EQ(regular_count, params.expected_regular) << "Mismatch in regular obstacle count";
    EXPECT_EQ(checkpoint_count, params.expected_checkpoints) << "Mismatch in checkpoint count";
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    ObstacleSpawnerTest,
    ::testing::Values(
        ObstacleSpawnerParams{2000, 12000, {0, 2000, 2001, 4000, 4001}, 2, 0, "SpawnsOnlyRegular"},
        ObstacleSpawnerParams{2000, 12000, {12000}, 0, 1, "SpawnsCheckpointAndNotRegular"},
        ObstacleSpawnerParams{2000, 12000, {0, 50, 1999}, 0, 0, "NoSpawnsBeforeInterval"},
        ObstacleSpawnerParams{2000, 12000, {2000, 4000, 6000}, 3, 0, "SpawnsAtIntervals"},
        ObstacleSpawnerParams{10000, 12000, {12001}, 0, 1, "SpawnsOnlyCheckpoint"}
    ),
    [](const testing::TestParamInfo<ObstacleSpawnerTest::ParamType>& info) {
        return info.param.description;
    }
);

// --- Obstacle Spawner Checkpoint Gap Test ---
struct ObstacleSpawnerGapParams {
    // These vectors represent the history of power-ups collected. The values
    // within are ignored; only the number of elements matters.
    std::vector<int> shrink_block_history;
    int expected_gap;
    std::string description;
};

void PrintTo(const ObstacleSpawnerGapParams& params, std::ostream* os) {
    *os << params.description;
}

// --- Obstacle Spawner Checkpoint Gap Test ---
class CheckpointGapCalculationTest : public ::testing::TestWithParam<ObstacleSpawnerGapParams> {
protected:
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    const Uint32 CHECKPOINT_INTERVAL = 1000;
    Config config{kTestRootPath};
    LevelManager level_manager{config};
    ObstacleSpawner spawner{level_manager, config.getCheckpointSafeZoneDuration(), SCREEN_WIDTH, SCREEN_HEIGHT, config.getPlayerSizeChangeAmount()};
};

TEST_P(CheckpointGapCalculationTest, CalculatesCorrectGapSize) {
    auto params = GetParam();
    spawner.shrink_powerups_since_checkpoint = params.shrink_block_history;

    // The gap calculation is based on the base_checkpoint_gap from the config.
    const int actual_gap = spawner.calculateCheckpointGapSize();
    EXPECT_EQ(actual_gap, params.expected_gap);
}

INSTANTIATE_TEST_SUITE_P(
    GapCalculationTests,
    CheckpointGapCalculationTest,
    ::testing::Values(
        // base_gap is 200 from config.json. gap_adjustment is 10. min_gap is 25.
        ObstacleSpawnerGapParams{{}, 200, "IsBaseWhenNoPowerups"},
        // 1 shrink -> 200 - 1*10 = 190
        ObstacleSpawnerGapParams{{1}, 190, "DecreasesWithOneShrinkBlock"},
        // 2 shrinks -> 200 - 2*10 = 180
        ObstacleSpawnerGapParams{{1, 1}, 180, "DecreasesWithTwoShrinkBlocks"},
        // 18 shrinks -> 200 - 18*10 = 20. Clamped to min_gap_height (25), so 25.
        ObstacleSpawnerGapParams{std::vector<int>(18), 25, "ClampsAtMinimum"}
    ),
    [](const testing::TestParamInfo<CheckpointGapCalculationTest::ParamType>& info) {
        return info.param.description;
    }
);

// This test is separate because it has a different structure (multiple spawns).
class ObstacleSpawnerStateTest : public SdlTest {
protected:
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    Config config{kTestRootPath};
    LevelManager level_manager{config};
    // We create a spawner with the test-specific interval and inject it into GameState.
    ObstacleSpawner spawner{level_manager, config.getCheckpointSafeZoneDuration(), SCREEN_WIDTH, SCREEN_HEIGHT, config.getPlayerSizeChangeAmount()};
    GameState game_state;

    ObstacleSpawnerStateTest() : game_state(config, SCREEN_WIDTH, SCREEN_HEIGHT) { injectSpawnerForTest(game_state, std::move(spawner)); }
};

TEST_F(ObstacleSpawnerStateTest, TrackersAreClearedAfterCheckpoint) {
    game_state.spawner.shrink_powerups_since_checkpoint.push_back(1);
    const Uint32 checkpoint_interval = game_state.level_manager.getCheckpointInterval();

    // First checkpoint spawn
    game_state.spawner.spawn_obstacles(checkpoint_interval, game_state);
    ASSERT_EQ(game_state.obstacles.size(), 1);
    EXPECT_TRUE(game_state.spawner.shrink_powerups_since_checkpoint.empty());

    // Second checkpoint spawn, should not be affected by the previous grow block
    game_state.spawner.spawn_obstacles(checkpoint_interval * 2, game_state);
    ASSERT_EQ(game_state.obstacles.size(), 2);
    const auto& checkpoint2 = game_state.obstacles.back();
    ASSERT_EQ(checkpoint2.type, ObstacleType::Checkpoint);
    ASSERT_TRUE(checkpoint2.rect2.has_value());

    const int expected_gap = config.getBaseCheckpointGap();
    const int actual_gap = checkpoint2.rect2->y - checkpoint2.rect.h;
    EXPECT_EQ(actual_gap, expected_gap);
};

TEST_F(ObstacleSpawnerStateTest, UiNextGapSizeIsUpdatedOnlyOnCheckpoint) {
    // This test uses the spawner injected in the test fixture's constructor.
    // The checkpoint interval is set to 1000ms there.
    const Uint32 checkpoint_interval = game_state.level_manager.getCheckpointInterval();

    // Grab the initial values before any changes.
    const int initial_ui_gap_size = game_state.ui_next_checkpoint_gap_size;
    const int initial_internal_gap_size = game_state.next_checkpoint_gap_size;

    // Manually trigger the logic that happens when a shrink power-up is spawned.
    // We simulate this happening at a time before the first checkpoint.
    const Uint32 regular_spawn_time = 1000;
    game_state.spawner.last_spawn_time = regular_spawn_time;
    game_state.spawner.shrink_powerups_since_checkpoint.push_back(1);
    // Manually call calculateCheckpointGapSize to update the internal prediction,
    // just as spawn_obstacles would.
    game_state.next_checkpoint_gap_size = game_state.spawner.calculateCheckpointGapSize();

    // Assert that the UI value has NOT changed, but the internal one has.
    EXPECT_EQ(game_state.ui_next_checkpoint_gap_size, initial_ui_gap_size) << "UI gap size should not change when a shrink power-up spawns.";
    EXPECT_NE(game_state.next_checkpoint_gap_size, initial_internal_gap_size) << "Internal gap size should change when a shrink power-up spawns.";

    // Now, spawn a checkpoint. This SHOULD update the UI value to the size of the checkpoint that was just created.
    // The size of this new checkpoint is based on the *updated* internal prediction.
    const int expected_spawned_gap_size = game_state.next_checkpoint_gap_size;
    game_state.spawner.spawn_obstacles(checkpoint_interval, game_state);

    // The UI should now show the size of the checkpoint that was just spawned.
    EXPECT_EQ(game_state.ui_next_checkpoint_gap_size, expected_spawned_gap_size) << "UI gap size should match the size of the newly spawned checkpoint.";
    // The internal prediction is now for the *next* checkpoint, which should be the base size again.
    EXPECT_EQ(game_state.next_checkpoint_gap_size, config.getBaseCheckpointGap()) << "Internal gap size should reset to base after a checkpoint spawns.";
}

// --- Top-Level Game Logic Tests ---

// Test fixture for game logic tests
class TopLevelGameLogicTest : public SdlTest {
protected:
    Config config{kTestRootPath};
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
};

TEST_F(TopLevelGameLogicTest, VictoryConditionIsMetAtMaxLevel) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Set up the game state to be on the final level, about to pass the last checkpoint.
    gameState.level = LevelManager::MAX_LEVEL;
    // Ensure the level manager is updated to the final level's config.
    gameState.level_manager.updateForLevel(gameState.level);
    gameState.checkpoints_passed_in_level = gameState.level_manager.getCheckpointsPerLevel() - 1;

    // Add a checkpoint for the player to pass.
    // The player is at x=100, so a checkpoint at x=50 is behind them.
    Obstacle checkpoint = createPlacedCheckpoint(50);
    gameState.obstacles.push_back(checkpoint);

    // Act: Run the game logic update. This should trigger the victory condition.
    updateGame(gameState);

    // Assert: The level should now be greater than the max level.
    // The main game loop is responsible for setting `running` to false.
    EXPECT_GT(gameState.level, LevelManager::MAX_LEVEL);
};

TEST_F(TopLevelGameLogicTest, GameEndsWhenVictoryConditionIsMet) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Set up the game state to be on the final level, about to pass the last checkpoint.
    gameState.level = LevelManager::MAX_LEVEL;
    // Ensure the level manager is updated to the final level's config.
    gameState.level_manager.updateForLevel(gameState.level);
    gameState.checkpoints_passed_in_level = gameState.level_manager.getCheckpointsPerLevel() - 1;

    // Add a checkpoint for the player to pass.
    // The player is at x=100, so a checkpoint at x=50 is behind them.
    Obstacle checkpoint = createPlacedCheckpoint(50);
    gameState.obstacles.push_back(checkpoint);

    ASSERT_TRUE(gameState.running);

    // Act: Run one iteration of the game loop.
    gameLoopIteration(gameState, config);

    // Assert: The game should no longer be running.
    EXPECT_FALSE(gameState.running);
};

TEST_F(TopLevelGameLogicTest, ProcessInputSetsRunningFalseOnQuit) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    ASSERT_TRUE(gameState.running);

    // Simulate a quit event
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    processInput(gameState, SCREEN_WIDTH, SCREEN_HEIGHT);

    EXPECT_FALSE(gameState.running);
};

TEST_F(TopLevelGameLogicTest, NoPostSpawnOverlaps) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 1. Spawn a checkpoint.
    const Uint32 checkpoint_spawn_time = gameState.level_manager.getCheckpointInterval();
    gameState.spawner.spawn_obstacles(checkpoint_spawn_time, gameState);
    ASSERT_EQ(gameState.obstacles.size(), 1) << "A checkpoint should have been spawned.";

    // 2. Simulate time passing and spawn a regular obstacle.
    // The time must be after the checkpoint's safe zone and after the regular spawn interval.
    const Uint32 safe_zone_end_time = checkpoint_spawn_time + config.getCheckpointSafeZoneDuration() + 1;
    const Uint32 regular_spawn_time = checkpoint_spawn_time + gameState.level_manager.getSpawnInterval();
    const Uint32 next_spawn_time = std::max(safe_zone_end_time, regular_spawn_time);
    gameState.spawner.spawn_obstacles(next_spawn_time, gameState);
    ASSERT_EQ(gameState.obstacles.size(), 2) << "A regular obstacle should have been spawned after the checkpoint.";

    // Simulate the game loop for a few seconds (e.g., 200 frames).
    const int num_frames_to_simulate = 200;
    for (int i = 0; i < num_frames_to_simulate; ++i) {
        // Update obstacle positions
        Obstacle::updateAndRemove(gameState.obstacles);

        // Check for overlaps between all pairs of obstacles.
        if (gameState.obstacles.size() >= 2) {
            for (size_t j = 0; j < gameState.obstacles.size(); ++j) {
                for (size_t k = j + 1; k < gameState.obstacles.size(); ++k) {
                    const auto& obs1 = gameState.obstacles[j];
                    const auto& obs2 = gameState.obstacles[k];

                    // Check for intersection between the primary rects
                    bool overlaps = SDL_HasIntersection(&obs1.rect, &obs2.rect);
                    EXPECT_FALSE(overlaps) << "Obstacles overlap after " << i << " frames at x1=" << obs1.rect.x << ", x2=" << obs2.rect.x;
                }
            }
        }
    }
}

TEST_F(TopLevelGameLogicTest, UpdateGame_PlayerCollidesWithHurtObstacle_EndsGame) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Place a "Hurt" obstacle directly on the player
    gameState.obstacles.emplace_back(gameState.player.rect.x, gameState.player.rect.y, 20, 20, 1, ObstacleType::Hurt, 0);

    ASSERT_TRUE(gameState.running);

    updateGame(gameState);

    EXPECT_FALSE(gameState.running);
};

TEST_F(TopLevelGameLogicTest, UpdateGame_PlayerCollidesWithGrowObstacle_PlayerGrows) {
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    int initial_width = gameState.player.rect.w;
    // Place a "Grow" obstacle directly on the player
    gameState.obstacles.emplace_back(gameState.player.rect.x, gameState.player.rect.y, 20, 20, 1, ObstacleType::Grow, 200);

    ASSERT_EQ(gameState.obstacles.size(), 1);

    updateGame(gameState);

    EXPECT_TRUE(gameState.running);
    EXPECT_GT(gameState.player.rect.w, initial_width);
    EXPECT_EQ(gameState.obstacles.size(), 0); // Obstacle should be removed
};

TEST_F(TopLevelGameLogicTest, RenderGameCompiles) {
    // This test's primary purpose is to ensure that renderGame compiles
    // correctly as part of the test suite. This helps catch `const`
    // correctness issues or other compile-time problems in the render path.
    GameState gameState(config, SCREEN_WIDTH, SCREEN_HEIGHT);
    renderGame(nullptr, gameState, config);
};