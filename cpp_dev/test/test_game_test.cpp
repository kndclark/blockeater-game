#include <gtest/gtest.h>
#include <SDL2/SDL.h>

// Helper function for collision detection
bool check_collision(const SDL_Rect& a, const SDL_Rect& b) {
    return SDL_HasIntersection(&a, &b);
}

TEST(GameTest, PlayerDoesNotCollideInitially) {
    SDL_Rect player = {320, 240, 40, 40};
    SDL_Rect obstacle = {200, 150, 60, 60};
    EXPECT_FALSE(check_collision(player, obstacle));
}

TEST(GameTest, PlayerCollidesWhenOverlapping) {
    SDL_Rect player = {200, 150, 60, 60};
    SDL_Rect obstacle = {200, 150, 60, 60};
    EXPECT_TRUE(check_collision(player, obstacle));
}

TEST(GameTest, PlayerMovesCorrectly) {
    int player_x = 320, player_y = 240;
    int player_speed = 5;
    // Simulate moving left
    player_x -= player_speed;
    EXPECT_EQ(player_x, 315);
    // Simulate moving right
    player_x += player_speed * 2;
    EXPECT_EQ(player_x, 325);
    // Simulate moving up
    player_y -= player_speed;
    EXPECT_EQ(player_y, 235);
    // Simulate moving down
    player_y += player_speed * 2;
    EXPECT_EQ(player_y, 245);
}

TEST(GameTest, CollisionResponseMovesPlayerBack) {
    int player_x = 200, player_y = 150;
    int player_w = 60, player_h = 60;
    int player_speed = 5;
    SDL_Rect obstacle = {200, 150, 60, 60};
    SDL_Rect player_rect = {player_x, player_y, player_w, player_h};
    // Simulate moving right into obstacle
    player_x += player_speed;
    player_rect.x = player_x;
    if (check_collision(player_rect, obstacle)) {
        player_x -= player_speed; // Move back
    }
    EXPECT_EQ(player_x, 200);
}