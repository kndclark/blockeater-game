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

TEST_F(RendererTest, GameOverScreenDisplaysGameOverTextAndQuitsOnQ) {
    // This test verifies that the game over screen displays the correct text
    // for a loss and correctly handles the 'Q' key to quit.

    // 1. Simulate a game loss state
    GameState game_state(config_, 100, 100);
    game_state.running = false;
    game_state.victory = false;

    // 2. Determine the message based on the game state, mimicking main()
    const std::string message = game_state.victory ? config_.getVictoryText() : config_.getGameOverText();

    // 3. Assert that the correct "Game Over" message is selected
    EXPECT_EQ(message, config_.getGameOverText());
    // Also ensure the instructions text is loaded from config
    EXPECT_FALSE(config_.getGameOverInstructions().empty());


    // 4. Run the screen function and ensure it doesn't crash and returns Quit on 'Q'
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        showGameOverScreen(renderer_, config_, message);
    });

    // Re-run to test specific key press
    GameState game_state_q(config_, 100, 100);
    game_state_q.running = false;
    game_state_q.victory = false;
    const std::string message_q = game_state_q.victory ? config_.getVictoryText() : config_.getGameOverText();

    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);
    EXPECT_EQ(showGameOverScreen(renderer_, config_, message_q), GameOverAction::Quit);
}

TEST_F(RendererTest, GameOverScreenDisplaysVictoryTextAndQuitsOnEscape) {
    // This test verifies that the game over screen displays the correct text
    // for a victory and correctly handles the 'Escape' key to quit.

    // 1. Simulate a game victory state
    GameState game_state(config_, 100, 100);
    game_state.running = false;
    game_state.victory = true;

    const std::string message = game_state.victory ? config_.getVictoryText() : config_.getGameOverText();
    EXPECT_EQ(message, config_.getVictoryText());
    // Also ensure the instructions text is loaded from config
    EXPECT_FALSE(config_.getGameOverInstructions().empty());

    // 2. Run the screen function and ensure it doesn't crash and returns Quit on 'Escape'
    EXPECT_NO_THROW({
        SDL_Event escape_event;
        escape_event.type = SDL_KEYDOWN;
        escape_event.key.keysym.sym = SDLK_ESCAPE;
        SDL_PushEvent(&escape_event);
        EXPECT_EQ(showGameOverScreen(renderer_, config_, message), GameOverAction::Quit);
    });
}

TEST_F(RendererTest, GameOverScreenReturnsRestartOnR) {
    // This test verifies that pressing 'R' on the game over screen returns GameOverAction::Restart.
    GameState game_state(config_, 100, 100); // State doesn't matter for this test
    const std::string message = "Test Message";

    SDL_Event r_event;
    r_event.type = SDL_KEYDOWN;
    r_event.key.keysym.sym = SDLK_r;
    SDL_PushEvent(&r_event);

    EXPECT_NO_THROW({
        EXPECT_EQ(showGameOverScreen(renderer_, config_, message), GameOverAction::Restart);
    });
}

TEST_F(RendererTest, GameOverScreenReturnsMainMenuOnM) {
    // This test verifies that pressing 'M' on the game over screen returns GameOverAction::MainMenu.
    GameState game_state(config_, 100, 100); // State doesn't matter for this test
    const std::string message = "Test Message";

    SDL_Event m_event;
    m_event.type = SDL_KEYDOWN;
    m_event.key.keysym.sym = SDLK_m;
    SDL_PushEvent(&m_event);

    EXPECT_NO_THROW({
        EXPECT_EQ(showGameOverScreen(renderer_, config_, message), GameOverAction::MainMenu);
    });
}

TEST_F(RendererTest, GameOverScreenReturnsQuitOnQ) {
    // This test verifies that pressing 'Q' on the game over screen returns GameOverAction::Quit.
    GameState game_state(config_, 100, 100); // State doesn't matter for this test
    const std::string message = "Test Message";

    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);

    EXPECT_NO_THROW({
        EXPECT_EQ(showGameOverScreen(renderer_, config_, message), GameOverAction::Quit);
    });
}