#include "ScoreManager.h"
#include "GameState.h"
#include "../config/Config.h"
#include "Obstacle.h"

ScoreCalculationResult ScoreManager::calculateScore(int base_score, const GameState& game_state, ObstacleType obstacle_type) const {
    ScoreCalculationResult result;
    float final_score = static_cast<float>(base_score);

    // Apply dash boost if the player is currently dashing.
    if (game_state.player.state == PlayerState::Dashing) {
        final_score *= config_.getDashBoostMultiplier();
        result.dash_boost_applied = true;
    }

    // Apply size boost ONLY for checkpoints, based on the player's width as a percentage of the upcoming gap size.
    if (obstacle_type == ObstacleType::Checkpoint && game_state.ui_next_checkpoint_gap_size > 0) {
        const float raw_size_percentage = (static_cast<float>(game_state.player.rect.w) / game_state.ui_next_checkpoint_gap_size) * 100.0f;
        // Round to the nearest whole number to match what the UI displays.
        const int rounded_percentage = static_cast<int>(std::round(raw_size_percentage));
        const auto& tiers = config_.getSizeBoostTiers();

        // Tiers are sorted by threshold descending. Find the first tier the player qualifies for based on the rounded percentage.
        auto tier_it = std::find_if(tiers.cbegin(), tiers.cend(), [rounded_percentage](const auto& tier) {
            return rounded_percentage >= tier.threshold_percent;
        });

        if (tier_it != tiers.cend()) {
            final_score *= tier_it->multiplier;
            if (tier_it->tier == "Perfect") result.size_boost_level = SizeBoostLevel::Perfect;
            else if (tier_it->tier == "Great") result.size_boost_level = SizeBoostLevel::Great;
            else if (tier_it->tier == "Good") result.size_boost_level = SizeBoostLevel::Good;
            else result.size_boost_level = SizeBoostLevel::None;
        }
    }

    result.score = static_cast<int>(final_score);
    return result;
}

bool ScoreManager::applyPenalty(int& score, int penalty) const {
    const int abs_penalty = std::abs(penalty);
    if (score >= abs_penalty) {
        score -= abs_penalty;
        SDL_Log("Score decreased by %d.", abs_penalty);
        return true; // Player survives
    }
    return false; // Player does not have enough score
}