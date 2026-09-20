use crate::config::Config;
use crate::types::ScoreCalculationResult;

#[derive(Debug, Clone)]
pub struct ScoreManager;

impl ScoreManager {
    pub fn calculate_score(
        base_score: i32,
        is_dashing: bool,
        player_width: i32,
        ui_next_checkpoint_gap_size: i32,
        config: &Config,
    ) -> ScoreCalculationResult {
        let mut final_score = base_score as f32;
        let mut dash_boost_applied = false;
        let mut size_boost_applied = false;

        if is_dashing {
            final_score *= config.dash_boost_multiplier;
            dash_boost_applied = true;
        }

        if ui_next_checkpoint_gap_size > 0 {
            let size_percentage =
                (player_width as f32 / ui_next_checkpoint_gap_size as f32) * 100.0;
            // Float precision guard matching C++ behaviour
            if size_percentage + 0.0001 >= config.size_boost_threshold {
                final_score *= config.size_boost_multiplier;
                size_boost_applied = true;
            }
        }

        ScoreCalculationResult {
            score: final_score as i32,
            dash_boost_applied,
            size_boost_applied,
        }
    }

    pub fn apply_penalty(score: &mut i32, penalty: i32) -> bool {
        let abs_penalty = penalty.abs();
        if *score >= abs_penalty {
            *score -= abs_penalty;
            true // Player survives
        } else {
            false // Player does not have enough score
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_apply_penalty_survives_and_dies() {
        let mut score = 600;
        assert!(ScoreManager::apply_penalty(&mut score, -500));
        assert_eq!(score, 100);

        // Not enough score to survive another 500 penalty
        assert!(!ScoreManager::apply_penalty(&mut score, -500));
        assert_eq!(score, 100); // Unchanged on failure
    }

    #[test]
    fn test_calculate_score_boost_detection() {
        let config = Config::load("").unwrap_or_default();
        let res = ScoreManager::calculate_score(100, true, 20, 200, &config);
        assert_eq!(res.score, 150);
        assert!(res.dash_boost_applied);
        assert!(!res.size_boost_applied);
    }
}
