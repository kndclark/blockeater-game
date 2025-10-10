#pragma once

#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include "../src/GameState.h"
#include "../config/Config.h"
#include <fstream>
#include <string>

inline const std::string kTestConfigPath = "../../config/json/config.json";

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
    new (&gameState.spawner) ObstacleSpawner(std::move(spawner));
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

    void TearDown() override {
        std::remove(malformed_filename.c_str());
        std::remove(partial_filename.c_str());
        std::remove(invalid_chances_filename.c_str());
        SdlTest::TearDown();
    }
};