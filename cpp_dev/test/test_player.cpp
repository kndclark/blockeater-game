#include <gtest/gtest.h>
#include "../src/Player.h"
#include "test_helpers.h"
// Consolidate movement and boundary tests into one parameterized test
struct PlayerMovementParams {
    int start_x;
    int start_y;
    SDL_Scancode key;
    int expected_x;
    int expected_y;
    std::string description;
};

class PlayerMovementTest : public SdlTest, public ::testing::WithParamInterface<PlayerMovementParams> {};

TEST_P(PlayerMovementTest, HandlesMovementCorrectly) {
    auto params = GetParam();
    Color c = {0,0,0,0};
    Player player(params.start_x, params.start_y, 40, 40, 5, c);
    const int screen_width = 640;
    const int screen_height = 480;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};

    keystate[params.key] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.rect.x, params.expected_x);
    EXPECT_EQ(player.rect.y, params.expected_y);
}

void PrintTo(const PlayerMovementParams& params, std::ostream* os) {
    *os << params.description;
}


INSTANTIATE_TEST_SUITE_P(
    PlayerTests,
    PlayerMovementTest,
    ::testing::Values(
        // Movement tests
        PlayerMovementParams{100, 100, SDL_SCANCODE_LEFT, 95, 100, "MovesLeft"},
        PlayerMovementParams{100, 100, SDL_SCANCODE_RIGHT, 105, 100, "MovesRight"},
        PlayerMovementParams{100, 100, SDL_SCANCODE_UP, 100, 95, "MovesUp"},
        PlayerMovementParams{100, 100, SDL_SCANCODE_DOWN, 100, 105, "MovesDown"},
        // Boundary tests (expected position is the same as start)
        PlayerMovementParams{0, 100, SDL_SCANCODE_LEFT, 0, 100, "StopsAtLeftBoundary"},
        PlayerMovementParams{640 - 40, 100, SDL_SCANCODE_RIGHT, 640 - 40, 100, "StopsAtRightBoundary"},
        PlayerMovementParams{100, 0, SDL_SCANCODE_UP, 100, 0, "StopsAtTopBoundary"},
        PlayerMovementParams{100, 480 - 40, SDL_SCANCODE_DOWN, 100, 480 - 40, "StopsAtBottomBoundary"}
    ),
    [](const testing::TestParamInfo<PlayerMovementTest::ParamType>& info) {
        return info.param.description;
    }
);

class PlayerDashTest : public SdlTest {
protected:
    // Use a speed of 10 for easier math in tests
    Player player{100, 100, 40, 40, 10, {0,0,0,0}};
    const int screen_width = 640;
    const int screen_height = 480;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};
};

TEST_F(PlayerDashTest, DashActivatesAndIncreasesSpeed) {
    ASSERT_FALSE(player.is_dashing);
    ASSERT_FALSE(player.on_cooldown);

    // Press Shift and Right arrow to dash right
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    // Check state
    EXPECT_TRUE(player.is_dashing);
    EXPECT_FALSE(player.on_cooldown);

    // Check position change
    int expected_x = 100 + static_cast<int>(10 * Player::DASH_SPEED_MULTIPLIER);
    EXPECT_EQ(player.rect.x, expected_x);
}

TEST_F(PlayerDashTest, DashForwardWithNoDirectionalInput) {
    ASSERT_FALSE(player.is_dashing);
    ASSERT_FALSE(player.on_cooldown);

    // Press Shift with no direction to dash forward
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    EXPECT_TRUE(player.is_dashing);
    int expected_x = 100 + static_cast<int>(10 * Player::DASH_SPEED_MULTIPLIER);
    EXPECT_EQ(player.rect.x, expected_x);
}

TEST_F(PlayerDashTest, DashEndsAndEntersCooldown) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    ASSERT_TRUE(player.is_dashing);

    // Wait for dash to end
    // Simulate time passing
    player.update(player.dash_start_time + Player::DASH_DURATION_MS + 1);

    EXPECT_FALSE(player.is_dashing);
    EXPECT_TRUE(player.on_cooldown);
}

TEST_F(PlayerDashTest, CannotDashWhileDashing) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    ASSERT_TRUE(player.is_dashing);
    Uint32 dash_start_time = player.dash_start_time;

    // Try to dash again immediately while holding the key
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_TRUE(player.is_dashing);
    // The dash start time should not have been reset
    EXPECT_EQ(player.dash_start_time, dash_start_time);
}

TEST_F(PlayerDashTest, CannotDashOnCooldown) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    keystate[SDL_SCANCODE_LSHIFT] = 0; // Release key

    // End dash and start cooldown by simulating time
    player.update(player.dash_start_time + Player::DASH_DURATION_MS + 1);
    ASSERT_TRUE(player.on_cooldown);
    ASSERT_FALSE(player.is_dashing);

    // Try to dash again while on cooldown
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    // Should still be on cooldown, not dashing
    EXPECT_FALSE(player.is_dashing);
    EXPECT_TRUE(player.on_cooldown);
}

TEST_F(PlayerDashTest, DashBecomesAvailableAfterCooldown) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    keystate[SDL_SCANCODE_LSHIFT] = 0;

    // Simulate time passing to end the dash and start the cooldown
    Uint32 cooldown_start_time = player.dash_start_time + Player::DASH_DURATION_MS + 1;
    player.update(cooldown_start_time);
    ASSERT_TRUE(player.on_cooldown);

    // Simulate time passing for the cooldown to end
    player.update(cooldown_start_time + Player::DASH_COOLDOWN_MS + 1);

    // Cooldown should be over
    EXPECT_FALSE(player.on_cooldown);
    EXPECT_FALSE(player.is_dashing);

    // Dash should be available again
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_TRUE(player.is_dashing);
}

TEST(PlayerTest, CreationAndSizeModification) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    Player player(10, 20, 30, 40, 5, {0,0,0,0});
    EXPECT_EQ(player.rect.x, 10);
    EXPECT_EQ(player.rect.y, 20);
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 40);
    EXPECT_EQ(player.speed, 5);
    EXPECT_EQ(player.default_w, 30);
    EXPECT_EQ(player.default_h, 40);

    // Test growing
    player.grow(10); // w: 30->40, h: 40->50
    EXPECT_EQ(player.rect.w, 40);
    EXPECT_EQ(player.rect.h, 50);

    // Test shrinking from grown state
    player.shrink(20); // w: 40->20, h: 50->30
    EXPECT_EQ(player.rect.w, 20);
    EXPECT_EQ(player.rect.h, 30);

    // Test resetting size
    player.resetSize();
    EXPECT_EQ(player.rect.w, 30);
    EXPECT_EQ(player.rect.h, 40);

    // Test shrinking below the minimum size (should clamp to MIN_SIZE)
    player.shrink(40); // w: 30->-10 (clamps to 20), h: 40->0 (clamps to 20)
    EXPECT_EQ(player.rect.w, Player::MIN_SIZE);
    EXPECT_EQ(player.rect.h, Player::MIN_SIZE);
}