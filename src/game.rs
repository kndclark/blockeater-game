use crate::config::Config;
use crate::level::LevelManager;
use crate::obstacle::Obstacle;
use crate::player::Player;
use crate::score::ScoreManager;
use crate::spawner::ObstacleSpawner;
use crate::types::{IntRect, ObstacleType};

#[derive(Debug, Clone)]
pub struct GameState {
    pub config: Config,
    pub player: Player,
    pub obstacles: Vec<Obstacle>,
    pub score: i32,
    pub level: i32,
    pub checkpoints_passed_in_level: i32,
    pub checkpoints_passed: i32,
    pub ui_next_checkpoint_gap_size: i32,
    pub next_checkpoint_gap_size: i32,
    pub level_manager: LevelManager,
    pub spawner: ObstacleSpawner,

    pub running: bool,
    pub victory: bool,
    pub paused: bool,

    pub frame_count: u32,
    pub last_fps_update_time: u32,

    // Scratch batch rects for rendering and zero allocation
    pub hurt_rects: Vec<IntRect>,
    pub grow_rects: Vec<IntRect>,
    pub shrink_rects: Vec<IntRect>,
    pub checkpoint_rects: Vec<IntRect>,
}

impl GameState {
    pub fn new(config: Config, screen_width: i32, screen_height: i32) -> Self {
        let player = Player::new(
            config.player_initial_x,
            screen_height / 2 - config.player_height / 2,
            config.player_width,
            config.player_height,
            config.player_speed,
            config.player_color,
            config.dash_speed_multiplier,
            config.dash_duration_ms,
            config.dash_cooldown_ms,
        );

        let level_manager = LevelManager::new(&config);
        let spawner = ObstacleSpawner::new(
            config.checkpoint_safe_zone_duration_ms,
            screen_width,
            screen_height,
            config.player_size_change_amount,
            0,
        );

        let initial_gap =
            spawner.calculate_checkpoint_gap_size(level_manager.effective_base_checkpoint_gap);

        Self {
            config,
            player,
            obstacles: Vec::new(),
            score: 0,
            level: 1,
            checkpoints_passed_in_level: 0,
            checkpoints_passed: 0,
            ui_next_checkpoint_gap_size: initial_gap,
            next_checkpoint_gap_size: initial_gap,
            level_manager,
            spawner,
            running: true,
            victory: false,
            paused: false,
            frame_count: 0,
            last_fps_update_time: 1, // Non-zero per tests
            hurt_rects: Vec::new(),
            grow_rects: Vec::new(),
            shrink_rects: Vec::new(),
            checkpoint_rects: Vec::new(),
        }
    }

    pub fn handle_collision_at(&mut self, idx: usize) -> bool {
        let obs = self.obstacles[idx].clone();
        match obs.obstacle_type {
            ObstacleType::Checkpoint => {
                self.running = false;
                false
            }
            ObstacleType::Hurt => {
                if ScoreManager::apply_penalty(&mut self.score, self.config.score_per_hurt) {
                    self.obstacles.remove(idx);
                    true
                } else {
                    self.running = false;
                    false
                }
            }
            ObstacleType::Grow => {
                let res = ScoreManager::calculate_score(
                    obs.points,
                    self.player.is_dashing,
                    self.player.rect.w,
                    self.ui_next_checkpoint_gap_size,
                    &self.config,
                );
                self.score += res.score;
                self.player.grow(self.config.player_size_change_amount);
                self.obstacles.remove(idx);
                true
            }
            ObstacleType::Shrink => {
                let res = ScoreManager::calculate_score(
                    obs.points,
                    self.player.is_dashing,
                    self.player.rect.w,
                    self.ui_next_checkpoint_gap_size,
                    &self.config,
                );
                self.score += res.score;
                self.player.shrink(self.config.player_size_change_amount);
                self.obstacles.remove(idx);
                true
            }
        }
    }

    pub fn handle_checkpoint_passing(&mut self, idx: usize) {
        let obs = &mut self.obstacles[idx];
        if obs.obstacle_type == ObstacleType::Checkpoint
            && !obs.passed
            && self.player.rect.x > obs.rect.x + obs.rect.w
        {
            obs.passed = true;
            let res = ScoreManager::calculate_score(
                self.config.score_per_checkpoint,
                self.player.is_dashing,
                self.player.rect.w,
                self.ui_next_checkpoint_gap_size,
                &self.config,
            );
            self.score += res.score;
            self.checkpoints_passed_in_level += 1;
            self.checkpoints_passed += 1;
            self.player.reset_size();

            if self.checkpoints_passed_in_level
                >= self.level_manager.effective_checkpoints_per_level
            {
                self.level += 1;
                self.checkpoints_passed_in_level = 0;
                self.level_manager
                    .update_for_level(self.level, &self.config);
            }
        }
    }

    pub fn check_victory_condition(&mut self) {
        if self.level > self.level_manager.max_level {
            self.victory = true;
            self.running = false;
        }
    }

    pub fn update(&mut self, current_time: u32) {
        self.player.update(current_time);

        self.spawner.spawn_obstacles(
            current_time,
            &self.level_manager,
            &self.config,
            &mut self.obstacles,
            &mut self.ui_next_checkpoint_gap_size,
            &mut self.next_checkpoint_gap_size,
        );

        Obstacle::update_and_remove(&mut self.obstacles);

        let mut idx = 0;
        while idx < self.obstacles.len() {
            let obs = &self.obstacles[idx];
            let mut collided = self.player.rect.intersects(&obs.rect);
            if let Some(r2) = obs.rect2 {
                collided = collided || self.player.rect.intersects(&r2);
            }

            if collided {
                let removed = self.handle_collision_at(idx);
                if !self.running {
                    break;
                }
                if !removed {
                    idx += 1;
                }
            } else {
                self.handle_checkpoint_passing(idx);
                idx += 1;
            }

            if !self.running {
                break;
            }
        }

        if self.running {
            self.check_victory_condition();
        }
    }
}

pub fn calculate_fps(
    frame_count: &mut u32,
    last_fps_update_time: &mut u32,
    current_time: u32,
) -> Option<f32> {
    *frame_count += 1;
    if current_time.saturating_sub(*last_fps_update_time) >= 1000 {
        let elapsed = (current_time - *last_fps_update_time) as f32 / 1000.0;
        let fps = *frame_count as f32 / elapsed;
        *frame_count = 0;
        *last_fps_update_time = current_time;
        Some(fps)
    } else {
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_game_state_flow() {
        let config = Config::load("").unwrap_or_default();
        let mut gs = GameState::new(config.clone(), 640, 480);
        assert!(gs.running);
        assert!(!gs.victory);
        assert_eq!(gs.score, 0);

        // Add a Grow obstacle at (100, 100) colliding with player
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.obstacles.push(Obstacle::new_regular(
            100,
            100,
            40,
            40,
            3,
            ObstacleType::Grow,
            200,
        ));

        gs.update(0);
        assert_eq!(gs.score, 200);
        assert_eq!(gs.player.rect.w, 50);
        assert!(gs.obstacles.is_empty());
    }

    #[test]
    fn test_fps_counter_unit() {
        let mut frame_count = 0;
        let mut last_fps_update_time = 0;

        assert_eq!(
            calculate_fps(&mut frame_count, &mut last_fps_update_time, 500),
            None
        );
        assert_eq!(frame_count, 1);

        frame_count = 59;
        let res = calculate_fps(&mut frame_count, &mut last_fps_update_time, 1000);
        assert!(res.is_some());
        assert!((res.unwrap() - 60.0).abs() < 0.01);
        assert_eq!(last_fps_update_time, 1000);
    }
}
