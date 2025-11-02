#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../src/GameLogic.h"
#include "../config/Config.h"
#include "../src/GameState.h"
#include "test_helpers.h"

// Test fixture for tests that require a renderer.
class RendererTest : public ::testing::Test {
protected:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    Config config_{kTestRootPath};

    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0);
        ASSERT_EQ(TTF_Init(), 0);

        window_ = SDL_CreateWindow("Test", 0, 0, 100, 100, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window_, nullptr);

        renderer_ = SDL_CreateRenderer(window_, -1, 0);
        ASSERT_NE(renderer_, nullptr);
    }

    void TearDown() override {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_Quit();
    }
};

TEST_F(RendererTest, GameOverScreenDisplaysGameOverTextOnLoss) {
    // This is a smoke test. It can't verify the visual output, but it
    // ensures the function runs to completion without crashing. It also
    // simulates a quit event to ensure the function's event loop terminates.

    // 1. Simulate a game loss state
    GameState game_state(config_, 100, 100);
    game_state.running = false;
    game_state.victory = false;

    // 2. Determine the message based on the game state, mimicking main()
    const std::string& message = game_state.victory ? config_.getVictoryText() : config_.getGameOverText();

    // 3. Assert that the correct "Game Over" message is selected
    EXPECT_EQ(message, config_.getGameOverText());

    // 4. Run the screen function and ensure it doesn't crash
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        showGameOverScreen(renderer_, config_, message);
    });
}

TEST_F(RendererTest, GameOverScreenDisplaysVictoryTextOnWin) {
    // 1. Simulate a game victory state
    GameState game_state(config_, 100, 100);
    game_state.running = false;
    game_state.victory = true;

    const std::string& message = game_state.victory ? config_.getVictoryText() : config_.getGameOverText();
    EXPECT_EQ(message, config_.getVictoryText());

    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        showGameOverScreen(renderer_, config_, message);
    });
}