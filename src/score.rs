use crate::config::Config;
use crate::types::{ObstacleType, ScoreCalculationResult, SizeBoostLevel};

#[derive(Debug, Clone)]
pub struct ScoreManager;

impl ScoreManager {
    pub fn calculate_score(
        base_score: i32,
        is_dashing: bool,
        player_width: i32,
        ui_next_checkpoint_gap_size: i32,
        obstacle_type: ObstacleType,
        config: &Config,
    ) -> ScoreCalculationResult {
        let mut final_score = base_score as f32;
        let mut dash_boost_applied = false;
        let mut size_boost_level = SizeBoostLevel::None;

        if is_dashing {
            final_score *= config.dash_boost_multiplier;
            dash_boost_applied = true;
        }

        if obstacle_type == ObstacleType::Checkpoint && ui_next_checkpoint_gap_size > 0 {
            let raw_percentage = (player_width as f64 / ui_next_checkpoint_gap_size as f64) * 100.0;
            let rounded_percentage = raw_percentage.round() as i32;

            for tier in &config.size_boost_tiers {
                if rounded_percentage >= tier.threshold_percent {
                    final_score *= tier.multiplier;
                    size_boost_level = tier.level();
                    break;
                }
            }
        }

        ScoreCalculationResult {
            score: final_score as i32,
            dash_boost_applied,
            size_boost_level,
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
        let res =
            ScoreManager::calculate_score(100, true, 20, 200, ObstacleType::Checkpoint, &config);
        assert_eq!(res.score, 150);
        assert!(res.dash_boost_applied);
        assert_eq!(res.size_boost_level, SizeBoostLevel::None);

        // Test Good tier (30%)
        let res_good =
            ScoreManager::calculate_score(100, false, 30, 100, ObstacleType::Checkpoint, &config);
        assert_eq!(res_good.score, 150);
        assert_eq!(res_good.size_boost_level, SizeBoostLevel::Good);

        // Test Perfect tier (80%)
        let res_perfect =
            ScoreManager::calculate_score(100, false, 80, 100, ObstacleType::Checkpoint, &config);
        assert_eq!(res_perfect.score, 500);
        assert_eq!(res_perfect.size_boost_level, SizeBoostLevel::Perfect);
    }
}
