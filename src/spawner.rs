use crate::config::Config;
use crate::level::LevelManager;
use crate::obstacle::Obstacle;
use crate::player::Player;
use crate::types::ObstacleType;

#[derive(Debug, Clone)]
pub struct ObstacleSpawner {
    pub last_spawn_time: u32,
    pub last_checkpoint_spawn_time: u32,
    pub checkpoint_safe_zone_duration: u32,
    pub screen_width: i32,
    pub screen_height: i32,
    pub player_size_change_amount: i32,
    pub shrink_powerups_since_checkpoint: Vec<i32>,
}

impl ObstacleSpawner {
    pub fn new(
        safe_zone_duration: u32,
        width: i32,
        height: i32,
        size_change: i32,
        start_time: u32,
    ) -> Self {
        Self {
            last_spawn_time: start_time,
            last_checkpoint_spawn_time: start_time,
            checkpoint_safe_zone_duration: safe_zone_duration,
            screen_width: width,
            screen_height: height,
            player_size_change_amount: size_change,
            shrink_powerups_since_checkpoint: Vec::new(),
        }
    }

    pub fn calculate_checkpoint_gap_size(&self, base_gap: i32) -> i32 {
        let shrink_count = self.shrink_powerups_since_checkpoint.len() as i32;
        let min_gap_height = Player::MIN_SIZE + 5;
        let shrink_effect = -shrink_count * self.player_size_change_amount;
        let calculated = base_gap + shrink_effect;

        calculated.max(min_gap_height).min(self.screen_height - 20)
    }

    pub fn spawn_obstacles(
        &mut self,
        current_time: u32,
        level_manager: &LevelManager,
        config: &Config,
        obstacles: &mut Vec<Obstacle>,
        ui_next_checkpoint_gap_size: &mut i32,
        next_checkpoint_gap_size: &mut i32,
    ) {
        let mut nearby_obstacles = Vec::new();
        let threshold_x = self.screen_width * 3 / 4;
        for obs in obstacles.iter() {
            if obs.rect.x > threshold_x {
                nearby_obstacles.push(obs.clone());
            }
        }

        let base_gap = level_manager.effective_base_checkpoint_gap;

        // Prioritize spawning checkpoints
        if current_time > 0
            && current_time
                >= self.last_checkpoint_spawn_time + level_manager.effective_checkpoint_interval_ms
        {
            self.last_checkpoint_spawn_time = current_time;
            let gap_height = self.calculate_checkpoint_gap_size(base_gap);
            let mut gap_y = 0;
            let checkpoint = Obstacle::create_checkpoint(
                self.screen_width,
                self.screen_height,
                level_manager.effective_obstacle_speed,
                gap_height,
                config.score_per_checkpoint,
                &nearby_obstacles,
                &mut gap_y,
            );
            obstacles.push(checkpoint);

            self.shrink_powerups_since_checkpoint.clear();
            *ui_next_checkpoint_gap_size = gap_height;
            *next_checkpoint_gap_size = self.calculate_checkpoint_gap_size(base_gap);
            self.last_spawn_time = current_time;
        } else if current_time > 0
            && current_time >= self.last_spawn_time + level_manager.effective_spawn_interval
            && current_time >= self.last_checkpoint_spawn_time + self.checkpoint_safe_zone_duration
        {
            self.last_spawn_time = current_time;
            let obs_cfg = config.get_obstacle_config();
            let new_obstacle = Obstacle::create_regular(
                self.screen_width,
                self.screen_height,
                level_manager.effective_obstacle_speed,
                &obs_cfg,
                &nearby_obstacles,
            );

            if new_obstacle.obstacle_type == ObstacleType::Shrink {
                self.shrink_powerups_since_checkpoint.push(1);
                *next_checkpoint_gap_size = self.calculate_checkpoint_gap_size(base_gap);
            }

            obstacles.push(new_obstacle);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_spawner_gap_calculation_logic() {
        let spawner = ObstacleSpawner::new(500, 640, 480, 10, 0);
        assert_eq!(spawner.calculate_checkpoint_gap_size(200), 200);

        let mut spawner_with_shrinks = spawner.clone();
        spawner_with_shrinks.shrink_powerups_since_checkpoint = vec![1, 1, 1];
        // 200 - 3 * 10 = 170
        assert_eq!(spawner_with_shrinks.calculate_checkpoint_gap_size(200), 170);

        // Clamping to Player::MIN_SIZE + 5 = 25
        spawner_with_shrinks.shrink_powerups_since_checkpoint = vec![1; 30];
        assert_eq!(spawner_with_shrinks.calculate_checkpoint_gap_size(200), 25);
    }

    #[test]
    fn test_spawner_clears_trackers_on_checkpoint() {
        let config = Config::load("").unwrap_or_default();
        let lm = LevelManager::new(&config);
        let mut spawner = ObstacleSpawner::new(500, 640, 480, 10, 0);
        spawner.shrink_powerups_since_checkpoint.push(1);

        let mut obstacles = Vec::new();
        let mut ui_gap = 200;
        let mut next_gap = 200;

        let cp_time = lm.effective_checkpoint_interval_ms;
        spawner.spawn_obstacles(
            cp_time,
            &lm,
            &config,
            &mut obstacles,
            &mut ui_gap,
            &mut next_gap,
        );

        assert_eq!(obstacles.len(), 1);
        assert_eq!(obstacles[0].obstacle_type, ObstacleType::Checkpoint);
        assert!(spawner.shrink_powerups_since_checkpoint.is_empty());
        assert_eq!(ui_gap, 190); // 200 - 10
        assert_eq!(next_gap, 200); // Reset for next
    }
}
