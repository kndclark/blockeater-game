#include <gtest/gtest.h>
#include "../src/Player.h"
#include "../config/Config.h"
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
    // Use default dash values for this test as it doesn't involve dashing.
    Config config(kTestRootPath);
    Player player({params.start_x,
                   params.start_y,
                   40, 40, 5, c,
                   config.getDashSpeedMultiplier(),
                   config.getDashDurationMs(),
                   config.getDashCooldownMs()
                  });

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
    Config config{kTestRootPath};
    // Use a speed of 10 for easier math in tests
    Player player{{100, 100, 40, 40, 10, {0,0,0,0},
                   config.getDashSpeedMultiplier(),
                   config.getDashDurationMs(),
                   config.getDashCooldownMs()
                  }};

    const int screen_width = 640;
    const int screen_height = 480;
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};
};

TEST_F(PlayerDashTest, DashActivatesAndIncreasesSpeed) {
    ASSERT_EQ(player.state, PlayerState::Ready);

    // Press Shift and Right arrow to dash right
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    // Check state
    EXPECT_EQ(player.state, PlayerState::Dashing);

    // Check position change
    int expected_x = 100 + static_cast<int>(10 * config.getDashSpeedMultiplier());
    EXPECT_EQ(player.rect.x, expected_x);
}

TEST_F(PlayerDashTest, DashForwardWithNoDirectionalInput) {
    ASSERT_EQ(player.state, PlayerState::Ready);

    // Press Shift with no direction to dash forward
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    EXPECT_EQ(player.state, PlayerState::Dashing);
    int expected_x = 100 + static_cast<int>(10 * config.getDashSpeedMultiplier());
    EXPECT_EQ(player.rect.x, expected_x);
}

TEST_F(PlayerDashTest, DashEndsAndEntersCooldown) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    ASSERT_EQ(player.state, PlayerState::Dashing);

    // Wait for dash to end
    // Simulate time passing
    player.update(player.dash_start_time + player.dash_duration_ms + 1);

    EXPECT_EQ(player.state, PlayerState::Cooldown);
}

TEST_F(PlayerDashTest, CannotDashWhileDashing) {
    // Start dashing
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    ASSERT_EQ(player.state, PlayerState::Dashing);
    Uint32 dash_start_time = player.dash_start_time;

    // Try to dash again immediately while holding the key
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.state, PlayerState::Dashing);
    // The dash start time should not have been reset
    EXPECT_EQ(player.dash_start_time, dash_start_time);
}

TEST_F(PlayerDashTest, CannotDashOnCooldown) {
    // Start dashing
    player.state = PlayerState::Dashing;
    player.dash_start_time = 1000;

    // End dash and start cooldown by simulating time
    player.update(1000 + player.dash_duration_ms + 1);
    ASSERT_EQ(player.state, PlayerState::Cooldown);

    // Try to dash again while on cooldown
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);

    // Should still be on cooldown, not dashing
    EXPECT_EQ(player.state, PlayerState::Cooldown);
}

TEST_F(PlayerDashTest, DashBecomesAvailableAfterCooldown) {
    // Start dashing
    player.state = PlayerState::Dashing;
    player.dash_start_time = 1000;

    // Simulate time passing to end the dash and start the cooldown
    Uint32 cooldown_start_time = 1000 + player.dash_duration_ms + 1;
    player.update(cooldown_start_time);
    ASSERT_EQ(player.state, PlayerState::Cooldown);

    // Simulate time passing for the cooldown to end
    player.update(cooldown_start_time + player.dash_cooldown_ms + 1);

    // Cooldown should be over
    EXPECT_EQ(player.state, PlayerState::Ready);

    // Dash should be available again
    keystate[SDL_SCANCODE_LSHIFT] = 1;
    player.handle_input(keystate, screen_width, screen_height);
    EXPECT_EQ(player.state, PlayerState::Dashing);
}

class PlayerGhostingTest : public SdlTest {
protected:
    Config config{kTestRootPath};
    Player player;

    PlayerGhostingTest() : player({100, 100, 40, 40, 10, {0,0,0,0},
                                   config.getDashSpeedMultiplier(),
                                   config.getDashDurationMs(),
                                   config.getDashCooldownMs()}) {}
};

TEST_F(PlayerGhostingTest, SpawnsGhostsWhileDashing) {
    player.state = PlayerState::Dashing;
    Uint32 current_time = 1000;

    // First spawn
    player.update_ghosts(current_time);
    ASSERT_EQ(player.ghosts.size(), 1);
    EXPECT_EQ(player.ghosts.back().rect.x, player.rect.x);
    EXPECT_EQ(player.ghosts.back().creation_time, current_time);

    // No spawn before interval
    player.update_ghosts(current_time + Player::GHOST_SPAWN_INTERVAL_MS - 1);
    ASSERT_EQ(player.ghosts.size(), 1);

    // Second spawn after interval
    current_time += Player::GHOST_SPAWN_INTERVAL_MS + 1;
    player.update_ghosts(current_time);
    ASSERT_EQ(player.ghosts.size(), 2);
}

TEST_F(PlayerGhostingTest, DoesNotSpawnGhostsWhenNotDashing) {
    player.state = PlayerState::Ready;
    Uint32 current_time = 1000;

    player.update_ghosts(current_time);
    EXPECT_TRUE(player.ghosts.empty());

    player.update_ghosts(current_time + Player::GHOST_SPAWN_INTERVAL_MS + 1);
    EXPECT_TRUE(player.ghosts.empty());
}

TEST_F(PlayerGhostingTest, RemovesOldGhosts) {
    player.state = PlayerState::Dashing;
    Uint32 spawn_time = 1000;

    // Spawn a ghost
    player.update_ghosts(spawn_time);
    ASSERT_EQ(player.ghosts.size(), 1);

    // Update just before it expires
    player.update_ghosts(spawn_time + Player::GHOST_SPAWN_INTERVAL_MS - 1);
    ASSERT_EQ(player.ghosts.size(), 1);

    // Set state to not dashing to prevent new ghosts from spawning
    player.state = PlayerState::Ready;

    // Update just after it expires
    player.update_ghosts(spawn_time + Player::GHOST_LIFETIME_MS + 1);
    EXPECT_TRUE(player.ghosts.empty());
}

TEST_F(PlayerGhostingTest, SpawnsAndRemovesInSameUpdate) {
    player.state = PlayerState::Dashing;
    Uint32 first_spawn_time = 1000;

    // 1. Spawn the first ghost.
    player.update_ghosts(first_spawn_time);
    ASSERT_EQ(player.ghosts.size(), 1);
    ASSERT_EQ(player.ghosts.front().creation_time, first_spawn_time);

    // 2. Simulate a long delay that is greater than both the spawn interval and the ghost lifetime.
    Uint32 update_time = first_spawn_time + Player::GHOST_LIFETIME_MS + 1;
    player.update_ghosts(update_time);

    // 3. Assert that the old ghost was removed AND a new one was spawned.
    // The size should still be 1, but it should be a *different* ghost.
    ASSERT_EQ(player.ghosts.size(), 1);
    EXPECT_NE(player.ghosts.front().creation_time, first_spawn_time) << "The original ghost should have been removed.";
    EXPECT_EQ(player.ghosts.front().creation_time, update_time) << "A new ghost should have been created at the current update time.";
}

TEST_F(PlayerGhostingTest, ClearsGhostsWhenDashEnds) {
    player.state = PlayerState::Dashing;
    player.update_ghosts(1000); // Spawn a ghost
    ASSERT_FALSE(player.ghosts.empty());

    // Simulate dash ending in the main update loop
    player.dash_start_time = 1000;
    player.update(1000 + player.dash_duration_ms + 1);

    EXPECT_EQ(player.state, PlayerState::Cooldown);
    EXPECT_TRUE(player.ghosts.empty());
}

TEST(PlayerTest, CreationAndSizeModification) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    Config config(kTestRootPath);
    Player player({10, 20, 30, 40, 5, {0,0,0,0},
                   config.getDashSpeedMultiplier(), config.getDashDurationMs(), config.getDashCooldownMs()
                  });
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