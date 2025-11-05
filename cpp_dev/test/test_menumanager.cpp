#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include "../src/MenuManager.h"
#include "../config/Config.h"
#include "test_helpers.h"

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

    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0);
        ASSERT_EQ(TTF_Init(), 0);

        window_.reset(SDL_CreateWindow("Test", 0, 0, 100, 100, SDL_WINDOW_HIDDEN));
        ASSERT_NE(window_, nullptr);

        renderer_.reset(SDL_CreateRenderer(window_.get(), -1, 0));
        ASSERT_NE(renderer_, nullptr);
    }

    void TearDown() override {
        // unique_ptr handles cleanup automatically.
        TTF_Quit();
        SDL_Quit();
    }
};

// --- Main Menu Tests ---

TEST_F(MenuManagerTest, MainMenuRendersAndQuitsOnQuitEvent) {
    // This test verifies that the main menu screen displays and
    // correctly handles the SDL_QUIT event.

    // Ensure the text is loaded from config
    EXPECT_FALSE(config_.getMainMenuTitle().empty());
    EXPECT_FALSE(config_.getMainMenuInstructions().empty());

    // Run the screen function and ensure it doesn't crash and returns Quit on SDL_QUIT
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::Quit);
    });
}

TEST_F(MenuManagerTest, MainMenuReturnsStartGameOnS) {
    SDL_Event s_event;
    s_event.type = SDL_KEYDOWN;
    s_event.key.keysym.sym = SDLK_s;
    SDL_PushEvent(&s_event);

    EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::StartGame);
}

TEST_F(MenuManagerTest, MainMenuReturnsQuitOnQ) {
    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);

    EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::Quit);
}

TEST_F(MenuManagerTest, MainMenuReturnsQuitOnEscape) {
    SDL_Event escape_event;
    escape_event.type = SDL_KEYDOWN;
    escape_event.key.keysym.sym = SDLK_ESCAPE;
    SDL_PushEvent(&escape_event);

    EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::Quit);
}

// --- Pause Menu Tests ---

TEST_F(MenuManagerTest, PauseMenuReturnsResumeOnEscape) {
    SDL_Event escape_event;
    escape_event.type = SDL_KEYDOWN;
    escape_event.key.keysym.sym = SDLK_ESCAPE;
    SDL_PushEvent(&escape_event);

    EXPECT_EQ(MenuManager::showPauseMenu(renderer_.get(), config_), PauseMenuAction::Resume);
}

TEST_F(MenuManagerTest, PauseMenuReturnsRestartOnR) {
    SDL_Event r_event;
    r_event.type = SDL_KEYDOWN;
    r_event.key.keysym.sym = SDLK_r;
    SDL_PushEvent(&r_event);

    EXPECT_EQ(MenuManager::showPauseMenu(renderer_.get(), config_), PauseMenuAction::Restart);
}

TEST_F(MenuManagerTest, PauseMenuReturnsMainMenuOnM) {
    SDL_Event m_event;
    m_event.type = SDL_KEYDOWN;
    m_event.key.keysym.sym = SDLK_m;
    SDL_PushEvent(&m_event);

    EXPECT_EQ(MenuManager::showPauseMenu(renderer_.get(), config_), PauseMenuAction::MainMenu);
}

TEST_F(MenuManagerTest, PauseMenuReturnsQuitOnQ) {
    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);

    EXPECT_EQ(MenuManager::showPauseMenu(renderer_.get(), config_), PauseMenuAction::Quit);
}

// --- Game Over Screen Tests ---

TEST_F(MenuManagerTest, GameOverScreenRendersCorrectMessage) {
    // This test verifies that the game over screen displays the correct message
    // passed to it, for both victory and game over scenarios.

    // 1. Test "Game Over" message
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getGameOverText());
    });

    // 2. Test "Victory" message
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getVictoryText());
    });
}

TEST_F(MenuManagerTest, GameOverScreenReturnsRestartOnR) {
    const std::string message = "Test Message";
    SDL_Event r_event;
    r_event.type = SDL_KEYDOWN;
    r_event.key.keysym.sym = SDLK_r;
    SDL_PushEvent(&r_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message), GameOverAction::Restart);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsMainMenuOnM) {
    const std::string message = "Test Message";
    SDL_Event m_event;
    m_event.type = SDL_KEYDOWN;
    m_event.key.keysym.sym = SDLK_m;
    SDL_PushEvent(&m_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message), GameOverAction::MainMenu);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsQuitOnQ) {
    const std::string message = "Test Message";
    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message), GameOverAction::Quit);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsQuitOnEscape) {
    const std::string message = "Test Message";
    SDL_Event escape_event;
    escape_event.type = SDL_KEYDOWN;
    escape_event.key.keysym.sym = SDLK_ESCAPE;
    SDL_PushEvent(&escape_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message), GameOverAction::Quit);
}