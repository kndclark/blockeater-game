#include "ScoreManager.h"
#include "GameState.h"

ScoreCalculationResult ScoreManager::calculateScore(int base_score, const GameState& game_state) const {
    ScoreCalculationResult result;
    float final_score = static_cast<float>(base_score);

    // Apply dash boost if the player is currently dashing.
    if (game_state.player.state == PlayerState::Dashing) {
        final_score *= config_.getDashBoostMultiplier();
        result.dash_boost_applied = true;
    }

    // Apply size boost if the player's width is a certain percentage of the upcoming gap size.
    if (game_state.ui_next_checkpoint_gap_size > 0) {
        const float size_percentage = (static_cast<float>(game_state.player.rect.w) / game_state.ui_next_checkpoint_gap_size) * 100.0f;
        const auto& tiers = config_.getSizeBoostTiers();

        // Tiers should be sorted by threshold descending. Find the first tier the player qualifies for.
        auto tier_it = std::find_if(tiers.cbegin(), tiers.cend(), [size_percentage](const auto& tier) {
            return size_percentage >= tier.threshold_percent;
        });

        if (tier_it != tiers.cend()) {
            final_score *= tier_it->multiplier;
            // This assumes 3 tiers: Perfect, Great, Good.
            result.size_boost_level = static_cast<SizeBoostLevel>(std::distance(tiers.cbegin(), tier_it) + 1);
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