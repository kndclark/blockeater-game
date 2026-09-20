use blockeater_game::config::Config;
use blockeater_game::game::GameState;
use blockeater_game::particles::ParticleSystem;
use blockeater_game::scoreboard::ScoreboardManager;
use blockeater_game::types::{
    AppStatus, GameOverAction, MainMenuAction, PauseMenuAction, SettingsMenuAction,
};
use blockeater_game::ui::*;
use macroquad::prelude::*;

fn window_conf() -> Conf {
    Conf {
        window_title: "THE BLOCKEATER".to_string(),
        window_width: 960,
        window_height: 720,
        high_dpi: true,
        ..Default::default()
    }
}

#[macroquad::main(window_conf)]
async fn main() {
    // Check for headless smoke test flag
    let args: Vec<String> = std::env::args().collect();
    if args.iter().any(|arg| arg == "--headless-smoke-test") {
        println!("Running headless smoke test...");
        let config = Config::load("").unwrap_or_default();
        let mut game_state = GameState::new(config, 640, 480);
        for t in 0..100 {
            game_state.update(t * 16);
        }
        println!("Headless smoke test completed successfully!");
        return;
    }

    let mut config = Config::load("").unwrap_or_default();
    let mut scoreboard_manager = ScoreboardManager::new("data/scores.json");
    let mut particle_system = ParticleSystem::new();

    let mut app_status = AppStatus::ShowingMainMenu;
    let mut game_state: Option<GameState> = None;

    let mut in_color_picker = false;
    let mut color_selection = 0;

    let mut player_name_input = String::new();
    let mut entering_name = false;
    let mut final_score = 0;
    let mut game_over_message = String::new();

    let mut screen_shake = 0.0f32;

    let bg_color = Color::new(0.08, 0.08, 0.12, 1.0);
    let play_area_bg = Color::new(0.12, 0.12, 0.16, 1.0);

    loop {
        let dt = get_frame_time().min(0.05);
        let current_time_ms = (get_time() * 1000.0) as u32;

        let virtual_w = config.screen_width as f32;
        let virtual_h = config.screen_height as f32;

        let scale = (screen_width() / virtual_w).min(screen_height() / virtual_h);
        let offset = Vec2::new(
            (screen_width() - virtual_w * scale) * 0.5,
            (screen_height() - virtual_h * scale) * 0.5,
        );

        clear_background(bg_color);

        // Screen shake decay
        if screen_shake > 0.0 {
            screen_shake = (screen_shake - dt * 25.0).max(0.0);
        }
        let shake_offset = if screen_shake > 0.0 {
            Vec2::new(
                rand::gen_range(-screen_shake, screen_shake),
                rand::gen_range(-screen_shake, screen_shake),
            )
        } else {
            Vec2::ZERO
        };

        match app_status {
            AppStatus::ShowingMainMenu => {
                if let Some(key) = get_last_key_pressed() {
                    if let Some(action) = handle_main_menu_key(key) {
                        match action {
                            MainMenuAction::StartGame => {
                                game_state = Some(GameState::new(
                                    config.clone(),
                                    config.screen_width,
                                    config.screen_height,
                                ));
                                particle_system.clear();
                                app_status = AppStatus::Running;
                            }
                            MainMenuAction::ShowScoreboard => {
                                app_status = AppStatus::ShowingScoreboard;
                            }
                            MainMenuAction::Settings => {
                                in_color_picker = false;
                                app_status = AppStatus::ShowingSettingsMenu;
                            }
                            MainMenuAction::Quit => {
                                break;
                            }
                        }
                    }
                }
                render_main_menu(&config, scale, offset);
            }

            AppStatus::ShowingScoreboard => {
                if is_key_pressed(KeyCode::B) || is_key_pressed(KeyCode::Escape) {
                    app_status = AppStatus::ShowingMainMenu;
                }
                render_scoreboard(&config, &scoreboard_manager, scale, offset);
            }

            AppStatus::ShowingSettingsMenu => {
                if let Some(key) = get_last_key_pressed() {
                    let choices_len = config.player_color_choices.len();
                    if let Some(action) = handle_settings_menu_key(
                        key,
                        &mut in_color_picker,
                        &mut color_selection,
                        choices_len,
                    ) {
                        match action {
                            SettingsMenuAction::Back => {
                                app_status = AppStatus::ShowingMainMenu;
                            }
                            SettingsMenuAction::ToggleFullscreen => {
                                // Macroquad doesn't have a direct toggle API, handled automatically by OS/window manager
                            }
                            SettingsMenuAction::ChangePlayerColor => {
                                if color_selection < config.player_color_choices.len() {
                                    config.player_color =
                                        config.player_color_choices[color_selection];
                                }
                            }
                        }
                    }
                }
                render_settings_menu(&config, in_color_picker, color_selection, scale, offset);
            }

            AppStatus::Running => {
                if let Some(gs) = game_state.as_mut() {
                    if is_key_pressed(KeyCode::Escape) {
                        gs.paused = !gs.paused;
                    }

                    if gs.paused {
                        if let Some(key) = get_last_key_pressed() {
                            if let Some(action) = handle_pause_menu_key(key) {
                                match action {
                                    PauseMenuAction::Resume => gs.paused = false,
                                    PauseMenuAction::Restart => {
                                        *gs = GameState::new(
                                            config.clone(),
                                            config.screen_width,
                                            config.screen_height,
                                        );
                                        particle_system.clear();
                                    }
                                    PauseMenuAction::MainMenu => {
                                        app_status = AppStatus::ShowingMainMenu;
                                    }
                                    PauseMenuAction::Quit => break,
                                }
                            }
                        }
                    } else {
                        // Input handling
                        let left = is_key_down(KeyCode::Left) || is_key_down(KeyCode::A);
                        let right = is_key_down(KeyCode::Right) || is_key_down(KeyCode::D);
                        let up = is_key_down(KeyCode::Up) || is_key_down(KeyCode::W);
                        let down = is_key_down(KeyCode::Down) || is_key_down(KeyCode::S);
                        let shift =
                            is_key_down(KeyCode::LeftShift) || is_key_down(KeyCode::RightShift);

                        gs.player.handle_input(
                            left,
                            right,
                            up,
                            down,
                            shift,
                            config.screen_width,
                            config.screen_height,
                            current_time_ms,
                        );

                        // Dash particle trail
                        if gs.player.is_dashing {
                            let p_center = Vec2::new(
                                (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                                (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                            );
                            particle_system.spawn(
                                p_center,
                                Vec2::new(
                                    rand::gen_range(-20.0, -5.0),
                                    rand::gen_range(-10.0, 10.0),
                                ),
                                Color::new(0.4, 0.8, 1.0, 0.7),
                                5.0,
                                0.3,
                            );
                        }

                        let prev_score = gs.score;
                        let prev_w = gs.player.rect.w;

                        // Simulation tick
                        gs.update(current_time_ms);

                        // Detect collision events for juicy particles and screen shake
                        if gs.score > prev_score {
                            let p_center = Vec2::new(
                                (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                                (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                            );
                            if gs.player.rect.w > prev_w {
                                // Grow
                                particle_system.burst(
                                    p_center,
                                    Color::new(0.3, 0.9, 0.4, 1.0),
                                    16,
                                    120.0,
                                    4.0,
                                );
                            } else if gs.player.rect.w < prev_w {
                                // Shrink
                                particle_system.burst(
                                    p_center,
                                    Color::new(1.0, 0.7, 0.2, 1.0),
                                    16,
                                    120.0,
                                    4.0,
                                );
                            } else {
                                // Checkpoint pass
                                particle_system.burst(
                                    p_center,
                                    Color::new(0.4, 0.8, 1.0, 1.0),
                                    24,
                                    160.0,
                                    5.0,
                                );
                                screen_shake = 4.0;
                            }
                        } else if gs.score < prev_score || (!gs.running && !gs.victory) {
                            // Damage
                            let p_center = Vec2::new(
                                (gs.player.rect.x + gs.player.rect.w / 2) as f32,
                                (gs.player.rect.y + gs.player.rect.h / 2) as f32,
                            );
                            particle_system.burst(
                                p_center,
                                Color::new(1.0, 0.2, 0.2, 1.0),
                                25,
                                180.0,
                                5.0,
                            );
                            screen_shake = 8.0;
                        }

                        particle_system.update(dt);

                        if !gs.running {
                            final_score = gs.score;
                            game_over_message = if gs.victory {
                                config.victory_text.clone()
                            } else {
                                config.game_over_text.clone()
                            };
                            entering_name = final_score > 0;
                            player_name_input.clear();
                            app_status = AppStatus::Quitting; // Will be handled below
                        }
                    }

                    // Render Play Area
                    let play_x = offset.x + shake_offset.x;
                    let play_y = offset.y + shake_offset.y;
                    draw_rectangle(
                        play_x,
                        play_y,
                        virtual_w * scale,
                        virtual_h * scale,
                        play_area_bg,
                    );
                    draw_rectangle_lines(
                        play_x,
                        play_y,
                        virtual_w * scale,
                        virtual_h * scale,
                        2.0 * scale,
                        Color::new(0.2, 0.2, 0.3, 1.0),
                    );

                    // Draw Obstacles
                    for obs in &gs.obstacles {
                        let col = config.get_obstacle_color(obs.obstacle_type).to_macroquad();
                        let rx = obs.rect.x as f32 * scale + play_x;
                        let ry = obs.rect.y as f32 * scale + play_y;
                        let rw = obs.rect.w as f32 * scale;
                        let rh = obs.rect.h as f32 * scale;

                        draw_rectangle(rx, ry, rw, rh, col);
                        draw_rectangle_lines(
                            rx,
                            ry,
                            rw,
                            rh,
                            1.5 * scale,
                            Color::new(1.0, 1.0, 1.0, 0.3),
                        );

                        if let Some(r2) = obs.rect2 {
                            let r2x = r2.x as f32 * scale + play_x;
                            let r2y = r2.y as f32 * scale + play_y;
                            let r2w = r2.w as f32 * scale;
                            let r2h = r2.h as f32 * scale;

                            draw_rectangle(r2x, r2y, r2w, r2h, col);
                            draw_rectangle_lines(
                                r2x,
                                r2y,
                                r2w,
                                r2h,
                                1.5 * scale,
                                Color::new(1.0, 1.0, 1.0, 0.3),
                            );
                        }
                    }

                    // Draw Particles
                    particle_system.draw(scale, Vec2::new(play_x, play_y));

                    // Draw Player
                    let px = gs.player.rect.x as f32 * scale + play_x;
                    let py = gs.player.rect.y as f32 * scale + play_y;
                    let pw = gs.player.rect.w as f32 * scale;
                    let ph = gs.player.rect.h as f32 * scale;
                    let p_col = config.player_color.to_macroquad();

                    draw_rectangle(px, py, pw, ph, p_col);
                    draw_rectangle_lines(px, py, pw, ph, 2.0 * scale, WHITE);

                    // Render HUD
                    render_hud(
                        gs.score,
                        gs.level,
                        gs.ui_next_checkpoint_gap_size,
                        gs.checkpoints_passed_in_level,
                        gs.level_manager.effective_checkpoints_per_level,
                        gs.player.rect.w,
                        gs.player.on_cooldown,
                        gs.player.get_dash_cooldown_remaining(current_time_ms),
                        &config,
                        scale,
                        offset,
                    );

                    if gs.paused {
                        render_pause_menu(&config, scale, offset);
                    }
                }

                // If game ended, transition to game over display
                if app_status == AppStatus::Quitting
                    && game_state.as_ref().is_some_and(|gs| !gs.running)
                {
                    app_status = AppStatus::Restarting; // Temporary state indicating Game Over display
                }
            }

            AppStatus::Restarting => {
                // Game Over / Victory screen
                if entering_name {
                    if let Some(c) = get_char_pressed() {
                        if player_name_input.len() < 10
                            && (c.is_alphanumeric() || c == ' ' || c == '_')
                        {
                            player_name_input.push(c);
                        }
                    }
                    if is_key_pressed(KeyCode::Backspace) {
                        player_name_input.pop();
                    }
                    if is_key_pressed(KeyCode::Enter) {
                        if !player_name_input.is_empty() {
                            scoreboard_manager.add_score(&player_name_input, final_score);
                        }
                        entering_name = false;
                        app_status = AppStatus::ShowingMainMenu;
                    }
                    if is_key_pressed(KeyCode::Escape) {
                        entering_name = false;
                    }
                } else if let Some(key) = get_last_key_pressed() {
                    if let Some(action) = handle_game_over_key(key) {
                        match action {
                            GameOverAction::Restart => {
                                game_state = Some(GameState::new(
                                    config.clone(),
                                    config.screen_width,
                                    config.screen_height,
                                ));
                                particle_system.clear();
                                app_status = AppStatus::Running;
                            }
                            GameOverAction::MainMenu => {
                                app_status = AppStatus::ShowingMainMenu;
                            }
                            GameOverAction::Quit => {
                                break;
                            }
                        }
                    }
                }

                render_game_over_screen(
                    &config,
                    &game_over_message,
                    final_score,
                    &player_name_input,
                    entering_name,
                    scale,
                    offset,
                );
            }

            AppStatus::Quitting => {
                break;
            }
        }

        next_frame().await;
    }
}
