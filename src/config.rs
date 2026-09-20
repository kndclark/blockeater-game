use crate::types::{GameColor, ObstacleSize, ObstacleType};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::Path;

pub const DEFAULT_CONFIG_JSON: &str = include_str!("../config/json/config.json");
pub const DEFAULT_LEVELS_JSON: &str = include_str!("../config/json/levels.json");
pub const DEFAULT_UI_TEXTS_JSON: &str = include_str!("../config/json/ui_texts.json");

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize, Default)]
pub struct LevelConfig {
    pub spawn_interval_ms: Option<u32>,
    pub checkpoint_interval_ms: Option<u32>,
    pub obstacle_speed: Option<i32>,
    pub grow_chance_percent: Option<i32>,
    pub shrink_chance_percent: Option<i32>,
    pub hurt_chance_percent: Option<i32>,
    pub base_checkpoint_gap: Option<i32>,
    pub checkpoints_per_level: Option<i32>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ObstacleConfig {
    pub grow_chance: i32,
    pub shrink_chance: i32,
    pub hurt_chance: i32,
    pub grow_dims: ObstacleSize,
    pub shrink_dims: ObstacleSize,
    pub hurt_dims: ObstacleSize,
    pub grow_points: i32,
    pub shrink_points: i32,
    pub checkpoint_points: i32,
}

#[derive(Debug, Clone)]
pub struct Config {
    pub root_path: String,
    pub player_color: GameColor,
    pub obstacle_colors: HashMap<ObstacleType, GameColor>,
    pub target_fps: i32,
    pub screen_width: i32,
    pub screen_height: i32,
    pub max_level: i32,
    pub obstacle_speed: i32,
    pub base_checkpoint_gap: i32,
    pub player_size_change_amount: i32,
    pub score_per_checkpoint: i32,
    pub score_per_grow: i32,
    pub score_per_shrink: i32,
    pub score_per_hurt: i32,
    pub dash_boost_multiplier: f32,
    pub size_boost_threshold: f32,
    pub size_boost_multiplier: f32,
    pub checkpoints_per_level: i32,
    pub spawn_interval_ms: u32,
    pub checkpoint_interval_ms: u32,
    pub checkpoint_safe_zone_duration_ms: u32,
    pub grow_chance_percent: i32,
    pub shrink_chance_percent: i32,
    pub hurt_chance_percent: i32,
    pub grow_dims: ObstacleSize,
    pub shrink_dims: ObstacleSize,
    pub hurt_dims: ObstacleSize,
    pub player_initial_x: i32,
    pub player_width: i32,
    pub player_height: i32,
    pub player_speed: i32,
    pub dash_speed_multiplier: f32,
    pub dash_duration_ms: u32,
    pub dash_cooldown_ms: u32,

    // UI Text
    pub score_prefix: String,
    pub level_prefix: String,
    pub level_progress_prefix: String,
    pub level_progress_suffix: String,
    pub dash_ready_text: String,
    pub dash_cooldown_prefix: String,
    pub dash_cooldown_suffix: String,
    pub cooldown_indicator_radius: i32,
    pub cooldown_indicator_color: GameColor,
    pub gap_size_prefix: String,
    pub player_size_prefix: String,
    pub player_size_suffix: String,
    pub game_over_text: String,
    pub victory_text: String,
    pub game_over_instructions: String,
    pub pause_menu_title: String,
    pub main_menu_title: String,
    pub main_menu_instructions: String,
    pub pause_menu_instructions: String,
    pub settings_menu_title: String,
    pub settings_menu_instructions: String,
    pub scoreboard_title: String,
    pub scoreboard_instructions: String,
    pub enter_name_prompt: String,
    pub final_score_text: String,
    pub font_path: String,
    pub font_size: i32,
    pub ui_text_color: GameColor,

    pub player_color_choices: Vec<GameColor>,
    pub level_configs: HashMap<i32, LevelConfig>,
}

impl Default for Config {
    fn default() -> Self {
        let mut obstacle_colors = HashMap::new();
        obstacle_colors.insert(ObstacleType::Hurt, GameColor::new(120, 120, 120, 255));
        obstacle_colors.insert(ObstacleType::Grow, GameColor::new(140, 140, 140, 255));
        obstacle_colors.insert(ObstacleType::Shrink, GameColor::new(160, 160, 160, 255));
        obstacle_colors.insert(ObstacleType::Checkpoint, GameColor::new(100, 100, 200, 255));

        Self {
            root_path: String::new(),
            player_color: GameColor::new(100, 100, 100, 255),
            obstacle_colors,
            target_fps: 60,
            screen_width: 640,
            screen_height: 480,
            max_level: 10,
            obstacle_speed: 3,
            base_checkpoint_gap: 120,
            player_size_change_amount: 10,
            score_per_checkpoint: 10,
            score_per_grow: 200,
            score_per_shrink: 100,
            score_per_hurt: -500,
            dash_boost_multiplier: 1.5,
            size_boost_threshold: 50.0,
            size_boost_multiplier: 2.0,
            checkpoints_per_level: 10,
            spawn_interval_ms: 1500,
            checkpoint_interval_ms: 10000,
            checkpoint_safe_zone_duration_ms: 500,
            grow_chance_percent: 40,
            shrink_chance_percent: 40,
            hurt_chance_percent: 20,
            grow_dims: ObstacleSize { w: 40, h: 40 },
            shrink_dims: ObstacleSize { w: 20, h: 20 },
            hurt_dims: ObstacleSize { w: 30, h: 30 },
            player_initial_x: 100,
            player_width: 40,
            player_height: 40,
            player_speed: 5,
            dash_speed_multiplier: 2.5,
            dash_duration_ms: 500,
            dash_cooldown_ms: 2000,
            score_prefix: "Score: ".to_string(),
            level_prefix: "Level: ".to_string(),
            level_progress_prefix: " (".to_string(),
            level_progress_suffix: " checkpoints to next level)".to_string(),
            dash_ready_text: "dash -> ready".to_string(),
            dash_cooldown_prefix: "dash -> cooldown (".to_string(),
            dash_cooldown_suffix: "s)".to_string(),
            cooldown_indicator_radius: 12,
            cooldown_indicator_color: GameColor::new(255, 255, 255, 255),
            gap_size_prefix: "Gap Size: ".to_string(),
            player_size_prefix: "Player Size: ".to_string(),
            player_size_suffix: "% of gap size".to_string(),
            game_over_text: "GAME OVER".to_string(),
            victory_text: "YOU WIN!".to_string(),
            game_over_instructions: "R = Restart | M = Menu | Q = Quit".to_string(),
            pause_menu_title: "Paused".to_string(),
            main_menu_title: "THE BLOCKEATER".to_string(),
            main_menu_instructions: "S = Start Game | Q = Quit".to_string(),
            pause_menu_instructions: "ESC = Resume | R = Restart | M = Menu | Q = Quit".to_string(),
            settings_menu_title: "Settings".to_string(),
            settings_menu_instructions: "C = Change Color | T = Toggle Fullscreen | B = Back"
                .to_string(),
            scoreboard_title: "Scoreboard".to_string(),
            scoreboard_instructions: "B = Back to Menu".to_string(),
            enter_name_prompt: "Enter Your Name:".to_string(),
            final_score_text: "Final Score: ".to_string(),
            font_path: "assets/font.ttf".to_string(),
            font_size: 24,
            ui_text_color: GameColor::new(255, 255, 255, 255),
            player_color_choices: vec![
                GameColor::new(128, 0, 128, 255), // Purple
                GameColor::new(0, 128, 0, 255),   // Green
                GameColor::new(0, 0, 128, 255),   // Blue
                GameColor::new(255, 165, 0, 255), // Orange
            ],
            level_configs: HashMap::new(),
        }
    }
}

impl Config {
    pub fn load(root_path: &str) -> Result<Self, String> {
        let mut cfg = Self {
            root_path: root_path.to_string(),
            ..Default::default()
        };

        let base_p = Path::new(root_path);
        let config_file = if root_path.ends_with(".json") {
            base_p.to_path_buf()
        } else {
            base_p.join("config/json/config.json")
        };

        if config_file.exists() {
            if let Ok(content) = std::fs::read_to_string(&config_file) {
                cfg.parse_config_json(&content)?;
            }
        } else {
            // Fallback to embedded default config
            cfg.parse_config_json(DEFAULT_CONFIG_JSON)?;
        }

        let levels_file = base_p.join("config/json/levels.json");
        if levels_file.exists() {
            if let Ok(content) = std::fs::read_to_string(&levels_file) {
                cfg.parse_levels_json(&content);
            }
        } else {
            cfg.parse_levels_json(DEFAULT_LEVELS_JSON);
        }

        let ui_file = base_p.join("config/json/ui_texts.json");
        if ui_file.exists() {
            if let Ok(content) = std::fs::read_to_string(&ui_file) {
                cfg.parse_ui_texts_json(&content);
            }
        } else {
            cfg.parse_ui_texts_json(DEFAULT_UI_TEXTS_JSON);
        }

        cfg.validate()?;
        Ok(cfg)
    }

    pub fn load_from_single_file(filepath: &str) -> Result<Self, String> {
        let mut cfg = Self::default();
        let path = Path::new(filepath);
        if path.exists() {
            let content = std::fs::read_to_string(path).map_err(|e| e.to_string())?;
            cfg.parse_config_json(&content)?;
        } else {
            cfg.parse_config_json(DEFAULT_CONFIG_JSON)?;
        }
        cfg.validate()?;
        Ok(cfg)
    }

    pub fn parse_config_json(&mut self, json_str: &str) -> Result<(), String> {
        let val: serde_json::Value = match serde_json::from_str(json_str) {
            Ok(v) => v,
            Err(_) => return Ok(()), // Malformed fallback
        };

        if let Some(colors) = val.get("colors") {
            if let Some(p) = colors.get("player") {
                if let Ok(c) = serde_json::from_value::<GameColor>(p.clone()) {
                    self.player_color = c;
                }
            }
            if let Some(h) = colors.get("obstacle_hurt") {
                if let Ok(c) = serde_json::from_value::<GameColor>(h.clone()) {
                    self.obstacle_colors.insert(ObstacleType::Hurt, c);
                }
            }
            if let Some(g) = colors.get("obstacle_grow") {
                if let Ok(c) = serde_json::from_value::<GameColor>(g.clone()) {
                    self.obstacle_colors.insert(ObstacleType::Grow, c);
                }
            }
            if let Some(s) = colors.get("obstacle_shrink") {
                if let Ok(c) = serde_json::from_value::<GameColor>(s.clone()) {
                    self.obstacle_colors.insert(ObstacleType::Shrink, c);
                }
            }
            if let Some(cp) = colors.get("obstacle_checkpoint") {
                if let Ok(c) = serde_json::from_value::<GameColor>(cp.clone()) {
                    self.obstacle_colors.insert(ObstacleType::Checkpoint, c);
                }
            }
        }

        if let Some(game) = val.get("game") {
            if let Some(v) = game.get("base_checkpoint_gap").and_then(|v| v.as_i64()) {
                self.base_checkpoint_gap = v as i32;
            }
            if let Some(v) = game.get("checkpoint_interval_ms").and_then(|v| v.as_u64()) {
                self.checkpoint_interval_ms = v as u32;
            }
            if let Some(v) = game.get("checkpoints_per_level").and_then(|v| v.as_i64()) {
                self.checkpoints_per_level = v as i32;
            }
            if let Some(v) = game.get("obstacle_speed").and_then(|v| v.as_i64()) {
                self.obstacle_speed = v as i32;
            }
            if let Some(v) = game
                .get("player_size_change_amount")
                .and_then(|v| v.as_i64())
            {
                self.player_size_change_amount = v as i32;
            }
            if let Some(v) = game.get("score_per_checkpoint").and_then(|v| v.as_i64()) {
                self.score_per_checkpoint = v as i32;
            }
            if let Some(v) = game.get("score_per_grow").and_then(|v| v.as_i64()) {
                self.score_per_grow = v as i32;
            }
            if let Some(v) = game.get("score_per_shrink").and_then(|v| v.as_i64()) {
                self.score_per_shrink = v as i32;
            }
            if let Some(v) = game.get("score_per_hurt").and_then(|v| v.as_i64()) {
                self.score_per_hurt = v as i32;
            }
            if let Some(v) = game.get("spawn_interval_ms").and_then(|v| v.as_u64()) {
                self.spawn_interval_ms = v as u32;
            }

            if let Some(dims) = game.get("obstacle_dimensions") {
                if let Some(g) = dims.get("grow") {
                    if let Ok(d) = serde_json::from_value::<ObstacleSize>(g.clone()) {
                        self.grow_dims = d;
                    }
                }
                if let Some(s) = dims.get("shrink") {
                    if let Ok(d) = serde_json::from_value::<ObstacleSize>(s.clone()) {
                        self.shrink_dims = d;
                    }
                }
                if let Some(h) = dims.get("hurt") {
                    if let Ok(d) = serde_json::from_value::<ObstacleSize>(h.clone()) {
                        self.hurt_dims = d;
                    }
                }
            }

            if let Some(chances) = game.get("obstacle_spawn_chances") {
                if let Some(g) = chances.get("grow").and_then(|v| v.as_i64()) {
                    self.grow_chance_percent = g as i32;
                }
                if let Some(s) = chances.get("shrink").and_then(|v| v.as_i64()) {
                    self.shrink_chance_percent = s as i32;
                }
                if let Some(h) = chances.get("hurt").and_then(|v| v.as_i64()) {
                    self.hurt_chance_percent = h as i32;
                }
            }

            if let Some(player) = game.get("player") {
                if let Some(v) = player.get("initial_x").and_then(|v| v.as_i64()) {
                    self.player_initial_x = v as i32;
                }
                if let Some(v) = player.get("width").and_then(|v| v.as_i64()) {
                    self.player_width = v as i32;
                }
                if let Some(v) = player.get("height").and_then(|v| v.as_i64()) {
                    self.player_height = v as i32;
                }
                if let Some(v) = player.get("speed").and_then(|v| v.as_i64()) {
                    self.player_speed = v as i32;
                }
                if let Some(dash) = player.get("dash") {
                    if let Some(v) = dash.get("speed_multiplier").and_then(|v| v.as_f64()) {
                        self.dash_speed_multiplier = v as f32;
                    }
                    if let Some(v) = dash.get("duration_ms").and_then(|v| v.as_u64()) {
                        self.dash_duration_ms = v as u32;
                    }
                    if let Some(v) = dash.get("cooldown_ms").and_then(|v| v.as_u64()) {
                        self.dash_cooldown_ms = v as u32;
                    }
                }
            }

            if let Some(boosts) = game.get("score_boosts") {
                if let Some(v) = boosts.get("dash_multiplier").and_then(|v| v.as_f64()) {
                    self.dash_boost_multiplier = v as f32;
                }
                if let Some(v) = boosts.get("size_multiplier").and_then(|v| v.as_f64()) {
                    self.size_boost_multiplier = v as f32;
                }
                if let Some(v) = boosts
                    .get("size_threshold_percent")
                    .and_then(|v| v.as_f64())
                {
                    self.size_boost_threshold = v as f32;
                }
            }
        }

        if let Some(settings) = val.get("settings") {
            if let Some(v) = settings.get("screen_width").and_then(|v| v.as_i64()) {
                self.screen_width = v as i32;
            }
            if let Some(v) = settings.get("screen_height").and_then(|v| v.as_i64()) {
                self.screen_height = v as i32;
            }
            if let Some(v) = settings.get("target_fps").and_then(|v| v.as_i64()) {
                self.target_fps = v as i32;
            }
        }

        Ok(())
    }

    pub fn parse_levels_json(&mut self, json_str: &str) {
        if let Ok(val) = serde_json::from_str::<serde_json::Value>(json_str) {
            if let Some(max_l) = val.get("max_level").and_then(|v| v.as_i64()) {
                self.max_level = max_l as i32;
            }
            if let Some(levels) = val.get("levels").and_then(|v| v.as_object()) {
                for (k, v) in levels {
                    if let Ok(lvl_num) = k.parse::<i32>() {
                        if let Ok(lc) = serde_json::from_value::<LevelConfig>(v.clone()) {
                            self.level_configs.insert(lvl_num, lc);
                        }
                    }
                }
            }
        }
    }

    pub fn parse_ui_texts_json(&mut self, json_str: &str) {
        if let Ok(val) = serde_json::from_str::<serde_json::Value>(json_str) {
            if let Some(ui) = val.get("ui_text") {
                if let Some(v) = ui.get("score_prefix").and_then(|v| v.as_str()) {
                    self.score_prefix = v.to_string();
                }
                if let Some(v) = ui.get("level_prefix").and_then(|v| v.as_str()) {
                    self.level_prefix = v.to_string();
                }
                if let Some(v) = ui.get("level_progress_prefix").and_then(|v| v.as_str()) {
                    self.level_progress_prefix = v.to_string();
                }
                if let Some(v) = ui.get("level_progress_suffix").and_then(|v| v.as_str()) {
                    self.level_progress_suffix = v.to_string();
                }
                if let Some(v) = ui.get("gap_size_prefix").and_then(|v| v.as_str()) {
                    self.gap_size_prefix = v.to_string();
                }
                if let Some(v) = ui.get("player_size_prefix").and_then(|v| v.as_str()) {
                    self.player_size_prefix = v.to_string();
                }
                if let Some(v) = ui.get("player_size_suffix").and_then(|v| v.as_str()) {
                    self.player_size_suffix = v.to_string();
                }
                if let Some(v) = ui.get("dash_ready_text").and_then(|v| v.as_str()) {
                    self.dash_ready_text = v.to_string();
                }
                if let Some(v) = ui.get("dash_cooldown_prefix").and_then(|v| v.as_str()) {
                    self.dash_cooldown_prefix = v.to_string();
                }
                if let Some(v) = ui.get("dash_cooldown_suffix").and_then(|v| v.as_str()) {
                    self.dash_cooldown_suffix = v.to_string();
                }
                if let Some(v) = ui.get("game_over_text").and_then(|v| v.as_str()) {
                    self.game_over_text = v.to_string();
                }
                if let Some(v) = ui.get("victory_text").and_then(|v| v.as_str()) {
                    self.victory_text = v.to_string();
                }
                if let Some(v) = ui.get("game_over_instructions").and_then(|v| v.as_str()) {
                    self.game_over_instructions = v.to_string();
                }
                if let Some(v) = ui.get("pause_menu_title").and_then(|v| v.as_str()) {
                    self.pause_menu_title = v.to_string();
                }
                if let Some(v) = ui.get("pause_menu_instructions").and_then(|v| v.as_str()) {
                    self.pause_menu_instructions = v.to_string();
                }
                if let Some(v) = ui.get("main_menu_title").and_then(|v| v.as_str()) {
                    self.main_menu_title = v.to_string();
                }
                if let Some(v) = ui.get("main_menu_instructions").and_then(|v| v.as_str()) {
                    self.main_menu_instructions = v.to_string();
                }
                if let Some(v) = ui.get("settings_menu_title").and_then(|v| v.as_str()) {
                    self.settings_menu_title = v.to_string();
                }
                if let Some(v) = ui
                    .get("settings_menu_instructions")
                    .and_then(|v| v.as_str())
                {
                    self.settings_menu_instructions = v.to_string();
                }
                if let Some(v) = ui.get("scoreboard_title").and_then(|v| v.as_str()) {
                    self.scoreboard_title = v.to_string();
                }
                if let Some(v) = ui.get("scoreboard_instructions").and_then(|v| v.as_str()) {
                    self.scoreboard_instructions = v.to_string();
                }
                if let Some(v) = ui.get("enter_name_prompt").and_then(|v| v.as_str()) {
                    self.enter_name_prompt = v.to_string();
                }
                if let Some(v) = ui.get("final_score_text").and_then(|v| v.as_str()) {
                    self.final_score_text = v.to_string();
                }
                if let Some(tc) = ui.get("text_color") {
                    if let Ok(c) = serde_json::from_value::<GameColor>(tc.clone()) {
                        self.ui_text_color = c;
                    }
                }
                if let Some(cd) = ui.get("dash_cooldown_indicator") {
                    if let Some(r) = cd.get("radius").and_then(|v| v.as_i64()) {
                        self.cooldown_indicator_radius = r as i32;
                    }
                    if let Some(col) = cd.get("color") {
                        if let Ok(c) = serde_json::from_value::<GameColor>(col.clone()) {
                            self.cooldown_indicator_color = c;
                        }
                    }
                }
                if let Some(font) = ui.get("font") {
                    if let Some(p) = font.get("path").and_then(|v| v.as_str()) {
                        self.font_path = p.to_string();
                    }
                    if let Some(s) = font.get("size").and_then(|v| v.as_i64()) {
                        self.font_size = s as i32;
                    }
                }
            }
        }
    }

    pub fn validate(&self) -> Result<(), String> {
        if self.grow_chance_percent + self.shrink_chance_percent + self.hurt_chance_percent != 100 {
            return Err("Obstacle spawn chances in config must sum to 100.".to_string());
        }
        Ok(())
    }

    pub fn get_obstacle_color(&self, typ: ObstacleType) -> GameColor {
        self.obstacle_colors
            .get(&typ)
            .copied()
            .unwrap_or(GameColor::new(128, 128, 128, 255))
    }

    pub fn get_obstacle_config(&self) -> ObstacleConfig {
        ObstacleConfig {
            grow_chance: self.grow_chance_percent,
            shrink_chance: self.shrink_chance_percent,
            hurt_chance: self.hurt_chance_percent,
            grow_dims: self.grow_dims,
            shrink_dims: self.shrink_dims,
            hurt_dims: self.hurt_dims,
            grow_points: self.score_per_grow,
            shrink_points: self.score_per_shrink,
            checkpoint_points: self.score_per_checkpoint,
        }
    }

    pub fn get_level_config(&self, level: i32) -> Option<&LevelConfig> {
        self.level_configs.get(&level)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_embedded_config_defaults() {
        let mut cfg = Config::default();
        assert!(cfg.parse_config_json(DEFAULT_CONFIG_JSON).is_ok());
        assert!(cfg.validate().is_ok());
        assert_eq!(cfg.grow_chance_percent, 40);
        assert_eq!(cfg.shrink_chance_percent, 40);
        assert_eq!(cfg.hurt_chance_percent, 20);
        assert_eq!(cfg.base_checkpoint_gap, 200);
    }

    #[test]
    fn test_validation_fails_when_sum_not_100() {
        let cfg = Config {
            grow_chance_percent: 50,
            shrink_chance_percent: 50,
            hurt_chance_percent: 50, // Sum = 150
            ..Default::default()
        };
        assert!(cfg.validate().is_err());
    }
}
