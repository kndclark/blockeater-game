#include <gtest/gtest.h>
#include <fstream> // For std::ofstream in ConfigTest
#include <vector>
#include <optional>
#include "../src/Player.h"
#include "../src/Obstacle.h"
#include "../src/Color.h"
#include "../config/Config.h"
#include "../src/GameLogic.h"

namespace {
const std::string kTestConfigPath = "../../config/config.json";
} // namespace

// Test suite for the Player class
TEST(PlayerTest, Creation) {
    Color c = {0,0,0,0};
    Player player(10, 20, 30, 40, 5, c);
    EXPECT_EQ(player.rect.x, 10);
    EXPECT_EQ(player.rect.y, 20);
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 40);
    EXPECT_EQ(player.speed, 5);
    EXPECT_EQ(player.default_w, 30);
    EXPECT_EQ(player.default_h, 40);
}

// --- Player Movement Test ---
struct PlayerMovementParams {
    SDL_Scancode key;
    int expected_x;
    int expected_y;
    std::string description;
};

void PrintTo(const PlayerMovementParams& params, std::ostream* os) {
    *os << params.description;
}

class PlayerMovementTest : public ::testing::TestWithParam<PlayerMovementParams> {};

TEST_P(PlayerMovementTest, HandlesMovementCorrectly) {
    auto params = GetParam();
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);
    const int screen_width = 640;
    const int screen_height = 480;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};

    keystate[params.key] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.x, params.expected_x);
    EXPECT_EQ(player.rect.y, params.expected_y);
}

INSTANTIATE_TEST_SUITE_P(
    PlayerTests,
    PlayerMovementTest,
    ::testing::Values(
        PlayerMovementParams{SDL_SCANCODE_LEFT, 95, 100, "MovesLeft"},
        PlayerMovementParams{SDL_SCANCODE_RIGHT, 105, 100, "MovesRight"},
        PlayerMovementParams{SDL_SCANCODE_UP, 100, 95, "MovesUp"},
        PlayerMovementParams{SDL_SCANCODE_DOWN, 100, 105, "MovesDown"}
    ),
    [](const testing::TestParamInfo<PlayerMovementTest::ParamType>& info) {
        return info.param.description;
    }
);

// --- Player Boundary Test ---
struct PlayerBoundaryParams {
    int start_x;
    int start_y;
    SDL_Scancode key;
    std::string description;
};

void PrintTo(const PlayerBoundaryParams& params, std::ostream* os) {
    *os << params.description;
}

class PlayerBoundaryTest : public ::testing::TestWithParam<PlayerBoundaryParams> {};

TEST_P(PlayerBoundaryTest, StopsAtBoundaries) {
    auto params = GetParam();
    const int screen_width = 640;
    const int screen_height = 480;
    const int player_size = 40;
    const int player_speed = 5;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};
    Color c = {0,0,0,0};

    Player player(params.start_x, params.start_y, player_size, player_size, player_speed, c);
    keystate[params.key] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.x, params.start_x);
    EXPECT_EQ(player.rect.y, params.start_y);
}

INSTANTIATE_TEST_SUITE_P(
    PlayerTests,
    PlayerBoundaryTest,
    ::testing::Values(
        PlayerBoundaryParams{0, 100, SDL_SCANCODE_LEFT, "LeftBoundary"},
        PlayerBoundaryParams{640 - 40, 100, SDL_SCANCODE_RIGHT, "RightBoundary"},
        PlayerBoundaryParams{100, 0, SDL_SCANCODE_UP, "TopBoundary"},
        PlayerBoundaryParams{100, 480 - 40, SDL_SCANCODE_DOWN, "BottomBoundary"}
    ),
    [](const testing::TestParamInfo<PlayerBoundaryTest::ParamType>& info) {
        return info.param.description;
    }
);

TEST(PlayerTest, SizeModification) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);

    // Test growing
    player.grow(10);
    EXPECT_EQ(player.rect.w, 50);
    EXPECT_EQ(player.rect.h, 50);

    // Test shrinking from grown state
    player.shrink(20);
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 30);

    // Test resetting size
    player.resetSize();
    EXPECT_EQ(player.rect.w, 40);
    EXPECT_EQ(player.rect.h, 40);

    // Test shrinking below the minimum size (should clamp to 10)
    player.shrink(40);
    EXPECT_EQ(player.rect.w, Player::MIN_SIZE);
    EXPECT_EQ(player.rect.h, Player::MIN_SIZE);
}

// Test suite for the Obstacle class
TEST(ObstacleTest, Creation) {
    Obstacle obstacle(50, 60, 70, 80, 3, ObstacleType::Hurt);
    EXPECT_EQ(obstacle.rect.x, 50);
    EXPECT_EQ(obstacle.rect.y, 60);
    EXPECT_EQ(obstacle.rect.w, 70);
    EXPECT_EQ(obstacle.rect.h, 80);
    EXPECT_EQ(obstacle.speed, 3);
    EXPECT_EQ(obstacle.type, ObstacleType::Hurt);
    EXPECT_FALSE(obstacle.rect2.has_value());
}

TEST(ObstacleTest, Update) {
    Obstacle obstacle(100, 100, 50, 50, 3, ObstacleType::Hurt);
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 97); //speed = 3, 100 - 3 = 97
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 94);
}

TEST(ObstacleTest, TypeAssignment) {
    Obstacle hurt_obstacle(0, 0, 10, 10, 1, ObstacleType::Hurt);
    EXPECT_EQ(hurt_obstacle.type, ObstacleType::Hurt);

    Obstacle grow_obstacle(0, 0, 10, 10, 1, ObstacleType::Grow);
    EXPECT_EQ(grow_obstacle.type, ObstacleType::Grow);

    Obstacle shrink_obstacle(0, 0, 10, 10, 1, ObstacleType::Shrink);
    EXPECT_EQ(shrink_obstacle.type, ObstacleType::Shrink);

    Obstacle checkpoint_obstacle({0,0,0,0}, {0,0,0,0}, 1);
    EXPECT_EQ(checkpoint_obstacle.type, ObstacleType::Checkpoint);
}

TEST(ObstacleTest, IsOffscreen) {
    // Obstacle fully on screen
    EXPECT_FALSE(Obstacle(10, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle touching left edge
    EXPECT_FALSE(Obstacle(0, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle partially offscreen
    EXPECT_FALSE(Obstacle(-10, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle fully offscreen (right edge at x=0)
    EXPECT_TRUE(Obstacle(-20, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
}

// A test fixture for tests that require SDL to be initialized.
class SdlTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0) << "Failed to initialize SDL: " << SDL_GetError();
    }

    void TearDown() override {
        SDL_Quit();
    }
};

// Use the fixture for the collision detection test
TEST_F(SdlTest, CollisionDetection) {
    Player player(100, 100, 40, 40, 5, {0,0,0,0});

    // No collision
    Obstacle no_collision(200, 200, 20, 20, 3, ObstacleType::Hurt);
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &no_collision.rect));

    // Collision
    Obstacle collision(110, 110, 40, 40, 3, ObstacleType::Hurt);
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &collision.rect));

    // Edge collision (intersecting by 1 pixel)
    Obstacle edge_collision(139, 100, 20, 20, 3, ObstacleType::Hurt);
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &edge_collision.rect));
}

TEST_F(SdlTest, CheckpointCollisionDetection) {
    Player player(100, 100, 40, 40, 5, {0,0,0,0});

    // Checkpoint with a gap the player can fit through
    SDL_Rect top_wall = {110, 0, 20, 90};
    SDL_Rect bottom_wall = {110, 150, 20, 330};
    Obstacle checkpoint_no_collide(top_wall, bottom_wall, 3);
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &checkpoint_no_collide.rect));
    ASSERT_TRUE(checkpoint_no_collide.rect2.has_value());
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &*checkpoint_no_collide.rect2));

    // Player collides with top wall
    SDL_Rect top_wall_collide = {110, 0, 20, 110};
    SDL_Rect bottom_wall_no_collide = {110, 150, 20, 330};
    Obstacle checkpoint_collide_top(top_wall_collide, bottom_wall_no_collide, 3);
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &checkpoint_collide_top.rect));
    ASSERT_TRUE(checkpoint_collide_top.rect2.has_value());
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &*checkpoint_collide_top.rect2));

    // Player collides with bottom wall
    SDL_Rect top_wall_no_collide_2 = {110, 0, 20, 90};
    SDL_Rect bottom_wall_collide = {110, 130, 20, 350};
    Obstacle checkpoint_collide_bottom(top_wall_no_collide_2, bottom_wall_collide, 3);
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &checkpoint_collide_bottom.rect));
    ASSERT_TRUE(checkpoint_collide_bottom.rect2.has_value());
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &*checkpoint_collide_bottom.rect2));
}

// A test fixture for tests that create temporary files.
class ConfigFileTest : public ::testing::Test {
protected:
    const std::string malformed_filename = "malformed.json";

    void TearDown() override {
        std::remove(malformed_filename.c_str());
    }
};

// Test suite for the Config class
TEST(ConfigTest, LoadsConfigFromFile) {
    // The test executable runs from the `test/build` directory, so we navigate up.
    Config config(kTestConfigPath);

    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 128);
    EXPECT_EQ(player_color.g, 0);
    EXPECT_EQ(player_color.b, 128);

    Color hurt_color = config.getObstacleColor(ObstacleType::Hurt);
    EXPECT_EQ(hurt_color.r, 255);
    EXPECT_EQ(hurt_color.g, 50);
    EXPECT_EQ(hurt_color.b, 50);

    Color grow_color = config.getObstacleColor(ObstacleType::Grow);
    EXPECT_EQ(grow_color.r, 50);
    EXPECT_EQ(grow_color.g, 200);
    EXPECT_EQ(grow_color.b, 50);

    Color shrink_color = config.getObstacleColor(ObstacleType::Shrink);
    EXPECT_EQ(shrink_color.r, 255);
    EXPECT_EQ(shrink_color.g, 165);
    EXPECT_EQ(shrink_color.b, 0);
}

TEST(ConfigTest, FallbackOnMissingFile) {
    Config config("nonexistent_file.json");

    // Should fall back to the hardcoded defaults defined in Config.cpp
    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 100);
    EXPECT_EQ(player_color.g, 100);
    EXPECT_EQ(player_color.b, 100);

    Color hurt_color = config.getObstacleColor(ObstacleType::Hurt);
    EXPECT_EQ(hurt_color.r, 120);
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
}

TEST(ConfigTest, LoadsGameConfigFromFile) {
    Config config(kTestConfigPath);
    EXPECT_EQ(config.getBaseCheckpointGap(), 80);
    EXPECT_EQ(config.getSpawnInterval(), 1500);
    EXPECT_EQ(config.getCheckpointInterval(), 10000);
}

// A helper struct for our parameterized test for determineObstacleType.
struct DetermineObstacleTypeParams {
    int grow_chance;
    int shrink_chance;
    int roll;
    ObstacleType expected_type;
};

// A custom printer for the test parameters to make test output more readable.
void PrintTo(const DetermineObstacleTypeParams& params, std::ostream* os) {
    *os << "grow_chance: " << params.grow_chance
        << ", shrink_chance: " << params.shrink_chance
        << ", roll: " << params.roll
        << ", expected_type: " << static_cast<int>(params.expected_type);
}

// Test suite for GameLogic functions
class DetermineObstacleTypeTest : public ::testing::TestWithParam<DetermineObstacleTypeParams> {};

TEST_P(DetermineObstacleTypeTest, CorrectlyDeterminesType) {
    auto params = GetParam();
    EXPECT_EQ(determineObstacleType(params.grow_chance, params.shrink_chance, params.roll), params.expected_type);
}

INSTANTIATE_TEST_SUITE_P(
    ObstacleLogicTests,
    DetermineObstacleTypeTest,
    ::testing::Values(
        // Test cases with 40/40/20 distribution
        DetermineObstacleTypeParams{40, 40, 0, ObstacleType::Grow},   // Lower bound for Grow
        DetermineObstacleTypeParams{40, 40, 39, ObstacleType::Grow},  // Upper bound for Grow
        DetermineObstacleTypeParams{40, 40, 40, ObstacleType::Shrink},// Lower bound for Shrink
        DetermineObstacleTypeParams{40, 40, 79, ObstacleType::Shrink},// Upper bound for Shrink
        DetermineObstacleTypeParams{40, 40, 80, ObstacleType::Hurt},  // Lower bound for Hurt
        DetermineObstacleTypeParams{40, 40, 99, ObstacleType::Hurt},  // Upper bound for Hurt
        // Test cases with 10/10/80 distribution
        DetermineObstacleTypeParams{10, 10, 9, ObstacleType::Grow},
        DetermineObstacleTypeParams{10, 10, 19, ObstacleType::Shrink},
        DetermineObstacleTypeParams{10, 10, 20, ObstacleType::Hurt}
    )
);

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

// --- New Obstacle Creation Tests ---

TEST(ObstacleCreationTest, CreateCheckpoint) {
    // We don't need to test the randomness, just that it creates the right kind of obstacle.
    Obstacle o = Obstacle::createCheckpoint(800, 600, 3, 150); // Pass a fixed gap height for testing
    EXPECT_EQ(o.type, ObstacleType::Checkpoint);
    EXPECT_TRUE(o.rect2.has_value());
    EXPECT_EQ(o.speed, 3);
    EXPECT_EQ(o.rect.x, 800);
}

struct CreateRegularTestParams {
    int grow_chance;
    int shrink_chance;
    int type_roll;
    ObstacleType expected_type;
    std::string description;
};

void PrintTo(const CreateRegularTestParams& params, std::ostream* os) {
    *os << params.description;
}

class CreateRegularTest : public ::testing::TestWithParam<CreateRegularTestParams> {
  protected:
    // This test class can be used to test createRegular by controlling the type roll.
    // We can't fully test createRegular without mocking rand(), but we can test the type logic.
    static Obstacle createRegularWithTypeRoll(int grow, int shrink, int roll) {
        ObstacleType type = determineObstacleType(grow, shrink, roll);
        switch (type) {
            case ObstacleType::Grow:   return Obstacle::createGrowBlock(0, 0, 1);
            case ObstacleType::Shrink: return Obstacle::createShrinkBlock(0, 0, 1);
            case ObstacleType::Hurt:
            default:                   return Obstacle::createHurtBlock(0, 0, 1);
        }
    }
};

TEST_P(CreateRegularTest, CreatesCorrectType) {
    auto params = GetParam();
    Obstacle o = createRegularWithTypeRoll(params.grow_chance, params.shrink_chance, params.type_roll);
    EXPECT_EQ(o.type, params.expected_type);
}

INSTANTIATE_TEST_SUITE_P(
    ObstacleLogicTests,
    CreateRegularTest,
    ::testing::Values(
        CreateRegularTestParams{30, 30, 29, ObstacleType::Grow, "RollIsGrow"},
        CreateRegularTestParams{30, 30, 59, ObstacleType::Shrink, "RollIsShrink"},
        CreateRegularTestParams{30, 30, 60, ObstacleType::Hurt, "RollIsHurt"},
        CreateRegularTestParams{100, 0, 50, ObstacleType::Grow, "AlwaysGrow"},
        CreateRegularTestParams{0, 100, 50, ObstacleType::Shrink, "AlwaysShrink"},
        CreateRegularTestParams{0, 0, 50, ObstacleType::Hurt, "AlwaysHurt"}
    ),
    [](const testing::TestParamInfo<CreateRegularTest::ParamType>& info) {
        return info.param.description;
    }
);

// --- UpdateAndRemove Test ---
struct UpdateAndRemoveParams {
    std::vector<Obstacle> initial_obstacles;
    size_t expected_remaining_count;
    std::multiset<ObstacleType> expected_remaining_types;
    long long expected_x_sum; // Sum of x-coordinates of remaining obstacles
    std::string description;
};

void PrintTo(const UpdateAndRemoveParams& params, std::ostream* os) {
    *os << params.description;
}

class UpdateAndRemoveTest : public ::testing::TestWithParam<UpdateAndRemoveParams> {};

TEST_P(UpdateAndRemoveTest, CorrectlyUpdatesAndRemoves) {
    auto params = GetParam();
    auto obstacles = params.initial_obstacles; // Make a copy to modify
    Obstacle::updateAndRemove(obstacles);

    ASSERT_EQ(obstacles.size(), params.expected_remaining_count);

    std::multiset<ObstacleType> actual_remaining_types;
    long long actual_x_sum = 0;
    for (const auto& o : obstacles) {
        actual_remaining_types.insert(o.type);
        actual_x_sum += o.rect.x;
    }
    EXPECT_EQ(actual_remaining_types, params.expected_remaining_types);
    EXPECT_EQ(actual_x_sum, params.expected_x_sum);
}

INSTANTIATE_TEST_SUITE_P(
    ObstacleLogicTests,
    UpdateAndRemoveTest,
    ::testing::Values(
        UpdateAndRemoveParams{{
                Obstacle(100, 100, 20, 20, 5, ObstacleType::Hurt),
                Obstacle(-30, 100, 20, 20, 5, ObstacleType::Hurt),
                // This obstacle should be removed after one update (10 - 30 = -20; -20 + 20 <= 0)
                Obstacle(10, 100, 20, 20, 30, ObstacleType::Grow),
                Obstacle(SDL_Rect{200, 0, 20, 200}, SDL_Rect{200, 300, 20, 300}, 2)
            }, 2, {ObstacleType::Hurt, ObstacleType::Checkpoint}, 95 + 198, "MixedOnAndOffscreen"},
        UpdateAndRemoveParams{{
                Obstacle(100, 100, 20, 20, 5, ObstacleType::Hurt),
                Obstacle(200, 100, 20, 20, 5, ObstacleType::Grow)
            }, 2, {ObstacleType::Hurt, ObstacleType::Grow}, 95 + 195, "AllOnscreen"},
        UpdateAndRemoveParams{{ Obstacle(-30, 100, 20, 20, 5, ObstacleType::Hurt) }, 0, {}, 0, "AllOffscreen"},
        UpdateAndRemoveParams{{}, 0, {}, 0, "EmptyVector"}
    ),
    [](const testing::TestParamInfo<UpdateAndRemoveTest::ParamType>& info) {
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

    handleCollision(player, it, obstacles, running);

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
        CollisionLogicParams{ObstacleType::Checkpoint, false, 1, PlayerSizeChange::EQUAL, "CheckpointCollision"}
    ),
    [](const testing::TestParamInfo<CollisionLogicTest::ParamType>& info) {
        return info.param.description;
    }
);

TEST_F(SdlTest, ConfigDefaultConstructor) {
    // This test relies on the IS_TEST_BUILD macro being set correctly
    // to find the config file from the test executable's path.
    Config config;

    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 128);
    EXPECT_EQ(player_color.g, 0);
    EXPECT_EQ(player_color.b, 128);
}

// --- Obstacle Batching Test ---
struct PrepareObstacleBatchesParams {
    std::vector<ObstacleType> obstacle_types;
    size_t expected_hurt;
    size_t expected_grow;
    size_t expected_shrink;
    size_t expected_checkpoints;
    std::string description;
};

void PrintTo(const PrepareObstacleBatchesParams& params, std::ostream* os) {
    *os << params.description;
}

class PrepareObstacleBatchesTest : public ::testing::TestWithParam<PrepareObstacleBatchesParams> {};

TEST_P(PrepareObstacleBatchesTest, CorrectlyBatchesObstacles) {
    auto params = GetParam();
    std::vector<Obstacle> obstacles;
    for (const auto& type : params.obstacle_types) {
        if (type == ObstacleType::Checkpoint) {
            obstacles.emplace_back(SDL_Rect{0,0,10,10}, SDL_Rect{0,0,10,10}, 1);
        } else {
            obstacles.emplace_back(0, 0, 10, 10, 1, type);
        }
    }

    std::vector<SDL_Rect> hurt_rects;
    std::vector<SDL_Rect> grow_rects;
    std::vector<SDL_Rect> shrink_rects;
    std::vector<SDL_Rect> checkpoint_rects;

    prepareObstacleBatches(obstacles, hurt_rects, grow_rects, shrink_rects, checkpoint_rects);
    EXPECT_EQ(hurt_rects.size(), params.expected_hurt);
    EXPECT_EQ(grow_rects.size(), params.expected_grow);
    EXPECT_EQ(shrink_rects.size(), params.expected_shrink);
    // Each checkpoint obstacle consists of two rectangles
    EXPECT_EQ(checkpoint_rects.size(), params.expected_checkpoints * 2);
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    PrepareObstacleBatchesTest,
    ::testing::Values(
        PrepareObstacleBatchesParams{{ObstacleType::Hurt, ObstacleType::Grow, ObstacleType::Grow, ObstacleType::Shrink, ObstacleType::Shrink, ObstacleType::Shrink}, 1, 2, 3, 0, "MixedObstacles"},
        PrepareObstacleBatchesParams{{}, 0, 0, 0, 0, "EmptyVector"},
        PrepareObstacleBatchesParams{{ObstacleType::Hurt, ObstacleType::Hurt, ObstacleType::Hurt}, 3, 0, 0, 0, "AllHurt"},
        PrepareObstacleBatchesParams{{ObstacleType::Grow, ObstacleType::Grow}, 0, 2, 0, 0, "AllGrow"},
        PrepareObstacleBatchesParams{{ObstacleType::Shrink}, 0, 0, 1, 0, "AllShrink"},
        PrepareObstacleBatchesParams{{ObstacleType::Checkpoint}, 0, 0, 0, 1, "OneCheckpoint"},
        PrepareObstacleBatchesParams{{ObstacleType::Checkpoint, ObstacleType::Hurt, ObstacleType::Checkpoint}, 1, 0, 0, 2, "MixedWithCheckpoints"}
    ),
    [](const testing::TestParamInfo<PrepareObstacleBatchesTest::ParamType>& info) {
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
    const int initial_w = 40;
    const int initial_h = 40;
    Player player(params.player_x, 100, initial_w, initial_h, 5, {0,0,0,0});
    // Grow the player to check if size is reset on checkpoint pass
    player.grow(20);
    ASSERT_EQ(player.rect.w, initial_w + 20);

    Obstacle obstacle(params.obstacle_x, 100, 20, 20, 3, params.obstacle_type);
    if (params.obstacle_type == ObstacleType::Checkpoint) {
        obstacle = Obstacle({params.obstacle_x, 0, 20, 100}, {params.obstacle_x, 200, 20, 100}, 3);
    }
    obstacle.passed = params.obstacle_initially_passed;
    int score = params.initial_score;

    handleCheckpointPassing(player, obstacle, score);

    EXPECT_EQ(score, params.expected_score);
    EXPECT_EQ(obstacle.passed, params.expected_passed_state);

    bool should_reset = params.obstacle_type == ObstacleType::Checkpoint &&
                        !params.obstacle_initially_passed &&
                        player.rect.x > obstacle.rect.x + obstacle.rect.w;

    EXPECT_EQ(player.rect.w, should_reset ? initial_w : initial_w + 20);
}

INSTANTIATE_TEST_SUITE_P(
    GameLogicTests,
    CheckpointPassingTest,
    ::testing::Values(
        CheckpointPassingParams{100, 121, false, 0, 0, false, ObstacleType::Checkpoint, "PlayerBeforeCheckpoint"},
        CheckpointPassingParams{121, 100, false, 0, 10, true, ObstacleType::Checkpoint, "PlayerPassesCheckpoint"},
        CheckpointPassingParams{122, 100, false, 10, 20, true, ObstacleType::Checkpoint, "PlayerPassesCheckpointWithScore"},
        CheckpointPassingParams{121, 100, true, 10, 10, true, ObstacleType::Checkpoint, "PlayerPassesAlreadyPassedCheckpoint"},
        CheckpointPassingParams{100, 100, false, 0, 0, false, ObstacleType::Checkpoint, "PlayerAtCheckpointEdge"},
        CheckpointPassingParams{121, 100, false, 0, 0, false, ObstacleType::Hurt, "DoesNotAffectNonCheckpoints"}
    ),
    [](const testing::TestParamInfo<CheckpointPassingTest::ParamType>& info) {
        return info.param.description;
    }
);

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

class ObstacleSpawnerTest : public ::testing::TestWithParam<ObstacleSpawnerParams> {};

TEST_P(ObstacleSpawnerTest, SpawnsCorrectlyOverTime) {
    auto params = GetParam();
    ObstacleSpawner spawner(params.regular_interval, params.checkpoint_interval, 800, 600, 3, 50, 50, 120);
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
        ObstacleSpawnerParams{1500, 60000, {0, 1500, 1501, 1502, 3001, 3002}, 2, 0, "SpawnsOnlyRegular"},
        ObstacleSpawnerParams{100, 200, {101, 201}, 1, 1, "SpawnsBothRegularAndCheckpoint"},
        ObstacleSpawnerParams{100, 200, {0, 50, 99}, 0, 0, "NoSpawnsBeforeInterval"},
        ObstacleSpawnerParams{100, 200, {100, 199, 200}, 1, 1, "SpawnsAtAndAfterInterval"},
        ObstacleSpawnerParams{1000, 200, {201}, 0, 1, "SpawnsOnlyCheckpoint"}
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
    static const int SCREEN_WIDTH = 800;
    static const int SCREEN_HEIGHT = 600;
    static const int OBSTACLE_SPEED = 3;
    static const int CHECKPOINT_INTERVAL = 1000;

    ObstacleSpawner spawner{500, CHECKPOINT_INTERVAL, SCREEN_WIDTH, SCREEN_HEIGHT, OBSTACLE_SPEED, 50, 50, 120};
};

TEST_P(CheckpointGapCalculationTest, CalculatesCorrectGapSize) {
    auto params = GetParam();
    spawner.shrink_powerups_since_checkpoint = params.shrink_block_history;

    const int actual_gap = spawner.calculateCheckpointGapSize();
    EXPECT_EQ(actual_gap, params.expected_gap);
}

INSTANTIATE_TEST_SUITE_P(
    GapCalculationTests,
    CheckpointGapCalculationTest,
    ::testing::Values(
        ObstacleSpawnerGapParams{{}, 120, "IsBaseWhenNoPowerups"},
        // 1 shrink -> 120 - 1*10 = 110
        ObstacleSpawnerGapParams{{1}, 110, "DecreasesWithOneShrinkBlock"},
        // 2 shrinks -> 120 - 2*10 = 100
        ObstacleSpawnerGapParams{{1, 1}, 100, "DecreasesWithTwoShrinkBlocks"},
        // 11 shrinks -> 120 - 11*10 = 10. Clamped to min_gap_height (15), so 15.
        ObstacleSpawnerGapParams{std::vector<int>(11), 15, "ClampsAtMinimum"}
    ),
    [](const testing::TestParamInfo<CheckpointGapCalculationTest::ParamType>& info) {
        return info.param.description;
    }
);

// This test is separate because it has a different structure (multiple spawns).
class ObstacleSpawnerStateTest : public ::testing::Test {
protected:
    static const int SCREEN_WIDTH = 800;
    static const int SCREEN_HEIGHT = 600;
    static const int OBSTACLE_SPEED = 3;
    static const int CHECKPOINT_INTERVAL = 1000;

    ObstacleSpawner spawner{500, CHECKPOINT_INTERVAL, SCREEN_WIDTH, SCREEN_HEIGHT, OBSTACLE_SPEED, 50, 50, 120};
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

    const int expected_gap = 120; // base_gap_height
    const int actual_gap = checkpoint2.rect2->y - checkpoint2.rect.h;
    EXPECT_EQ(actual_gap, expected_gap);
}
