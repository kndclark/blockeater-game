#include <gtest/gtest.h>
#include <vector>
#include <numeric>
#include <set>
#include <optional>
#include <utility>
#include <set>
#include "../config/Config.h"
#include "../src/Obstacle.h"
#include "../src/GameLogic.h"
#include "test_helpers.h"

// Test suite for the Obstacle class
TEST(ObstacleTest, Creation) {
    Obstacle obstacle(50, 60, 70, 80, 3, ObstacleType::Hurt, 100);
    EXPECT_EQ(obstacle.rect.x, 50);
    EXPECT_EQ(obstacle.rect.y, 60);
    EXPECT_EQ(obstacle.rect.w, 70);
    EXPECT_EQ(obstacle.rect.h, 80);
    EXPECT_EQ(obstacle.speed, 3);
    EXPECT_EQ(obstacle.type, ObstacleType::Hurt);
    EXPECT_EQ(obstacle.points, 100);
    EXPECT_FALSE(obstacle.rect2.has_value());
}

TEST(ObstacleCreationTest, CreateCheckpoint) {
    // We don't need to test the randomness, just that it creates the right kind of obstacle.
    int dummy_gap_y;
    std::vector<Obstacle> nearby;
    Obstacle o = Obstacle::createCheckpoint(800, 600, 3, 150, 50, nearby, dummy_gap_y); // Pass a fixed gap height for testing
    EXPECT_EQ(o.type, ObstacleType::Checkpoint);
    EXPECT_EQ(o.points, 50);
    EXPECT_TRUE(o.rect2.has_value());
    EXPECT_EQ(o.speed, 3);
    EXPECT_EQ(o.rect.x, 800);
}

TEST(ObstacleTest, Update) {
    Obstacle obstacle(100, 100, 50, 50, 3, ObstacleType::Hurt, 0);
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 97); //speed = 3, 100 - 3 = 97
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 94);
}

TEST(ObstacleTest, IsOffscreen) {
    // Obstacle fully on screen
    EXPECT_FALSE(Obstacle(10, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Obstacle touching left edge
    EXPECT_FALSE(Obstacle(0, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Obstacle partially offscreen
    EXPECT_FALSE(Obstacle(-10, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Obstacle fully offscreen (right edge at x=0)
    EXPECT_TRUE(Obstacle(-20, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
}

TEST(ObstacleTest, TypeAssignment) {
    Obstacle hurt_obstacle(0, 0, 10, 10, 1, ObstacleType::Hurt, 0);
    EXPECT_EQ(hurt_obstacle.type, ObstacleType::Hurt);
    EXPECT_EQ(hurt_obstacle.points, 0);

    Obstacle grow_obstacle(0, 0, 10, 10, 1, ObstacleType::Grow, 200);
    EXPECT_EQ(grow_obstacle.type, ObstacleType::Grow);
    EXPECT_EQ(grow_obstacle.points, 200);

    Obstacle shrink_obstacle(0, 0, 10, 10, 1, ObstacleType::Shrink, 100);
    EXPECT_EQ(shrink_obstacle.type, ObstacleType::Shrink);
    EXPECT_EQ(shrink_obstacle.points, 100);

    Obstacle checkpoint_obstacle({0,0,0,0}, {0,0,0,0}, 1, 10);
    EXPECT_EQ(checkpoint_obstacle.type, ObstacleType::Checkpoint);
    EXPECT_EQ(checkpoint_obstacle.points, 10);
}

TEST(ObstacleCreationTest, CreateRegularWithPoints) {
    Config config(kTestRootPath);
    ObstacleConfig obs_config = config.getObstacleConfig();
    ASSERT_EQ(obs_config.grow_points, 200);
    ASSERT_EQ(obs_config.shrink_points, 100);

    // Force a "Grow" obstacle by setting shrink/hurt chances to 0
    obs_config.grow_chance = 100;
    obs_config.shrink_chance = 0;
    std::vector<Obstacle> nearby;
    Obstacle grow_obstacle = Obstacle::createRegular(800, 600, 3, obs_config, nearby);
    EXPECT_EQ(grow_obstacle.type, ObstacleType::Grow);
    EXPECT_EQ(grow_obstacle.points, 200);

    // Force a "Shrink" obstacle
    obs_config.grow_chance = 0;
    obs_config.shrink_chance = 100;
    Obstacle shrink_obstacle = Obstacle::createRegular(800, 600, 3, obs_config, nearby);
    EXPECT_EQ(shrink_obstacle.type, ObstacleType::Shrink);
    EXPECT_EQ(shrink_obstacle.points, 100);
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
            obstacles.emplace_back(SDL_Rect{0,0,10,10}, SDL_Rect{0,0,10,10}, 1, 10);
        } else {
            obstacles.emplace_back(0, 0, 10, 10, 1, type, 0);
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

// --- Obstacle Placement Test ---
// Test to ensure obstacles are not placed inside the safe gap.
TEST(ObstaclePlacementTest, CalculateSafeY_AvoidsGap) {
    const int screen_height = 600;
    const int obstacle_height = 50;
    // Checkpoint with a gap from y=200 to y=400. The walls are forbidden.
    std::vector<Obstacle> nearby_obstacles = { Obstacle({0, 0, 10, 200}, {0, 400, 10, 200}, 1, 0) };

    const int num_trials = 1000;
    for (int i = 0; i < num_trials; ++i) {
        int y = Obstacle::calculateSafeY(screen_height, obstacle_height, nearby_obstacles);
        // The forbidden zones are the walls [0, 200] and [400, 600].
        // The new obstacle must not overlap with them.
        EXPECT_FALSE((y < 200 && y + obstacle_height > 0) || (y < 600 && y + obstacle_height > 400))
            << "Obstacle at y=" << y << " overlaps with checkpoint walls.";
    }
}

TEST(ObstaclePlacementTest, CalculateSafeY_AvoidsLastObstacle) {
    const int screen_height = 600;
    const int obstacle_height = 50;
    const int clearance = 50;

    // Place a previous obstacle in the middle of the screen
    const SDL_Rect last_obstacle_rect = {0, 275, 50, 50};
    std::vector<Obstacle> nearby_obstacles = { Obstacle(last_obstacle_rect.x, last_obstacle_rect.y, last_obstacle_rect.w, last_obstacle_rect.h, 1, ObstacleType::Hurt, 0) };

    const int num_trials = 1000;
    for (int i = 0; i < num_trials; ++i) {
        int y = Obstacle::calculateSafeY(screen_height, obstacle_height, nearby_obstacles);

        // Check if the new obstacle's y-range overlaps with the last obstacle's clearance zone.
        bool overlaps = (y < last_obstacle_rect.y + last_obstacle_rect.h + clearance) &&
                        (y + obstacle_height > last_obstacle_rect.y - clearance);

        EXPECT_FALSE(overlaps) << "Obstacle spawned at y=" << y << " overlaps with last obstacle at y=" << last_obstacle_rect.y;
    }
}

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
                Obstacle(100, 100, 20, 20, 5, ObstacleType::Hurt, 0),
                Obstacle(-30, 100, 20, 20, 5, ObstacleType::Hurt, 0),
                // This obstacle should be removed after one update (10 - 30 = -20; -20 + 20 <= 0)
                Obstacle(10, 100, 20, 20, 30, ObstacleType::Grow, 0),
                Obstacle(SDL_Rect{200, 0, 20, 200}, SDL_Rect{200, 300, 20, 300}, 2, 10)
            }, 2, {ObstacleType::Hurt, ObstacleType::Checkpoint}, 95 + 198, "MixedOnAndOffscreen"},
        UpdateAndRemoveParams{{
                Obstacle(100, 100, 20, 20, 5, ObstacleType::Hurt, 0),
                Obstacle(200, 100, 20, 20, 5, ObstacleType::Grow, 0)
            }, 2, {ObstacleType::Hurt, ObstacleType::Grow}, 95 + 195, "AllOnscreen"},
        UpdateAndRemoveParams{{Obstacle(-30, 100, 20, 20, 5, ObstacleType::Hurt, 0)}, 0, {}, 0, "AllOffscreen"},
        UpdateAndRemoveParams{{Obstacle(100, 100, 20, 20, 5, ObstacleType::Hurt, 0), Obstacle(10, 100, 20, 20, 30, ObstacleType::Grow, 0)}, 1, {ObstacleType::Hurt}, 95, "RemoveLastElement"},
        UpdateAndRemoveParams{{}, 0, {}, 0, "EmptyVector"}
    ),
    [](const testing::TestParamInfo<UpdateAndRemoveTest::ParamType>& info) {
        return info.param.description;
    }
);