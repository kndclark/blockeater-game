#include <gtest/gtest.h>
#include "../src/Player.h"
#include "../src/Obstacle.h"
#include "../config/Config.h"
#include "test_helpers.h"

// Use the fixture for the collision detection test
TEST_F(SdlTest, CollisionDetection) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    Config config(kTestRootPath);
    Player player({100, 100, 40, 40, 5, {0,0,0,0},
                   config.getDashSpeedMultiplier(), config.getDashDurationMs(),
                   config.getDashCooldownMs()
                  });

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
    // NOLINTNEXTLINE(readability-magic-numbers)
    Config config(kTestRootPath);
    Player player({100, 100, 40, 40, 5, {0,0,0,0},
                   config.getDashSpeedMultiplier(), config.getDashDurationMs(),
                   config.getDashCooldownMs()
                  });

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