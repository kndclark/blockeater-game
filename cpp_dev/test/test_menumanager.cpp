#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include "../src/MenuManager.h"
#include "../src/ScoreboardManager.h"
#include "../config/Config.h"
#include "test_helpers.h"
#include "nlohmann/json.hpp"

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

TEST_F(MenuManagerTest, MainMenuReturnsSettingsOnE) {
    SDL_Event e_event;
    e_event.type = SDL_KEYDOWN;
    e_event.key.keysym.sym = SDLK_e;
    SDL_PushEvent(&e_event);

    EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::Settings);
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
    // Create a dummy scoreboard manager for this test
    ScoreboardManager scoreboard_manager("test_scores.json");

    // This test verifies that the game over screen displays the correct message
    // passed to it, for both victory and game over scenarios.

    // 1. Test "Game Over" message
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_KEYDOWN;
        quit_event.key.keysym.sym = SDLK_ESCAPE; // To exit name input
        SDL_PushEvent(&quit_event);
        quit_event.type = SDL_QUIT; // To exit game over screen
        SDL_PushEvent(&quit_event);
        MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getGameOverText(), 0, scoreboard_manager);
    });

    // 2. Test "Victory" message
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_KEYDOWN;
        quit_event.key.keysym.sym = SDLK_ESCAPE; // To exit name input
        SDL_PushEvent(&quit_event);
        quit_event.type = SDL_QUIT; // To exit game over screen
        SDL_PushEvent(&quit_event);
        MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getVictoryText(), 1000, scoreboard_manager);
    });
}

TEST_F(MenuManagerTest, GameOverScreenReturnsRestartOnR) {
    ScoreboardManager scoreboard_manager("test_scores.json");
    // Use a neutral message that won't trigger the name input screen.
    const std::string message = "Test Message - No Name Input";
    SDL_Event r_event;
    r_event.type = SDL_KEYDOWN;
    r_event.key.keysym.sym = SDLK_r;
    SDL_PushEvent(&r_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message, 0, scoreboard_manager), GameOverAction::Restart);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsMainMenuOnM) {
    ScoreboardManager scoreboard_manager("test_scores.json");
    const std::string message = "Test Message - No Name Input";
    SDL_Event m_event;
    m_event.type = SDL_KEYDOWN;
    m_event.key.keysym.sym = SDLK_m;
    SDL_PushEvent(&m_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message, 0, scoreboard_manager), GameOverAction::MainMenu);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsQuitOnQ) {
    ScoreboardManager scoreboard_manager("test_scores.json");
    const std::string message = "Test Message - No Name Input";
    SDL_Event q_event;
    q_event.type = SDL_KEYDOWN;
    q_event.key.keysym.sym = SDLK_q;
    SDL_PushEvent(&q_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message, 0, scoreboard_manager), GameOverAction::Quit);
}

// --- Settings Menu Tests ---

TEST_F(MenuManagerTest, SettingsMenuRendersAndReturnsBackOnQuitEvent) {
    // Ensure the text is loaded from config
    EXPECT_FALSE(config_.getSettingsMenuTitle().empty());
    EXPECT_FALSE(config_.getSettingsMenuInstructions().empty());

    // Run the screen function and ensure it doesn't crash and returns Back on SDL_QUIT
    EXPECT_NO_THROW({
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event); // Push a quit event to exit the loop
        EXPECT_EQ(MenuManager::showSettingsMenu(renderer_.get(), config_), SettingsMenuAction::Back);
    });
}

TEST_F(MenuManagerTest, SettingsMenuReturnsBackOnB) {
    SDL_Event b_event;
    b_event.type = SDL_KEYDOWN;
    b_event.key.keysym.sym = SDLK_b;
    SDL_PushEvent(&b_event);

    EXPECT_EQ(MenuManager::showSettingsMenu(renderer_.get(), config_), SettingsMenuAction::Back);
}

TEST_F(MenuManagerTest, SettingsMenuReturnsBackOnEscape) {
    SDL_Event escape_event;
    escape_event.type = SDL_KEYDOWN;
    escape_event.key.keysym.sym = SDLK_ESCAPE;
    SDL_PushEvent(&escape_event);

    EXPECT_EQ(MenuManager::showSettingsMenu(renderer_.get(), config_), SettingsMenuAction::Back);
}

TEST_F(MenuManagerTest, SettingsMenuReturnsToggleFullscreenOnT) {
    SDL_Event t_event;
    t_event.type = SDL_KEYDOWN;
    t_event.key.keysym.sym = SDLK_t;
    SDL_PushEvent(&t_event);

    EXPECT_EQ(MenuManager::showSettingsMenu(renderer_.get(), config_), SettingsMenuAction::ToggleFullscreen);
}

TEST_F(MenuManagerTest, GameOverScreenReturnsQuitOnEscape) {
    ScoreboardManager scoreboard_manager("test_scores.json");
    const std::string message = "Test Message - No Name Input";
    SDL_Event escape_event;
    escape_event.type = SDL_KEYDOWN;
    escape_event.key.keysym.sym = SDLK_ESCAPE;
    SDL_PushEvent(&escape_event);

    EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, message, 0, scoreboard_manager), GameOverAction::Quit);
}

TEST_F(MenuManagerTest, SettingsMenuCanChangeAndSavePlayerColor) {
    // 1. Get the initial color from the test config file.
    const Color initial_color = config_.getPlayerColor(); // Should be purple from config.json
    const Color new_color = config_.getPlayerColorChoices()[1]; // The second choice is green

    // The test will "change" the color from purple to green by selecting the second option.
    ASSERT_NE(initial_color, new_color) << "The initial color should be different from the one we are changing to.";

    // 2. Simulate user input to change the color.
    // C to enter color picker, RETURN to confirm the default selection (0), B to go back.
    SDL_Event c_event, enter_event, b_event;
    c_event.type = enter_event.type = b_event.type = SDL_KEYDOWN;
    c_event.key.keysym.sym = SDLK_c;
    enter_event.key.keysym.sym = SDLK_RETURN;
    b_event.key.keysym.sym = SDLK_b; // This event is just to exit the menu loop

    SDL_Event right_event;
    right_event.type = SDL_KEYDOWN;
    right_event.key.keysym.sym = SDLK_RIGHT;

    SDL_PushEvent(&c_event);
    SDL_PushEvent(&right_event); // Move selection to the right once (to green)
    SDL_PushEvent(&enter_event);
    SDL_PushEvent(&b_event);

    // Push a quit event to ensure the menu loop terminates in the test environment.
    SDL_Event quit_event;
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);

    // 3. Run the settings menu. This modifies the in-memory config.
    // The renderer is provided by the MenuManagerTest fixture.
    MenuManager::showSettingsMenu(renderer_.get(), config_);

    // 4. Verify the in-memory config object was updated.
    Color final_color = config_.getPlayerColor();

    EXPECT_EQ(final_color, new_color);
}

TEST_F(MenuManagerTest, MainMenuReturnsShowScoreboardOnC) {
    SDL_Event c_event;
    c_event.type = SDL_KEYDOWN;
    c_event.key.keysym.sym = SDLK_c;
    SDL_PushEvent(&c_event);

    EXPECT_EQ(MenuManager::showMainMenu(renderer_.get(), config_), MainMenuAction::ShowScoreboard);
}

TEST_F(MenuManagerTest, GameOverScreenNameInputTransitions) {
    ScoreboardManager scoreboard_manager("test_scores.json");

    // 1. Test that entering a name and pressing Enter returns MainMenu
    {
        SDL_Event text_event, enter_event;
        text_event.type = SDL_TEXTINPUT;
        strcpy(text_event.text.text, "A"); // NOLINT(cppcoreguidelines-pro-type-strcpy)

        enter_event.type = SDL_KEYDOWN;
        enter_event.key.keysym.sym = SDLK_RETURN;

        SDL_PushEvent(&text_event);
        SDL_PushEvent(&enter_event);

        GameOverAction action = MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getVictoryText(), 1000, scoreboard_manager);
        EXPECT_EQ(action, GameOverAction::MainMenu);
    }

    // 2. Test that pressing Escape during name input cancels and waits for another action (we'll send Quit)
    {
        SDL_Event escape_event, quit_event;
        escape_event.type = SDL_KEYDOWN;
        escape_event.key.keysym.sym = SDLK_ESCAPE;
        quit_event.type = SDL_QUIT;

        SDL_PushEvent(&escape_event);
        SDL_PushEvent(&quit_event); // This will be caught by the outer game over loop
        EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getGameOverText(), 1, scoreboard_manager), GameOverAction::Quit);
    }

    // 3. Test that a score of 0 skips the name input and waits for another action (we'll send Quit)
    {
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event);
        EXPECT_EQ(MenuManager::showGameOverScreen(renderer_.get(), config_, config_.getGameOverText(), 0, scoreboard_manager), GameOverAction::Quit);
    }
}