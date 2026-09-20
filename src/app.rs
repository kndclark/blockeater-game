//! Application state machine, lifecycle management, and game loop execution.

use std::collections::HashSet;
use std::time::{Duration, Instant};

use macroquad::prelude::*;

use crate::config::Config;
use crate::game::GameState;
use crate::particles::ParticleSystem;
use crate::platform::toggle_fullscreen;
use crate::player::PlayerInput;
use crate::render::{
    render_game_over_view, render_game_scene, render_main_menu_screen, render_scoreboard_screen,
    render_settings_screen, Viewport,
};
use crate::scoreboard::ScoreboardManager;
use crate::types::{
    AppStatus, GameOverAction, MainMenuAction, PauseMenuAction, SettingsMenuAction,
};
use crate::ui::{
    handle_game_over_key, handle_main_menu_key, handle_pause_menu_key, handle_settings_menu_key,
};

/// Main application orchestrator.
pub struct App {
    pub config: Config,
    pub scoreboard_manager: ScoreboardManager,
    pub particle_system: ParticleSystem,
    pub app_status: AppStatus,
    pub game_state: Option<GameState>,

    pub in_color_picker: bool,
    pub color_selection: usize,

    pub player_name_input: String,
    pub entering_name: bool,
    pub final_score: i32,
    pub game_over_message: String,

    pub is_fullscreen: bool,
    pub held_keys_on_death: HashSet<KeyCode>,

    pub time_accumulator: f32,
    pub fixed_dt: f32,
    pub fixed_dt_ms: u32,
    pub sim_time_ms: u32,

    pub screen_shake: f32,
    pub target_frame_duration: Duration,
}

impl Default for App {
    fn default() -> Self {
        Self::new()
    }
}

impl App {
    pub fn new() -> Self {
        let config = Config::load("").unwrap_or_default();
        let scoreboard_manager = ScoreboardManager::new("data/scores.json");
        let particle_system = ParticleSystem::new();

        let target_fps = config.target_fps.max(30) as u64;
        let fixed_dt = 1.0 / (target_fps as f32);
        let fixed_dt_ms = (fixed_dt * 1000.0) as u32;
        let target_frame_duration = Duration::from_micros(1_000_000 / target_fps);

        Self {
            config,
            scoreboard_manager,
            particle_system,
            app_status: AppStatus::ShowingMainMenu,
            game_state: None,
            in_color_picker: false,
            color_selection: 0,
            player_name_input: String::new(),
            entering_name: false,
            final_score: 0,
            game_over_message: String::new(),
            is_fullscreen: false,
            held_keys_on_death: HashSet::new(),
            time_accumulator: 0.0,
            fixed_dt,
            fixed_dt_ms,
            sim_time_ms: 0,
            screen_shake: 0.0,
            target_frame_duration,
        }
    }

    pub fn start_new_game(&mut self) {
        self.game_state = Some(GameState::new(
            self.config.clone(),
            self.config.screen_width,
            self.config.screen_height,
        ));
        self.particle_system.clear();
        self.time_accumulator = 0.0;
        self.sim_time_ms = 0;
        self.held_keys_on_death.clear();
        self.app_status = AppStatus::Running;
    }

    pub fn handle_main_menu(&mut self) {
        if let Some(key) = get_last_key_pressed() {
            if let Some(action) = handle_main_menu_key(key) {
                match action {
                    MainMenuAction::StartGame => {
                        clear_input_queue();
                        self.start_new_game();
                    }
                    MainMenuAction::ShowScoreboard => {
                        self.app_status = AppStatus::ShowingScoreboard;
                    }
                    MainMenuAction::Settings => {
                        self.in_color_picker = false;
                        self.app_status = AppStatus::ShowingSettingsMenu;
                    }
                    MainMenuAction::Quit => {
                        self.app_status = AppStatus::Quitting;
                    }
                }
            }
        }
    }

    pub fn handle_scoreboard(&mut self) {
        if is_key_pressed(KeyCode::B) || is_key_pressed(KeyCode::Escape) {
            self.app_status = AppStatus::ShowingMainMenu;
        }
    }

    pub fn handle_settings(&mut self) {
        if let Some(key) = get_last_key_pressed() {
            let choices_len = self.config.player_color_choices.len();
            if let Some(action) = handle_settings_menu_key(
                key,
                &mut self.in_color_picker,
                &mut self.color_selection,
                choices_len,
            ) {
                match action {
                    SettingsMenuAction::Back => {
                        self.app_status = AppStatus::ShowingMainMenu;
                    }
                    SettingsMenuAction::ToggleFullscreen => {
                        toggle_fullscreen(&mut self.is_fullscreen);
                    }
                    SettingsMenuAction::ChangePlayerColor => {
                        if self.color_selection < self.config.player_color_choices.len() {
                            self.config.player_color =
                                self.config.player_color_choices[self.color_selection];
                        }
                    }
                }
            }
        }
    }

    pub fn handle_running(&mut self, dt: f32, viewport: Viewport, shake_offset: Vec2) -> bool {
        let Some(gs) = self.game_state.as_mut() else {
            self.app_status = AppStatus::ShowingMainMenu;
            return true;
        };

        if is_key_pressed(KeyCode::Escape) {
            gs.paused = !gs.paused;
            clear_input_queue();
        }

        if gs.paused {
            self.time_accumulator = 0.0;
            if let Some(key) = get_last_key_pressed() {
                if let Some(action) = handle_pause_menu_key(key) {
                    match action {
                        PauseMenuAction::Resume => gs.paused = false,
                        PauseMenuAction::Restart => {
                            self.start_new_game();
                            return true;
                        }
                        PauseMenuAction::MainMenu => {
                            self.app_status = AppStatus::ShowingMainMenu;
                            return true;
                        }
                        PauseMenuAction::Quit => {
                            self.app_status = AppStatus::Quitting;
                            return false;
                        }
                    }
                }
            }
        } else {
            clear_input_queue();

            self.time_accumulator += dt;
            if self.time_accumulator > 0.2 {
                self.time_accumulator = 0.2;
            }

            while self.time_accumulator >= self.fixed_dt {
                self.time_accumulator -= self.fixed_dt;
                self.sim_time_ms = self.sim_time_ms.wrapping_add(self.fixed_dt_ms);

                let input = PlayerInput::new(
                    is_key_down(KeyCode::Left) || is_key_down(KeyCode::A),
                    is_key_down(KeyCode::Right) || is_key_down(KeyCode::D),
                    is_key_down(KeyCode::Up) || is_key_down(KeyCode::W),
                    is_key_down(KeyCode::Down) || is_key_down(KeyCode::S),
                    is_key_down(KeyCode::LeftShift) || is_key_down(KeyCode::RightShift),
                );

                gs.player.apply_input(
                    &input,
                    self.config.screen_width,
                    self.config.screen_height,
                    self.sim_time_ms,
                );

                // Dash particle trail
                if gs.player.is_dashing {
                    let p_center = Vec2::new(
                        (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                        (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                    );
                    self.particle_system.spawn(
                        p_center,
                        Vec2::new(rand::gen_range(-20.0, -5.0), rand::gen_range(-10.0, 10.0)),
                        Color::new(0.4, 0.8, 1.0, 0.7),
                        5.0,
                        0.3,
                    );
                }

                let prev_score = gs.score;
                let prev_w = gs.player.rect.w;

                // Simulation tick
                gs.update(self.sim_time_ms);

                // Detect collision events for visual FX
                if gs.score > prev_score {
                    let p_center = Vec2::new(
                        (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                        (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                    );
                    if gs.player.rect.w > prev_w {
                        // Grow
                        self.particle_system.burst(
                            p_center,
                            Color::new(0.3, 0.9, 0.4, 1.0),
                            16,
                            120.0,
                            4.0,
                        );
                    } else if gs.player.rect.w < prev_w {
                        // Shrink
                        self.particle_system.burst(
                            p_center,
                            Color::new(1.0, 0.7, 0.2, 1.0),
                            16,
                            120.0,
                            4.0,
                        );
                    } else {
                        // Checkpoint pass
                        self.particle_system.burst(
                            p_center,
                            Color::new(0.4, 0.8, 1.0, 1.0),
                            24,
                            160.0,
                            5.0,
                        );
                        self.screen_shake = 4.0;
                    }
                } else if gs.score < prev_score || (!gs.running && !gs.victory) {
                    // Damage
                    let p_center = Vec2::new(
                        (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                        (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                    );
                    self.particle_system.burst(
                        p_center,
                        Color::new(1.0, 0.2, 0.2, 1.0),
                        25,
                        180.0,
                        5.0,
                    );
                    self.screen_shake = 8.0;
                }

                if !gs.running {
                    self.final_score = gs.score;
                    self.game_over_message = if gs.victory {
                        self.config.victory_text.clone()
                    } else {
                        self.config.game_over_text.clone()
                    };
                    self.entering_name = self.final_score > 0;
                    self.player_name_input.clear();
                    self.held_keys_on_death = macroquad::input::get_keys_down();
                    clear_input_queue();
                    self.app_status = AppStatus::Restarting;
                    break;
                }
            }

            self.particle_system.update(dt);
        }

        render_game_scene(
            viewport,
            shake_offset,
            gs,
            &self.config,
            &self.particle_system,
            self.sim_time_ms,
        );

        true
    }

    pub fn handle_game_over(&mut self, viewport: Viewport) -> bool {
        if self.entering_name {
            if !self.held_keys_on_death.is_empty() {
                self.held_keys_on_death.retain(|&k| is_key_down(k));
                clear_input_queue();
            } else {
                while let Some(c) = get_char_pressed() {
                    if self.player_name_input.len() < 10
                        && (c.is_alphanumeric() || c == ' ' || c == '_')
                    {
                        self.player_name_input.push(c);
                    }
                }
                if is_key_pressed(KeyCode::Backspace) {
                    self.player_name_input.pop();
                }
                if is_key_pressed(KeyCode::Enter) {
                    if !self.player_name_input.is_empty() {
                        self.scoreboard_manager
                            .add_score(&self.player_name_input, self.final_score);
                    }
                    self.entering_name = false;
                    self.app_status = AppStatus::ShowingMainMenu;
                }
                if is_key_pressed(KeyCode::Escape) {
                    self.entering_name = false;
                }
            }
        } else if let Some(key) = get_last_key_pressed() {
            if let Some(action) = handle_game_over_key(key) {
                match action {
                    GameOverAction::Restart => {
                        self.start_new_game();
                        return true;
                    }
                    GameOverAction::MainMenu => {
                        self.app_status = AppStatus::ShowingMainMenu;
                    }
                    GameOverAction::Quit => {
                        self.app_status = AppStatus::Quitting;
                        return false;
                    }
                }
            }
        }

        render_game_over_view(
            &self.config,
            &self.game_over_message,
            self.final_score,
            &self.player_name_input,
            self.entering_name,
            viewport,
        );

        true
    }

    pub async fn step(&mut self) -> bool {
        let frame_start = Instant::now();
        let dt = get_frame_time().min(0.05);

        let virtual_w = self.config.screen_width as f32;
        let virtual_h = self.config.screen_height as f32;
        let viewport = Viewport::calculate(virtual_w, virtual_h);

        clear_background(Color::new(0.08, 0.08, 0.12, 1.0));

        if self.screen_shake > 0.0 {
            self.screen_shake = (self.screen_shake - dt * 25.0).max(0.0);
        }
        let shake_offset = if self.screen_shake > 0.0 {
            Vec2::new(
                rand::gen_range(-self.screen_shake, self.screen_shake),
                rand::gen_range(-self.screen_shake, self.screen_shake),
            )
        } else {
            Vec2::ZERO
        };

        match self.app_status {
            AppStatus::ShowingMainMenu => {
                self.handle_main_menu();
                render_main_menu_screen(&self.config, viewport);
            }
            AppStatus::ShowingScoreboard => {
                self.handle_scoreboard();
                render_scoreboard_screen(&self.config, &self.scoreboard_manager, viewport);
            }
            AppStatus::ShowingSettingsMenu => {
                self.handle_settings();
                render_settings_screen(
                    &self.config,
                    self.in_color_picker,
                    self.color_selection,
                    viewport,
                );
            }
            AppStatus::Running => {
                if !self.handle_running(dt, viewport, shake_offset) {
                    return false;
                }
            }
            AppStatus::Restarting => {
                if !self.handle_game_over(viewport) {
                    return false;
                }
            }
            AppStatus::Quitting => {
                return false;
            }
        }

        if self.app_status == AppStatus::Quitting {
            return false;
        }

        next_frame().await;
        let elapsed = frame_start.elapsed();
        if elapsed < self.target_frame_duration {
            std::thread::sleep(self.target_frame_duration - elapsed);
        }

        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_app_initial_state() {
        let app = App::new();
        assert_eq!(app.app_status, AppStatus::ShowingMainMenu);
        assert!(app.game_state.is_none());
        assert!(!app.entering_name);
        assert_eq!(app.final_score, 0);
        assert!(!app.in_color_picker);
        assert_eq!(app.color_selection, 0);
    }

    #[test]
    fn test_app_start_new_game() {
        let mut app = App::new();
        app.start_new_game();
        assert_eq!(app.app_status, AppStatus::Running);
        assert!(app.game_state.is_some());
        let gs = app.game_state.as_ref().unwrap();
        assert_eq!(gs.score, 0);
        assert_eq!(gs.level, 1);
        assert!(gs.running);
        assert!(!gs.victory);
        assert!(!gs.paused);
        assert_eq!(gs.player.rect.x, app.config.player_initial_x);
        assert_eq!(
            gs.player.rect.y,
            app.config.screen_height / 2 - app.config.player_height / 2
        );
    }

    #[test]
    fn test_app_restart_game_cleans_previous_state() {
        let mut app = App::new();
        app.start_new_game();

        // Mutate game state to simulate active gameplay
        {
            let gs = app.game_state.as_mut().unwrap();
            gs.score = 750;
            gs.level = 4;
            gs.player.grow(30);
            gs.running = false;
        }
        app.sim_time_ms = 15000;
        app.time_accumulator = 0.15;
        app.held_keys_on_death.insert(KeyCode::W);

        // Restart
        app.start_new_game();

        assert_eq!(app.app_status, AppStatus::Running);
        assert_eq!(app.sim_time_ms, 0);
        assert_eq!(app.time_accumulator, 0.0);
        assert!(app.held_keys_on_death.is_empty());

        let gs = app.game_state.as_ref().unwrap();
        assert_eq!(gs.score, 0);
        assert_eq!(gs.level, 1);
        assert!(gs.running);
        assert_eq!(gs.player.rect.w, app.config.player_width);
        assert_eq!(gs.player.rect.h, app.config.player_height);
    }

    #[test]
    fn test_app_fixed_timestep_properties() {
        let app = App::new();
        let expected_fps = app.config.target_fps.max(30) as f32;
        let expected_dt = 1.0 / expected_fps;
        assert!((app.fixed_dt - expected_dt).abs() < 0.0001);
        assert_eq!(app.fixed_dt_ms, (expected_dt * 1000.0) as u32);
        assert_eq!(
            app.target_frame_duration.as_micros(),
            (1_000_000 / (app.config.target_fps.max(30) as u128))
        );
    }
}
