#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../src/GameLogic.h"
#include "../config/Config.h"
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

TEST_F(RendererTest, GameOverScreenRuns) {
    // This is a smoke test. It can't verify the visual output, but it
    // ensures the function runs to completion without crashing. It also
    // simulates a quit event to ensure the function's event loop terminates.
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        showGameOverScreen(renderer_, config_);
    });
}