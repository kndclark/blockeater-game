#include <gtest/gtest.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <memory>
#include "../src/Scoreboard.h"
#include "../config/Config.h"

#include "test_helpers.h" // For kTestConfigPath

// Test fixture for UI tests that require SDL and SDL_ttf initialization.
class UiTest : public ::testing::Test {
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

// Test that the config correctly resolves the font path
TEST_F(UiTest, ConfigFontPathResolution) {
    // The config loaded by the fixture should have resolved the path
    const std::string& font_path = config_.getFontPath();
    EXPECT_NE(font_path.find("assets/font.ttf"), std::string::npos);

    // Check that the file actually exists at that path
    std::ifstream font_file(font_path);
    EXPECT_TRUE(font_file.good()) << "Font file not found at: " << font_path;
}

// Test successful creation of the Scoreboard.
TEST_F(UiTest, ScoreboardCreationSuccess) {
    EXPECT_NO_THROW({
        Scoreboard scoreboard(renderer_.get(), config_);
    });
}

// Test that Scoreboard creation throws an exception with an invalid font path.
// We use a mock Config class to simulate providing an invalid path.
TEST_F(UiTest, ScoreboardCreationFailure) {
    // A mock config class that overrides getFontPath to return an invalid path.
    class MockConfig : public Config {
    public:
        MockConfig() : Config(kTestRootPath) {} // Initialize with a valid config that finds assets
        const std::string& getFontPath() const override {
            return invalid_path_;
        }
    private:
        std::string invalid_path_ = "this/path/does/not/exist.ttf";
    };

    MockConfig invalid_config;
    EXPECT_THROW({
        Scoreboard scoreboard(renderer_.get(), invalid_config);
    }, std::runtime_error);
}

// Test that the render method can be called without crashing.
TEST_F(UiTest, ScoreboardRender) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // This is a smoke test to ensure render() doesn't crash.
    // We can't easily verify the visual output in a unit test.
    EXPECT_NO_THROW({
        SDL_RenderClear(renderer_.get()); // NOLINT(readability-magic-numbers)
        scoreboard.render(12345, 1, 80, 2, 5, 40, false, 0, SizeBoostLevel::None, 0, false, 0); // NOLINT(readability-magic-numbers)
        SDL_RenderPresent(renderer_.get());
    });
}

// Test the logic for generating the level progress text. By using the UiTest
// fixture, we ensure SDL is initialized correctly before the Scoreboard is created.
TEST_F(UiTest, CalculatesGapsToNextLevelCorrectly) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // Helper lambda to build the expected string from the config.
    // This makes the test resilient to changes in ui_texts.json.
    auto build_expected_text = [&](int level, int to_next) {
        return config_.getLevelPrefix() + std::to_string(level) +
               config_.getLevelProgressPrefix() + std::to_string(to_next) + config_.getLevelProgressSuffix();
    };

    EXPECT_EQ(scoreboard.getLevelText(1, 0, 5), build_expected_text(1, 5));
    EXPECT_EQ(scoreboard.getLevelText(1, 1, 5), build_expected_text(1, 4));
    EXPECT_EQ(scoreboard.getLevelText(1, 4, 5), build_expected_text(1, 1));
    // After passing 5 checkpoints (0-4), the 6th checkpoint (index 5) means a level up.
    EXPECT_EQ(scoreboard.getLevelText(2, 5, 5), build_expected_text(2, 5));
    // 34 checkpoints passed, 8 per level. 34 % 8 = 2 checkpoints into the current level. 8 - 2 = 6 to go.
    EXPECT_EQ(scoreboard.getLevelText(7, 34, 8), build_expected_text(7, 6));
}

TEST_F(UiTest, CalculatesPlayerSizeTextCorrectly) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // Helper lambda to build the expected string from the config.
    auto build_expected_text = [&](int player_size, int gap_size) {
        if (gap_size <= 0) return config_.getGapSizePrefix() + "N/A";
        int percentage = static_cast<int>(std::round((static_cast<double>(player_size) / gap_size) * 100.0));
        return config_.getGapSizePrefix() + std::to_string(percentage) + config_.getGapSizeSuffix();
    };

    EXPECT_EQ(scoreboard.getPlayerSizeText(40, 200), build_expected_text(40, 200)); // 20%
    EXPECT_EQ(scoreboard.getPlayerSizeText(60, 150), build_expected_text(60, 150)); // 40%
    EXPECT_EQ(scoreboard.getPlayerSizeText(150, 150), build_expected_text(150, 150)); // 100%
    EXPECT_EQ(scoreboard.getPlayerSizeText(89, 100), build_expected_text(89, 100)); // 89%
    EXPECT_EQ(scoreboard.getPlayerSizeText(90, 100), build_expected_text(90, 100)); // 90%
    EXPECT_EQ(scoreboard.getPlayerSizeText(89, 99), build_expected_text(89, 99));   // 89.89... -> 90%
    EXPECT_EQ(scoreboard.getPlayerSizeText(50, 0), build_expected_text(50, 0)); // N/A
}

// Test the logic for generating the dash status text.
TEST_F(UiTest, ScoreboardCalculatesDashStatusTextCorrectly) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // Helper lambda to build the expected cooldown string.
    auto build_cooldown_text = [&](const std::string& time_str) {
        return config_.getDashCooldownPrefix() + time_str + config_.getDashCooldownSuffix();
    };

    // Test "ready" state
    EXPECT_EQ(scoreboard.getDashStatusText(false, 0), config_.getDashReadyText());
    EXPECT_EQ(scoreboard.getDashStatusText(true, 0), config_.getDashReadyText()); // Cooldown just finished

    // Test "cooldown" state with various times
    EXPECT_EQ(scoreboard.getDashStatusText(true, 2000), build_cooldown_text("2.0"));
    EXPECT_EQ(scoreboard.getDashStatusText(true, 1540), build_cooldown_text("1.5"));
    EXPECT_EQ(scoreboard.getDashStatusText(true, 1550), build_cooldown_text("1.6")); // Should round up
    EXPECT_EQ(scoreboard.getDashStatusText(true, 999), build_cooldown_text("1.0")); // Should round from 0.999
}

// Test that the dash status rendering can be called without crashing.
TEST_F(UiTest, ScoreboardRendersDashStatus) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // Smoke test to ensure renderDashStatus() doesn't crash in various states.
    EXPECT_NO_THROW({
        SDL_RenderClear(renderer_.get());
        scoreboard.renderDashStatus(false, 0, false, 0); // Ready
        scoreboard.renderDashStatus(true, 2000, false, 0); // Full cooldown
        scoreboard.renderDashStatus(true, 999, true, 100);  // Partial cooldown with boost
        SDL_RenderPresent(renderer_.get());
    });
}

// Test the calculation logic for the cooldown circle.
TEST_F(UiTest, ScoreboardCalculatesCooldownCirclePoints) {
    Scoreboard scoreboard(renderer_.get(), config_);
    std::vector<SDL_Point> points;
    const int segments = 30;

    // Test 0% progress (should draw 1 point at the start)
    scoreboard.drawCooldownCircle(0.0f, 0, 0, 10, {255, 255, 255, 255}, &points);
    EXPECT_EQ(points.size(), 1);

    // Test 50% progress
    scoreboard.drawCooldownCircle(0.5f, 0, 0, 10, {255, 255, 255, 255}, &points);
    // +1 because the loop is i <= segments * progress
    EXPECT_EQ(points.size(), static_cast<int>(segments * 0.5f) + 1);

    // Test 100% progress
    scoreboard.drawCooldownCircle(1.0f, 0, 0, 10, {255, 255, 255, 255}, &points);
    EXPECT_EQ(points.size(), segments + 1);
}

struct BoostFlashColorParams {
    SizeBoostLevel level;
    Uint32 time_since_boost;
    Color expected_color;
    std::string description;
};

class BoostFlashColorTest : public UiTest, public ::testing::WithParamInterface<BoostFlashColorParams> {};

TEST_P(BoostFlashColorTest, SelectsCorrectFlashColorForBoostMessage) {
    auto params = GetParam();
    Scoreboard scoreboard(renderer_.get(), config_);

    // The function under test. It should return the flashing color for the boost message.
    Color result_color = scoreboard.getFlashColorForBoostMessage(params.level, params.time_since_boost);

    EXPECT_EQ(result_color, params.expected_color) << "Failed on: " << params.description;
}

INSTANTIATE_TEST_SUITE_P(
    ScoreboardColorTests,
    BoostFlashColorTest,
    ::testing::ValuesIn([] {
        Config config(kTestRootPath); // Use the real Config to load from JSON
        Color good_color = config.getSizeBoostTierColor(SizeBoostLevel::Good);
        Color great_color = config.getSizeBoostTierColor(SizeBoostLevel::Great);
        Color default_color = config.getUiTextColor();
        const auto& rainbow_colors = config.getRainbowColors();

        return std::vector<BoostFlashColorParams>{
            // Good tier should flash between green and default
            {SizeBoostLevel::Good, 100, good_color, "Good_FirstFlash"},
            {SizeBoostLevel::Good, 250, default_color, "Good_SecondFlash"},
            {SizeBoostLevel::Good, 400, good_color, "Good_ThirdFlash"},

            // Great tier should flash between yellow and default
            {SizeBoostLevel::Great, 100, great_color, "Great_FirstFlash"},
            {SizeBoostLevel::Great, 250, default_color, "Great_SecondFlash"},

            // Perfect tier should cycle through rainbow colors
            {SizeBoostLevel::Perfect, 50, rainbow_colors[0], "Perfect_Rainbow_0"},
            {SizeBoostLevel::Perfect, 100, rainbow_colors[1], "Perfect_Rainbow_1"},
            {SizeBoostLevel::Perfect, 150, rainbow_colors[2], "Perfect_Rainbow_2"},
            {SizeBoostLevel::Perfect, 200, rainbow_colors[3], "Perfect_Rainbow_3"},
            {SizeBoostLevel::Perfect, 250, rainbow_colors[4], "Perfect_Rainbow_4"},
            {SizeBoostLevel::Perfect, 300, rainbow_colors[5], "Perfect_Rainbow_5"},
            {SizeBoostLevel::Perfect, 350, rainbow_colors[0], "Perfect_Rainbow_WrapAround"}, // Wraps around
        };
    }()),
    [](const testing::TestParamInfo<BoostFlashColorTest::ParamType>& info) { return info.param.description; }
);

// Test that rendering different score and level values works without crashing.
TEST_F(UiTest, ScoreboardRendersVariousValues) {
    Scoreboard scoreboard(renderer_.get(), config_);

    // We can't easily check the visual output, but we can confirm that
    // rendering different values completes without throwing any exceptions.
    EXPECT_NO_THROW({
        SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer_.get()); // NOLINT(readability-magic-numbers)
        scoreboard.render(0, 1, 80, 0, 5, 40, false, 0, SizeBoostLevel::None, 0, false, 0);        // Initial score, dash ready
        scoreboard.render(99999, 10, 60, 53, 10, 60, true, 1234, SizeBoostLevel::Good, 100, true, 50);   // High score and level, dash on cooldown
        scoreboard.render(-100, 5, 25, 28, 8, 20, false, 0, SizeBoostLevel::Perfect, 200, false, 0);     // Negative score (if possible in game)
        SDL_RenderPresent(renderer_.get());
    });
}