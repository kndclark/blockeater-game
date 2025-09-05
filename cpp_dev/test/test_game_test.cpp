#include <gtest/gtest.h>
#include "../src/Player.h"
#include "../src/Obstacle.h"

// Test suite for the Player class
TEST(PlayerTest, Creation) {
    Player player(10, 20, 30, 40, 5);
    EXPECT_EQ(player.rect.x, 10);
    EXPECT_EQ(player.rect.y, 20);
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 40);
    EXPECT_EQ(player.speed, 5);
}

TEST(PlayerTest, Movement) {
    Player player(100, 100, 40, 40, 5);
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

    // Test left boundary
    Player player_left(0, 100, player_size, player_size, player_speed);
    keystate[SDL_SCANCODE_LEFT] = 1;
    player_left.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_left.rect.x, 0);
    keystate[SDL_SCANCODE_LEFT] = 0;

    // Test right boundary
    Player player_right(screen_width - player_size, 100, player_size, player_size, player_speed);
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player_right.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_right.rect.x, screen_width - player_size);
    keystate[SDL_SCANCODE_RIGHT] = 0;

    // Test top boundary
    Player player_top(100, 0, player_size, player_size, player_speed);
    keystate[SDL_SCANCODE_UP] = 1;
    player_top.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_top.rect.y, 0);
    keystate[SDL_SCANCODE_UP] = 0;

    // Test bottom boundary
    Player player_bottom(100, screen_height - player_size, player_size, player_size, player_speed);
    keystate[SDL_SCANCODE_DOWN] = 1;
    player_bottom.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player_bottom.rect.y, screen_height - player_size);
}

TEST(PlayerTest, SizeModification) {
    Player player(100, 100, 40, 40, 5);

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
    Obstacle obstacle(50, 60, 70, 80, 3, ObstacleType::Hurt);
    EXPECT_EQ(obstacle.rect.x, 50);
    EXPECT_EQ(obstacle.rect.y, 60);
    EXPECT_EQ(obstacle.rect.w, 70);
    EXPECT_EQ(obstacle.rect.h, 80);
    EXPECT_EQ(obstacle.speed, 3);
    EXPECT_EQ(obstacle.type, ObstacleType::Hurt);
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
class CollisionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // We need to initialize SDL to use its functions like SDL_HasIntersection
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0) << "Failed to initialize SDL: " << SDL_GetError();
    }

    void TearDown() override {
        SDL_Quit();
    }
};

// Use the fixture for the collision detection test
TEST_F(CollisionTest, Detection) {
    Player player(100, 100, 40, 40, 5);

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