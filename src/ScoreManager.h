#pragma once

#include "../config/Config.h"

struct GameState; // Forward declaration to break include cycle

enum class SizeBoostLevel {
    None,
    Good,
    Great,
    Perfect
};

struct ScoreCalculationResult {
    int score;
    bool dash_boost_applied = false;
    SizeBoostLevel size_boost_level = SizeBoostLevel::None;
};

class ScoreManager {
public:
    explicit ScoreManager(const Config& config) : config_(config) {}

    ScoreCalculationResult calculateScore(int base_score, const GameState& game_state) const;
    bool applyPenalty(int& score, int penalty) const;

private:
    const Config& config_;
};