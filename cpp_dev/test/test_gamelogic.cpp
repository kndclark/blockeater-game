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
    std::string description;
};

void PrintTo(const CollisionLogicParams& params, std::ostream* os) {
    *os << params.description;
}

class CollisionLogicTest : public SdlTest, public ::testing::WithParamInterface<CollisionLogicParams> {};

TEST_P(CollisionLogicTest, HandlesCollisions) {
    auto params = GetParam();
    Player player(100, 100, 40, 40, 5, {0,0,0,0});
    std::vector<Obstacle> obstacles;
    obstacles.emplace_back(100, 100, 20, 20, 1, params.obstacle_type);

    auto it = obstacles.begin();
    bool running = true;
    int initial_width = player.rect.w;
    Config config; // Use default config

    it = handleCollision(player, it, obstacles, running, config.getPlayerSizeChangeAmount());

    EXPECT_EQ(running, params.expected_running);
    EXPECT_EQ(obstacles.size(), params.expected_obstacle_count);

    switch (params.player_size_change) {
        case PlayerSizeChange::EQUAL:   EXPECT_EQ(player.rect.w, initial_width); break;
        case PlayerSizeChange::GREATER: EXPECT_GT(player.rect.w, initial_width); break;
        case PlayerSizeChange::LESS:    EXPECT_LT(player.rect.w, initial_width); break;
    }
}

INSTANTIATE_TEST_SUITE_P(
    CollisionTests,
    CollisionLogicTest,
    ::testing::Values(
        CollisionLogicParams{ObstacleType::Hurt, false, 1, PlayerSizeChange::EQUAL, "HurtCollision"},
        CollisionLogicParams{ObstacleType::Grow, true, 0, PlayerSizeChange::GREATER, "GrowCollision"},
        CollisionLogicParams{ObstacleType::Shrink, true, 0, PlayerSizeChange::LESS, "ShrinkCollision"},
        CollisionLogicParams{ObstacleType::Checkpoint, false, 1, PlayerSizeChange::EQUAL, "CheckpointCollision"} // A collision with a checkpoint wall should be fatal.
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
    Config config;
    GameState game_state(config, 800, 600);
    game_state.player.rect.x = params.player_x;
    game_state.score = params.initial_score;

    const int initial_w = 40;
    // Grow the player to check if size is reset on checkpoint pass
    game_state.player.grow(20);
    ASSERT_EQ(game_state.player.rect.w, initial_w + 20);

    Obstacle obstacle(params.obstacle_x, 100, 20, 20, 3, params.obstacle_type);
    if (params.obstacle_type == ObstacleType::Checkpoint) {
        obstacle = Obstacle({params.obstacle_x, 0, 20, 100}, {params.obstacle_x, 200, 20, 100}, 3);
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
        CheckpointPassingParams{121, 100, false, 0, 10, true, ObstacleType::Checkpoint, "PlayerPassesCheckpoint"}, // Score becomes 10
        CheckpointPassingParams{122, 100, false, 10, 20, true, ObstacleType::Checkpoint, "PlayerPassesCheckpointWithScore"},
        CheckpointPassingParams{121, 100, true, 10, 10, true, ObstacleType::Checkpoint, "PlayerPassesAlreadyPassedCheckpoint"},
        CheckpointPassingParams{100, 100, false, 0, 0, false, ObstacleType::Checkpoint, "PlayerAtCheckpointEdge"},
        CheckpointPassingParams{121, 100, false, 0, 0, false, ObstacleType::Hurt, "DoesNotAffectNonCheckpoints"}
    ),
    [](const testing::TestParamInfo<CheckpointPassingTest::ParamType>& info) {
        return info.param.description;
    }
);

TEST(GameLogicTest, LevelUp) {
    Config config;
    GameState game_state(config, 800, 600);
    int checkpoints_for_lvl1 = game_state.level_manager.getCheckpointsPerLevel();
    ASSERT_EQ(checkpoints_for_lvl1, 5); // From levels.json
    game_state.checkpoints_passed = checkpoints_for_lvl1 - 1; // = 4

    // Pass a checkpoint. checkpoints_passed becomes 5. 5 % 5 == 0, so level up to 2.
    Obstacle checkpoint1 = Obstacle::createCheckpoint(0, 600, 3, 150); // NOLINT(readability-magic-numbers)
    checkpoint1.rect.x = 50; // Place it behind the player
    if(checkpoint1.rect2) checkpoint1.rect2->x = 50;
    handleCheckpointPassing(game_state.player, checkpoint1, game_state);
    EXPECT_EQ(game_state.checkpoints_passed, checkpoints_for_lvl1);
    EXPECT_EQ(game_state.level, 2);
    EXPECT_TRUE(checkpoint1.passed);

    // After leveling up, LevelManager now uses level 2's config.
    // checkpoints_per_level for level 2 is also 5.
    // Pass another checkpoint. checkpoints_passed becomes 6. 6 % 5 != 0, so level stays 2.
    Obstacle checkpoint2 = Obstacle::createCheckpoint(0, 600, 3, 150); // NOLINT(readability-magic-numbers)
    checkpoint2.rect.x = 50;
    if(checkpoint2.rect2) checkpoint2.rect2->x = 50;
    handleCheckpointPassing(game_state.player, checkpoint2, game_state);
    EXPECT_EQ(game_state.checkpoints_passed, checkpoints_for_lvl1 + 1);
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
    Config config; // Use default config
    LevelManager level_manager(config); // This will apply level 1 config by default
    ObstacleSpawner spawner(level_manager, params.checkpoint_interval, 800, 600, config.getPlayerSizeChangeAmount(), {40,40}, {20,20}, {30,30});
    std::vector<Obstacle> obstacles;

    for (const auto& time : params.spawn_times) {
        spawner.spawn_obstacles(time, obstacles);
    }
    size_t regular_count = std::count_if(obstacles.begin(), obstacles.end(), [](const Obstacle& o){ return o.type != ObstacleType::Checkpoint; });
    size_t checkpoint_count = obstacles.size() - regular_count;

    EXPECT_EQ(regular_count, params.expected_regular) << "Mismatch in regular obstacle count";
    EXPECT_EQ(checkpoint_count, params.expected_checkpoints) << "Mismatch in checkpoint count";
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    ObstacleSpawnerTest,
    ::testing::Values(
        ObstacleSpawnerParams{2000, 60000, {0, 2000, 2001, 4000, 4001}, 2, 0, "SpawnsOnlyRegular"},
        ObstacleSpawnerParams{2000, 2000, {1501, 2001}, 0, 1, "SpawnsBothRegularAndCheckpoint"},
        ObstacleSpawnerParams{2000, 2000, {0, 50, 1999}, 0, 0, "NoSpawnsBeforeInterval"},
        ObstacleSpawnerParams{2000, 2000, {2000, 2000, 2000}, 0, 1, "SpawnsAtAndAfterInterval"},
        ObstacleSpawnerParams{10000, 2000, {2001}, 0, 1, "SpawnsOnlyCheckpoint"}
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
    Config config{kTestConfigPath};
    LevelManager level_manager{config};
    ObstacleSpawner spawner{level_manager, CHECKPOINT_INTERVAL, SCREEN_WIDTH, SCREEN_HEIGHT, config.getPlayerSizeChangeAmount(), {40,40}, {20,20}, {30,30}};
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
class ObstacleSpawnerStateTest : public ::testing::Test {
protected:
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    const Uint32 CHECKPOINT_INTERVAL = 1000;
    Config config{kTestConfigPath};
    LevelManager level_manager{config};
    ObstacleSpawner spawner{level_manager, CHECKPOINT_INTERVAL, SCREEN_WIDTH, SCREEN_HEIGHT, config.getPlayerSizeChangeAmount(), {40,40}, {20,20}, {30,30}};
    std::vector<Obstacle> obstacles;
};

TEST_F(ObstacleSpawnerStateTest, TrackersAreClearedAfterCheckpoint) {
    spawner.shrink_powerups_since_checkpoint.push_back(1);

    // First checkpoint spawn
    spawner.spawn_obstacles(CHECKPOINT_INTERVAL, obstacles);
    ASSERT_EQ(obstacles.size(), 1);
    EXPECT_TRUE(spawner.shrink_powerups_since_checkpoint.empty());

    // Second checkpoint spawn, should not be affected by the previous grow block
    spawner.spawn_obstacles(CHECKPOINT_INTERVAL * 2, obstacles);
    ASSERT_EQ(obstacles.size(), 2);
    const auto& checkpoint2 = obstacles.back();
    ASSERT_EQ(checkpoint2.type, ObstacleType::Checkpoint);
    ASSERT_TRUE(checkpoint2.rect2.has_value());

    const int expected_gap = config.getBaseCheckpointGap();
    const int actual_gap = checkpoint2.rect2->y - checkpoint2.rect.h;
    EXPECT_EQ(actual_gap, expected_gap);
}