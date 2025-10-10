#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <memory>
#include "../src/Scoreboard.h"

namespace {
// This path is relative to the test executable in `test/build/`
const std::string kRelativeFontPath = "../../assets/font.ttf";
} // namespace

// Test fixture for UI tests that require SDL and SDL_ttf initialization.
class UiTest : public ::testing::Test {
protected:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::string font_path_;

    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0);
        ASSERT_EQ(TTF_Init(), 0);

        window_ = SDL_CreateWindow("Test", 0, 0, 100, 100, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window_, nullptr);

        renderer_ = SDL_CreateRenderer(window_, -1, 0);
        ASSERT_NE(renderer_, nullptr);

        // Construct a robust path to the font file.
        char* base_path = SDL_GetBasePath();
        if (base_path) {
            font_path_ = std::string(base_path) + kRelativeFontPath;
            SDL_free(base_path);
        } else {
            // Fallback for when the base path can't be determined.
            // This might happen on some platforms.
            font_path_ = kRelativeFontPath;
        }
    }

    void TearDown() override {
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        TTF_Quit();
        SDL_Quit();
    }
};

// Test successful creation of the Scoreboard.
TEST_F(UiTest, ScoreboardCreationSuccess) {
    EXPECT_NO_THROW({
        Scoreboard scoreboard(renderer_, font_path_, 24);
    });
}

// Test that Scoreboard creation throws an exception with an invalid font path.
TEST_F(UiTest, ScoreboardCreationFailure) {
    const std::string invalid_font_path = "nonexistent_font.ttf";
    EXPECT_THROW({
        Scoreboard scoreboard(renderer_, invalid_font_path, 24);
    }, std::runtime_error);
}

// Test that the render method can be called without crashing.
TEST_F(UiTest, ScoreboardRender) {
    Scoreboard scoreboard(renderer_, font_path_, 24);

    // This is a smoke test to ensure render() doesn't crash.
    // We can't easily verify the visual output in a unit test.
    EXPECT_NO_THROW({
        SDL_RenderClear(renderer_);
        scoreboard.render(12345, 1, 80, 2, 5);
        SDL_RenderPresent(renderer_);
    });
}

// Test the logic for generating the level progress text. By using the UiTest
// fixture, we ensure SDL is initialized correctly before the Scoreboard is created.
TEST_F(UiTest, CalculatesGapsToNextLevelCorrectly) {
    Scoreboard scoreboard(renderer_, font_path_, 24);

    EXPECT_EQ(scoreboard.getLevelText(1, 0, 5), "Level: 1 (5 to next)");
    EXPECT_EQ(scoreboard.getLevelText(1, 1, 5), "Level: 1 (4 to next)");
    EXPECT_EQ(scoreboard.getLevelText(1, 4, 5), "Level: 1 (1 to next)");
    // After passing 5 checkpoints (0-4), the 6th checkpoint (index 5) means a level up.
    EXPECT_EQ(scoreboard.getLevelText(2, 5, 5), "Level: 2 (5 to next)");
    // 34 checkpoints passed, 8 per level. 34 % 8 = 2 checkpoints into the current level. 8 - 2 = 6 to go.
    EXPECT_EQ(scoreboard.getLevelText(7, 34, 8), "Level: 7 (6 to next)");
}

// Test that rendering different score and level values works without crashing.
TEST_F(UiTest, ScoreboardRendersVariousValues) {
    Scoreboard scoreboard(renderer_, font_path_, 24);

    // We can't easily check the visual output, but we can confirm that
    // rendering different values completes without throwing any exceptions.
    EXPECT_NO_THROW({
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        scoreboard.render(0, 1, 80, 0, 5);        // Initial score
        scoreboard.render(99999, 10, 60, 53, 10);   // High score and level
        scoreboard.render(-100, 5, 25, 28, 8);     // Negative score (if possible in game)
        SDL_RenderPresent(renderer_);
    });
}