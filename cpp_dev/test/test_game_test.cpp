#include <gtest/gtest.h>
#include <fstream> // For std::ofstream in ConfigTest
#include <vector>
#include "../src/Player.h"
#include "../src/Obstacle.h"
#include "../src/Color.h"
#include "../src/Config.h"
#include "../src/GameLogic.h"

namespace {
const std::string kTestColorConfigPath = "../../config/colors.json";
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
}

TEST(PlayerTest, Movement) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);
    const int screen_width = 640;
    const int screen_height = 480;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};

    // Test moving left
    keystate[SDL_SCANCODE_LEFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.x, 95);
    keystate[SDL_SCANCODE_LEFT] = 0;

    // Test moving right
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.x, 100);
    keystate[SDL_SCANCODE_RIGHT] = 0;

    // Test moving up
    keystate[SDL_SCANCODE_UP] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.y, 95);
    keystate[SDL_SCANCODE_UP] = 0;

    // Test moving down
    keystate[SDL_SCANCODE_DOWN] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.y, 100);
}

TEST(PlayerTest, BoundaryCollision) {
    const int screen_width = 640;
    const int screen_height = 480;
    const int player_size = 40;
    const int player_speed = 5;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};
    Color c = {0,0,0,0};

    // Test left boundary
    Player player_left(0, 100, player_size, player_size, player_speed, c);
    keystate[SDL_SCANCODE_LEFT] = 1;
    player_left.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_left.rect.x, 0);
    keystate[SDL_SCANCODE_LEFT] = 0;

    // Test right boundary
    Player player_right(screen_width - player_size, 100, player_size, player_size, player_speed, c);
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player_right.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_right.rect.x, screen_width - player_size);
    keystate[SDL_SCANCODE_RIGHT] = 0;

    // Test top boundary
    Player player_top(100, 0, player_size, player_size, player_speed, c);
    keystate[SDL_SCANCODE_UP] = 1;
    player_top.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_top.rect.y, 0);
    keystate[SDL_SCANCODE_UP] = 0;

    // Test bottom boundary
    Player player_bottom(100, screen_height - player_size, player_size, player_size, player_speed, c);
    keystate[SDL_SCANCODE_DOWN] = 1;
    player_bottom.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_bottom.rect.y, screen_height - player_size);
}

TEST(PlayerTest, SizeModification) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);

    // Test growing
    player.grow(10);
    EXPECT_EQ(player.rect.w, 50);
    EXPECT_EQ(player.rect.h, 50);

    // Test shrinking
    player.shrink(20);
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 30);

    // Test shrinking below the minimum size (should clamp to 10)
    player.shrink(30);
    EXPECT_EQ(player.rect.w, 10);
    EXPECT_EQ(player.rect.h, 10);
}

// Test suite for the Obstacle class
TEST(ObstacleTest, Creation) {
    Color c = {0,0,0,0};
    Obstacle obstacle(50, 60, 70, 80, 3, ObstacleType::Hurt, c);
    EXPECT_EQ(obstacle.rect.x, 50);
    EXPECT_EQ(obstacle.rect.y, 60);
    EXPECT_EQ(obstacle.rect.w, 70);
    EXPECT_EQ(obstacle.rect.h, 80);
    EXPECT_EQ(obstacle.speed, 3);
    EXPECT_EQ(obstacle.type, ObstacleType::Hurt);
}

TEST(ObstacleTest, Update) {
    Color c = {0,0,0,0};
    Obstacle obstacle(100, 100, 50, 50, 3, ObstacleType::Hurt, c);
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 97); //speed = 3, 100 - 3 = 97
    obstacle.update();
    EXPECT_EQ(obstacle.rect.x, 94);
}

TEST(ObstacleTest, TypeAssignment) {
    Color c = {0,0,0,0};
    Obstacle hurt_obstacle(0, 0, 10, 10, 1, ObstacleType::Hurt, c);
    EXPECT_EQ(hurt_obstacle.type, ObstacleType::Hurt);

    Obstacle grow_obstacle(0, 0, 10, 10, 1, ObstacleType::Grow, c);
    EXPECT_EQ(grow_obstacle.type, ObstacleType::Grow);

    Obstacle shrink_obstacle(0, 0, 10, 10, 1, ObstacleType::Shrink, c);
    EXPECT_EQ(shrink_obstacle.type, ObstacleType::Shrink);
}

TEST(ObstacleTest, IsOffscreen) {
    Color c = {0,0,0,0};
    // Obstacle fully on screen
    EXPECT_FALSE(Obstacle(10, 10, 20, 20, 1, ObstacleType::Hurt, c).is_offscreen());
    // Obstacle touching left edge
    EXPECT_FALSE(Obstacle(0, 10, 20, 20, 1, ObstacleType::Hurt, c).is_offscreen());
    // Obstacle partially offscreen
    EXPECT_FALSE(Obstacle(-10, 10, 20, 20, 1, ObstacleType::Hurt, c).is_offscreen());
    // Obstacle fully offscreen (right edge at x=0)
    EXPECT_TRUE(Obstacle(-20, 10, 20, 20, 1, ObstacleType::Hurt, c).is_offscreen());
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

// A test fixture for collision detection tests.
class CollisionTest : public SdlTest {};

// Use the fixture for the collision detection test
TEST_F(CollisionTest, Detection) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);

    // No collision
    Obstacle no_collision(200, 200, 20, 20, 3, ObstacleType::Hurt, c);
    EXPECT_FALSE(SDL_HasIntersection(&player.rect, &no_collision.rect));

    // Collision
    Obstacle collision(110, 110, 40, 40, 3, ObstacleType::Hurt, c);
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &collision.rect));

    // Edge collision (intersecting by 1 pixel)
    Obstacle edge_collision(139, 100, 20, 20, 3, ObstacleType::Hurt, c);
    EXPECT_TRUE(SDL_HasIntersection(&player.rect, &edge_collision.rect));
}

// A test fixture for collision logic tests.
class CollisionLogicTest : public SdlTest {};

// A test fixture for tests that create temporary files.
class ConfigFileTest : public ::testing::Test {
protected:
    const std::string malformed_filename = "malformed.json";

    void TearDown() override {
        std::remove(malformed_filename.c_str());
    }
};

// Test suite for the Config class
TEST(ConfigTest, LoadsColorsFromFile) {
    // The test executable runs from the `test/build` directory, so we navigate up.
    Config config(kTestColorConfigPath);

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

    // Should fall back to the hardcoded gray defaults defined in Config.cpp
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

// Test suite for GameLogic functions
TEST(GameLogicTest, DetermineObstacleType) {
    const int grow_chance = 40;
    const int shrink_chance = 40;
    // Hurt chance is implicitly 20

    // Test Grow range
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 0), ObstacleType::Grow);
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 39), ObstacleType::Grow);

    // Test Shrink range
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 40), ObstacleType::Shrink);
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 79), ObstacleType::Shrink);

    // Test Hurt range
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 80), ObstacleType::Hurt);
    EXPECT_EQ(determineObstacleType(grow_chance, shrink_chance, 99), ObstacleType::Hurt);

    // Test with different percentages
    EXPECT_EQ(determineObstacleType(10, 10, 9), ObstacleType::Grow);
    EXPECT_EQ(determineObstacleType(10, 10, 19), ObstacleType::Shrink);
    EXPECT_EQ(determineObstacleType(10, 10, 20), ObstacleType::Hurt);
}

TEST_F(CollisionLogicTest, HandleHurtCollision) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);
    std::vector<Obstacle> obstacles;
    obstacles.emplace_back(100, 100, 20, 20, 1, ObstacleType::Hurt, c);

    auto it = obstacles.begin();
    bool running = true;

    handleCollision(player, it, obstacles, running);

    EXPECT_FALSE(running);
    EXPECT_EQ(obstacles.size(), 1); // Obstacle is not removed
    EXPECT_EQ(it, obstacles.end()); // Iterator is advanced
}

TEST_F(CollisionLogicTest, HandleGrowCollision) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);
    std::vector<Obstacle> obstacles;
    obstacles.emplace_back(100, 100, 20, 20, 1, ObstacleType::Grow, c);

    auto it = obstacles.begin();
    bool running = true;
    int initial_width = player.rect.w;

    handleCollision(player, it, obstacles, running);

    EXPECT_TRUE(running);
    EXPECT_GT(player.rect.w, initial_width);
    EXPECT_EQ(obstacles.size(), 0); // Obstacle is removed
    EXPECT_EQ(it, obstacles.end()); // Iterator points to end after erase
}

TEST_F(CollisionLogicTest, HandleShrinkCollision) {
    Color c = {0,0,0,0};
    Player player(100, 100, 40, 40, 5, c);
    std::vector<Obstacle> obstacles;
    obstacles.emplace_back(100, 100, 20, 20, 1, ObstacleType::Shrink, c);

    auto it = obstacles.begin();
    bool running = true;
    int initial_width = player.rect.w;

    handleCollision(player, it, obstacles, running);

    EXPECT_TRUE(running);
    EXPECT_LT(player.rect.w, initial_width);
    EXPECT_EQ(obstacles.size(), 0); // Obstacle is removed
    EXPECT_EQ(it, obstacles.end()); // Iterator points to end after erase
}

TEST_F(SdlTest, ConfigDefaultConstructor) {
    // This test relies on the IS_TEST_BUILD macro being set correctly
    // to find the config file from the test executable's path.
    Config config;

    Color player_color = config.getPlayerColor();
    EXPECT_EQ(player_color.r, 128);
    EXPECT_EQ(player_color.g, 0);
    EXPECT_EQ(player_color.b, 128);
}
