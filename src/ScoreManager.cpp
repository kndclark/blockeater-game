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
        float size_percentage = (static_cast<float>(game_state.player.rect.w) / game_state.ui_next_checkpoint_gap_size) * 100.0f;
        if (size_percentage >= config_.getSizeBoostThreshold()) {
            final_score *= config_.getSizeBoostMultiplier();
            result.size_boost_applied = true;
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