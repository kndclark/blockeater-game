use crate::config::Config;

#[derive(Debug, Clone)]
pub struct LevelManager {
    pub effective_spawn_interval: u32,
    pub effective_obstacle_speed: i32,
    pub effective_grow_chance: i32,
    pub effective_shrink_chance: i32,
    pub effective_hurt_chance: i32,
    pub effective_base_checkpoint_gap: i32,
    pub effective_checkpoints_per_level: i32,
    pub effective_checkpoint_interval_ms: u32,
    pub max_level: i32,
}

impl LevelManager {
    pub fn new(base_config: &Config) -> Self {
        let mut lm = Self {
            effective_spawn_interval: base_config.spawn_interval_ms,
            effective_obstacle_speed: base_config.obstacle_speed,
            effective_grow_chance: base_config.grow_chance_percent,
            effective_shrink_chance: base_config.shrink_chance_percent,
            effective_hurt_chance: base_config.hurt_chance_percent,
            effective_base_checkpoint_gap: base_config.base_checkpoint_gap,
            effective_checkpoints_per_level: base_config.checkpoints_per_level,
            effective_checkpoint_interval_ms: base_config.checkpoint_interval_ms,
            max_level: base_config.max_level,
        };
        lm.update_for_level(1, base_config);
        lm
    }

    pub fn update_for_level(&mut self, level: i32, base_config: &Config) {
        if let Some(lc) = base_config.get_level_config(level) {
            self.effective_spawn_interval = lc
                .spawn_interval_ms
                .unwrap_or(base_config.spawn_interval_ms);
            self.effective_obstacle_speed = lc.obstacle_speed.unwrap_or(base_config.obstacle_speed);
            self.effective_grow_chance = lc
                .grow_chance_percent
                .unwrap_or(base_config.grow_chance_percent);
            self.effective_shrink_chance = lc
                .shrink_chance_percent
                .unwrap_or(base_config.shrink_chance_percent);
            self.effective_hurt_chance = lc
                .hurt_chance_percent
                .unwrap_or(base_config.hurt_chance_percent);
            self.effective_base_checkpoint_gap = lc
                .base_checkpoint_gap
                .unwrap_or(base_config.base_checkpoint_gap);
            self.effective_checkpoints_per_level = lc
                .checkpoints_per_level
                .unwrap_or(base_config.checkpoints_per_level);
            self.effective_checkpoint_interval_ms = lc
                .checkpoint_interval_ms
                .unwrap_or(base_config.checkpoint_interval_ms);
        } else {
            self.effective_spawn_interval = base_config.spawn_interval_ms;
            self.effective_obstacle_speed = base_config.obstacle_speed;
            self.effective_grow_chance = base_config.grow_chance_percent;
            self.effective_shrink_chance = base_config.shrink_chance_percent;
            self.effective_hurt_chance = base_config.hurt_chance_percent;
            self.effective_base_checkpoint_gap = base_config.base_checkpoint_gap;
            self.effective_checkpoints_per_level = base_config.checkpoints_per_level;
            self.effective_checkpoint_interval_ms = base_config.checkpoint_interval_ms;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_level_manager_defaults_and_override() {
        let config = Config::load("").unwrap_or_default();
        let mut lm = LevelManager::new(&config);

        assert_eq!(lm.effective_obstacle_speed, 3);
        assert_eq!(lm.effective_base_checkpoint_gap, 200);

        lm.update_for_level(5, &config);
        assert_eq!(lm.effective_obstacle_speed, 7);
        assert_eq!(lm.effective_base_checkpoint_gap, 105);

        // Non-existent level falls back to base config
        lm.update_for_level(999, &config);
        assert_eq!(lm.effective_obstacle_speed, config.obstacle_speed);
        assert_eq!(lm.effective_base_checkpoint_gap, config.base_checkpoint_gap);
    }
}
