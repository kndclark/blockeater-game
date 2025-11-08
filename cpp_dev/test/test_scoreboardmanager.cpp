#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "../src/ScoreboardManager.h"

class ScoreboardManagerTest : public ::testing::Test {
protected:
    const std::string test_dir = "temp_test_data/";
    const std::string test_filepath = test_dir + "scores.json";

    void SetUp() override {
        // Create a temporary directory to simulate the 'data' folder
        std::system(("mkdir -p " + test_dir).c_str());
    }

    void TearDown() override {
        std::system(("rm -rf " + test_dir).c_str());
    }
};

TEST_F(ScoreboardManagerTest, AddsAndSortsScores) {
    ScoreboardManager sm(test_filepath);

    sm.addScore("PlayerC", 300);
    sm.addScore("PlayerA", 100);
    sm.addScore("PlayerB", 500);

    const auto& scores = sm.getScores();
    ASSERT_EQ(scores.size(), 3);
    EXPECT_EQ(scores[0].name, "PlayerB");
    EXPECT_EQ(scores[0].score, 500);
    EXPECT_EQ(scores[1].name, "PlayerC");
    EXPECT_EQ(scores[1].score, 300);
    EXPECT_EQ(scores[2].name, "PlayerA");
    EXPECT_EQ(scores[2].score, 100);
}

TEST_F(ScoreboardManagerTest, LimitsScoresToTen) {
    ScoreboardManager sm(test_filepath);

    for (int i = 0; i < 12; ++i) {
        sm.addScore("Player" + std::to_string(i), i * 100);
    }

    const auto& scores = sm.getScores();
    ASSERT_EQ(scores.size(), 10);
    // The lowest score should be 200 (from players 0 and 1 being dropped)
    EXPECT_EQ(scores.back().score, 200);
}

TEST_F(ScoreboardManagerTest, SavesAndLoadsScores) {
    // Step 1: Create a manager, add scores, and let it save.
    {
        ScoreboardManager sm1(test_filepath);
        sm1.addScore("Hero", 9001);
        sm1.addScore("Zero", 10);
    } // sm1 goes out of scope, its destructor doesn't save, but addScore does.

    // Step 2: Verify the file was created and has content.
    std::ifstream f(test_filepath);
    ASSERT_TRUE(f.good());
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_FALSE(content.empty());
    f.close();

    // Step 3: Create a new manager and check if it loads the scores.
    ScoreboardManager sm2(test_filepath);
    const auto& scores = sm2.getScores();
    ASSERT_EQ(scores.size(), 2);
    EXPECT_EQ(scores[0].name, "Hero");
    EXPECT_EQ(scores[0].score, 9001);
    EXPECT_EQ(scores[1].name, "Zero");
    EXPECT_EQ(scores[1].score, 10);
}

TEST_F(ScoreboardManagerTest, CreatesFileOnAddIfNonExistent) {
    // 1. Verify the file does not exist initially.
    {
        std::ifstream f(test_filepath);
        ASSERT_FALSE(f.good());
    }

    // 2. Create a ScoreboardManager. It should be empty.
    ScoreboardManager sm(test_filepath);
    EXPECT_TRUE(sm.getScores().empty());

    // 3. Adding a score should work and create the file.
    sm.addScore("First", 100);
    EXPECT_EQ(sm.getScores().size(), 1);

    // 4. Verify the file now exists.
    std::ifstream f(test_filepath);
    EXPECT_TRUE(f.good()) << "File " << test_filepath << " was not created.";
}

TEST_F(ScoreboardManagerTest, DoesNotAddZeroOrNegativeScores) {
    ScoreboardManager sm(test_filepath);

    sm.addScore("Zero", 0);
    sm.addScore("Negative", -100);

    const auto& scores = sm.getScores();
    EXPECT_TRUE(scores.empty());
}