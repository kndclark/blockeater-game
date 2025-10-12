#pragma once

#include "../config/Config.h"

struct GameState; // Forward declaration to break include cycle

struct ScoreCalculationResult {
    int score;
    bool dash_boost_applied = false;
    bool size_boost_applied = false;
};

class ScoreManager {
public:
    explicit ScoreManager(const Config& config) : config_(config) {}

    ScoreCalculationResult calculateScore(int base_score, const GameState& game_state) const;
    bool applyPenalty(int& score, int penalty) const;

private:
    const Config& config_;
};