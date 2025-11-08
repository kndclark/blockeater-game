#pragma once

#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include "../src/GameState.h"
#include <SDL2/SDL_ttf.h>
#include "../config/Config.h"
#include <fstream>
#include <string>

inline const std::string kTestRootPath = "../../";
inline const std::string kTestConfigPath = kTestRootPath + "config/json/config.json";
 
// A test-only subclass of Config to expose the protected load_levels method.
class TestConfig : public Config {
public:
    // Inherit constructors from Config
    using Config::Config;

    // Test-only constructor to load directly from a specific config file path.
    // This is used for tests that create temporary config files.
    explicit TestConfig(const std::string& direct_path, bool /* is_direct_path */) {
        load_defaults();
        load_from_path(direct_path);
        if (getGrowChance() + getShrinkChance() + getHurtChance() != 100) {
            throw std::runtime_error("Obstacle spawn chances in config must sum to 100.");
        }
    }
 
    // Publicly expose load_levels for testing purposes
    void load_levels_for_test(const std::string& filepath) {
        load_levels(filepath);
    }
};
 
inline TestConfig loadTestConfig(const std::string& config_path) {
    TestConfig config(config_path);
    config.load_levels_for_test(config_path + ".levels.json");
    return config;
}

// A test-only subclass of LevelManager to allow setting custom intervals.
class TestLevelManager : public LevelManager {
public:
    explicit TestLevelManager(const Config& base_config) : LevelManager(base_config) {}

    void set_spawn_interval(Uint32 interval) { effective_spawn_interval_ = interval; }
    void set_checkpoint_interval(Uint32 interval) { effective_checkpoint_interval_ms_ = interval; }
};

inline TestLevelManager createTestLevelManager(const Config& config, Uint32 spawn_interval, Uint32 checkpoint_interval) {
    TestLevelManager lm(config);
    lm.set_spawn_interval(spawn_interval);
    lm.set_checkpoint_interval(checkpoint_interval);
    return lm;
}

// A test-only subclass of GameState to allow injecting a TestLevelManager and a
// re-initialized ObstacleSpawner. This avoids modifying the production GameState class.
class TestGameState : public GameState {
public:
    // This constructor takes a custom level manager and initializes a new spawner with it.
    // It calls a base constructor that does not initialize the spawner, then does it here.
    TestGameState(const Config& config, int screen_width, int screen_height, TestLevelManager&& lm, Uint32 start_time = 0)
        : GameState(config, screen_width, screen_height), // This still constructs the base members
          custom_level_manager(std::move(lm)), // Store our custom manager
          // This spawner will shadow the base one, which is what we want for this test.
          spawner(custom_level_manager, config.getCheckpointSafeZoneDuration(), screen_width, screen_height, config.getPlayerSizeChangeAmount(), start_time)
    {
        // We must manually re-initialize GameState members that depend on the spawner's initial state.
        ui_next_checkpoint_gap_size = spawner.calculateCheckpointGapSize();
        next_checkpoint_gap_size = spawner.calculateCheckpointGapSize();
    }

    // These members shadow the base class members, effectively replacing them for this test class instance.
    TestLevelManager custom_level_manager;
    ObstacleSpawner spawner;
};
// Helper to check that the Config class correctly overrides screen dimensions
// with the native resolution when available, or falls back to the default.
inline void checkConfigScreenResolution(const Config& config) {
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        // If we can get the display mode, the config should have used it.
        EXPECT_EQ(config.getScreenWidth(), dm.w);
        EXPECT_EQ(config.getScreenHeight(), dm.h);
    } else {
        // If we can't, it should have fallen back to the default/file values (640x480).
        EXPECT_EQ(config.getScreenWidth(), 640);
        EXPECT_EQ(config.getScreenHeight(), 480);
    }
}

// Helper to inject a configured spawner into a GameState for testing.
// This replaces the need for a test-only constructor in GameState itself.
inline void injectSpawnerForTest(GameState& gameState, ObstacleSpawner&& spawner) {
    // Use placement new to construct a new ObstacleSpawner in place of the old one.
    new (&gameState.spawner) ObstacleSpawner(std::move(spawner)); // NOLINT(bugprone-use-after-move)
    gameState.next_checkpoint_gap_size = gameState.spawner.calculateCheckpointGapSize();
}

// A test fixture for tests that require SDL to be initialized.
class SdlTest : public ::testing::Test {
protected:
    void SetUp() override { ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0) << "Failed to initialize SDL: " << SDL_GetError(); }
    void TearDown() override { SDL_Quit(); }
};

// A test fixture for tests that create temporary files.
class ConfigFileTest : public SdlTest {
protected:
    const std::string malformed_filename = "malformed.json";
    const std::string partial_filename = "partial.json";
    const std::string invalid_chances_filename = "invalid_chances.json";

    void SetUp() override {
        SdlTest::SetUp();
        ASSERT_EQ(TTF_Init(), 0);
    }

    void TearDown() override {
        std::remove(malformed_filename.c_str());
        std::remove(partial_filename.c_str());
        std::remove(invalid_chances_filename.c_str());
        TTF_Quit();
        SdlTest::TearDown();
    }
};

// Test fixture for tests that require a renderer.
class MenuManagerTest : public ::testing::Test {
protected:
    struct SdlDeleter {
        void operator()(SDL_Window* w) const { if (w) SDL_DestroyWindow(w); }
        void operator()(SDL_Renderer* r) const { if (r) SDL_DestroyRenderer(r); }
    };

    std::unique_ptr<SDL_Window, SdlDeleter> window_;
    std::unique_ptr<SDL_Renderer, SdlDeleter> renderer_;
    Config config_{kTestRootPath};
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0);
        ASSERT_EQ(TTF_Init(), 0);

        window_.reset(SDL_CreateWindow("Test", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_HIDDEN));
        ASSERT_NE(window_, nullptr);

        renderer_.reset(SDL_CreateRenderer(window_.get(), -1, 0));
        ASSERT_NE(renderer_, nullptr);
    }

    void TearDown() override {
        TTF_Quit();
        SDL_Quit();
    }
};