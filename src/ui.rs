use crate::config::Config;
use crate::scoreboard::ScoreboardManager;
use crate::types::{GameOverAction, MainMenuAction, PauseMenuAction, SettingsMenuAction};
use macroquad::prelude::*;

pub fn get_level_text(
    level: i32,
    checkpoints_passed: i32,
    checkpoints_per_level: i32,
    config: &Config,
) -> String {
    if checkpoints_per_level <= 0 {
        return format!("{}{}", config.level_prefix, level);
    }
    let checkpoints_in_level = checkpoints_passed % checkpoints_per_level;
    let checkpoints_to_next = checkpoints_per_level - checkpoints_in_level;
    format!(
        "{}{}{}{}{}",
        config.level_prefix,
        level,
        config.level_progress_prefix,
        checkpoints_to_next,
        config.level_progress_suffix
    )
}

pub fn get_player_size_text(player_size: i32, gap_size: i32, config: &Config) -> String {
    if gap_size <= 0 {
        return format!("{}N/A", config.player_size_prefix);
    }
    let percentage = ((player_size as f64 / gap_size as f64) * 100.0) as i32;
    format!(
        "{}{}{}",
        config.player_size_prefix, percentage, config.player_size_suffix
    )
}

pub fn get_dash_status_text(on_cooldown: bool, cooldown_remaining: u32, config: &Config) -> String {
    if on_cooldown && cooldown_remaining > 0 {
        let seconds = cooldown_remaining as f32 / 1000.0;
        let seconds_int = (seconds * 10.0).round() as i32;
        let time_str = format!("{}.{}", seconds_int / 10, seconds_int % 10);
        format!(
            "{}{}{}",
            config.dash_cooldown_prefix, time_str, config.dash_cooldown_suffix
        )
    } else {
        config.dash_ready_text.clone()
    }
}

pub fn calculate_cooldown_circle_points(
    progress: f32,
    x: i32,
    y: i32,
    radius: i32,
) -> Vec<(i32, i32)> {
    let segments = 30;
    let num_points = (segments as f32 * progress) as i32 + 1;
    let mut points = Vec::with_capacity(num_points as usize);

    for i in 0..num_points {
        let angle =
            -std::f32::consts::FRAC_PI_2 + (i as f32 / segments as f32) * std::f32::consts::TAU;
        let px = x + (radius as f32 * angle.cos()) as i32;
        let py = y + (radius as f32 * angle.sin()) as i32;
        points.push((px, py));
    }
    points
}

pub fn handle_main_menu_key(key: KeyCode) -> Option<MainMenuAction> {
    match key {
        KeyCode::S => Some(MainMenuAction::StartGame),
        KeyCode::C => Some(MainMenuAction::ShowScoreboard),
        KeyCode::E => Some(MainMenuAction::Settings),
        KeyCode::Q | KeyCode::Escape => Some(MainMenuAction::Quit),
        _ => None,
    }
}

pub fn handle_pause_menu_key(key: KeyCode) -> Option<PauseMenuAction> {
    match key {
        KeyCode::Escape => Some(PauseMenuAction::Resume),
        KeyCode::R => Some(PauseMenuAction::Restart),
        KeyCode::M => Some(PauseMenuAction::MainMenu),
        KeyCode::Q => Some(PauseMenuAction::Quit),
        _ => None,
    }
}

pub fn handle_game_over_key(key: KeyCode) -> Option<GameOverAction> {
    match key {
        KeyCode::R => Some(GameOverAction::Restart),
        KeyCode::M => Some(GameOverAction::MainMenu),
        KeyCode::Q | KeyCode::Escape => Some(GameOverAction::Quit),
        _ => None,
    }
}

pub fn handle_settings_menu_key(
    key: KeyCode,
    in_color_picker: &mut bool,
    color_selection: &mut usize,
    choices_len: usize,
) -> Option<SettingsMenuAction> {
    if *in_color_picker {
        match key {
            KeyCode::Left => {
                if *color_selection > 0 {
                    *color_selection -= 1;
                }
                None
            }
            KeyCode::Right => {
                if *color_selection + 1 < choices_len {
                    *color_selection += 1;
                }
                None
            }
            KeyCode::Enter => {
                *in_color_picker = false;
                Some(SettingsMenuAction::ChangePlayerColor)
            }
            KeyCode::Escape => {
                *in_color_picker = false;
                None
            }
            _ => None,
        }
    } else {
        match key {
            KeyCode::C => {
                *in_color_picker = true;
                None
            }
            KeyCode::T => Some(SettingsMenuAction::ToggleFullscreen),
            KeyCode::B | KeyCode::Escape => Some(SettingsMenuAction::Back),
            _ => None,
        }
    }
}

// ── Rendering Helpers ──────────────────────────────────────────────────────────

#[allow(clippy::too_many_arguments)]
pub fn render_hud(
    score: i32,
    level: i32,
    ui_next_checkpoint_gap_size: i32,
    checkpoints_passed_in_level: i32,
    checkpoints_per_level: i32,
    player_width: i32,
    on_cooldown: bool,
    cooldown_remaining: u32,
    config: &Config,
    scale: f32,
    offset: Vec2,
) {
    let font_size = 20.0 * scale;
    let text_col = config.ui_text_color.to_macroquad();

    let score_str = format!("{}{}", config.score_prefix, score);
    let level_str = get_level_text(
        level,
        checkpoints_passed_in_level,
        checkpoints_per_level,
        config,
    );
    let gap_str = format!("{}{}", config.gap_size_prefix, ui_next_checkpoint_gap_size);
    let size_str = get_player_size_text(player_width, ui_next_checkpoint_gap_size, config);

    let start_x = 16.0 * scale + offset.x;
    let mut cur_y = 28.0 * scale + offset.y;
    let line_gap = 22.0 * scale;

    draw_text(&score_str, start_x, cur_y, font_size, text_col);
    cur_y += line_gap;
    draw_text(&level_str, start_x, cur_y, font_size, text_col);
    cur_y += line_gap;
    draw_text(&gap_str, start_x, cur_y, font_size, text_col);
    cur_y += line_gap;
    draw_text(&size_str, start_x, cur_y, font_size, text_col);

    // Dash status in bottom left
    let dash_text = get_dash_status_text(on_cooldown, cooldown_remaining, config);
    let bottom_y = (config.screen_height as f32 - 16.0) * scale + offset.y;
    let mut dash_text_x = start_x;

    if on_cooldown && cooldown_remaining > 0 {
        let progress =
            1.0 - (cooldown_remaining as f32 / config.dash_cooldown_ms as f32).clamp(0.0, 1.0);
        let rad = config.cooldown_indicator_radius as f32 * scale;
        let circle_center = Vec2::new(start_x + rad, bottom_y - rad * 0.5);

        // Draw background track
        draw_poly_lines(
            circle_center.x,
            circle_center.y,
            24,
            rad,
            0.0,
            2.0 * scale,
            Color::new(0.3, 0.3, 0.3, 0.5),
        );

        // Draw progress arc
        let points = calculate_cooldown_circle_points(
            progress,
            circle_center.x as i32,
            circle_center.y as i32,
            rad as i32,
        );
        for pair in points.windows(2) {
            draw_line(
                pair[0].0 as f32,
                pair[0].1 as f32,
                pair[1].0 as f32,
                pair[1].1 as f32,
                2.5 * scale,
                config.cooldown_indicator_color.to_macroquad(),
            );
        }

        dash_text_x += rad * 2.0 + 10.0 * scale;
    }

    draw_text(&dash_text, dash_text_x, bottom_y, font_size, text_col);
}

pub fn render_main_menu(config: &Config, scale: f32, offset: Vec2) {
    let sw = config.screen_width as f32 * scale;
    let sh = config.screen_height as f32 * scale;
    let cx = sw * 0.5 + offset.x;
    let cy = sh * 0.5 + offset.y;

    let title_size = 48.0 * scale;
    let text_col = config.ui_text_color.to_macroquad();

    let title_dim = measure_text(&config.main_menu_title, None, title_size as u16, 1.0);
    draw_text(
        &config.main_menu_title,
        cx - title_dim.width * 0.5,
        cy - 50.0 * scale,
        title_size,
        text_col,
    );

    let inst_size = 20.0 * scale;
    let inst_dim = measure_text(&config.main_menu_instructions, None, inst_size as u16, 1.0);
    draw_text(
        &config.main_menu_instructions,
        cx - inst_dim.width * 0.5,
        cy + 25.0 * scale,
        inst_size,
        Color::new(0.8, 0.8, 0.9, 1.0),
    );
}

pub fn render_pause_menu(config: &Config, scale: f32, offset: Vec2) {
    let sw = config.screen_width as f32 * scale;
    let sh = config.screen_height as f32 * scale;
    let cx = sw * 0.5 + offset.x;
    let cy = sh * 0.5 + offset.y;

    draw_rectangle(offset.x, offset.y, sw, sh, Color::new(0.0, 0.0, 0.0, 0.65));

    let title_size = 42.0 * scale;
    let title_dim = measure_text(&config.pause_menu_title, None, title_size as u16, 1.0);
    draw_text(
        &config.pause_menu_title,
        cx - title_dim.width * 0.5,
        cy - 40.0 * scale,
        title_size,
        config.ui_text_color.to_macroquad(),
    );

    let inst_size = 20.0 * scale;
    let inst_dim = measure_text(&config.pause_menu_instructions, None, inst_size as u16, 1.0);
    draw_text(
        &config.pause_menu_instructions,
        cx - inst_dim.width * 0.5,
        cy + 30.0 * scale,
        inst_size,
        Color::new(0.8, 0.8, 0.9, 1.0),
    );
}

pub fn render_settings_menu(
    config: &Config,
    in_color_picker: bool,
    color_selection: usize,
    scale: f32,
    offset: Vec2,
) {
    let sw = config.screen_width as f32 * scale;
    let sh = config.screen_height as f32 * scale;
    let cx = sw * 0.5 + offset.x;
    let cy = sh * 0.5 + offset.y;
    let text_col = config.ui_text_color.to_macroquad();

    if in_color_picker {
        let title = "Choose Color";
        let title_size = 40.0 * scale;
        let title_dim = measure_text(title, None, title_size as u16, 1.0);
        draw_text(
            title,
            cx - title_dim.width * 0.5,
            cy - 80.0 * scale,
            title_size,
            text_col,
        );

        let box_size = 44.0 * scale;
        let pad = 12.0 * scale;
        let total_w = config.player_color_choices.len() as f32 * (box_size + pad) - pad;
        let mut start_x = cx - total_w * 0.5;

        for (i, c) in config.player_color_choices.iter().enumerate() {
            let rect = Rect::new(start_x, cy - 20.0 * scale, box_size, box_size);
            draw_rectangle(rect.x, rect.y, rect.w, rect.h, c.to_macroquad());
            if i == color_selection {
                draw_rectangle_lines(
                    rect.x - 3.0 * scale,
                    rect.y - 3.0 * scale,
                    rect.w + 6.0 * scale,
                    rect.h + 6.0 * scale,
                    3.0 * scale,
                    WHITE,
                );
            }
            start_x += box_size + pad;
        }

        let hint = "Left/Right to select, Enter to confirm, ESC to cancel";
        let hint_size = 18.0 * scale;
        let hint_dim = measure_text(hint, None, hint_size as u16, 1.0);
        draw_text(
            hint,
            cx - hint_dim.width * 0.5,
            cy + 60.0 * scale,
            hint_size,
            Color::new(0.8, 0.8, 0.9, 1.0),
        );
    } else {
        let title_size = 40.0 * scale;
        let title_dim = measure_text(&config.settings_menu_title, None, title_size as u16, 1.0);
        draw_text(
            &config.settings_menu_title,
            cx - title_dim.width * 0.5,
            cy - 40.0 * scale,
            title_size,
            text_col,
        );

        let inst_size = 20.0 * scale;
        let inst_dim = measure_text(
            &config.settings_menu_instructions,
            None,
            inst_size as u16,
            1.0,
        );
        draw_text(
            &config.settings_menu_instructions,
            cx - inst_dim.width * 0.5,
            cy + 25.0 * scale,
            inst_size,
            Color::new(0.8, 0.8, 0.9, 1.0),
        );
    }
}

pub fn render_scoreboard(
    config: &Config,
    scoreboard: &ScoreboardManager,
    scale: f32,
    offset: Vec2,
) {
    let sw = config.screen_width as f32 * scale;
    let sh = config.screen_height as f32 * scale;
    let cx = sw * 0.5 + offset.x;
    let text_col = config.ui_text_color.to_macroquad();

    let title_size = 40.0 * scale;
    let title_dim = measure_text(&config.scoreboard_title, None, title_size as u16, 1.0);
    draw_text(
        &config.scoreboard_title,
        cx - title_dim.width * 0.5,
        offset.y + 50.0 * scale,
        title_size,
        text_col,
    );

    let mut y_pos = offset.y + 110.0 * scale;
    let col_size = 18.0 * scale;

    draw_text(
        "Rank",
        cx - 240.0 * scale,
        y_pos,
        col_size,
        Color::new(0.6, 0.8, 1.0, 1.0),
    );
    draw_text(
        "Name",
        cx - 140.0 * scale,
        y_pos,
        col_size,
        Color::new(0.6, 0.8, 1.0, 1.0),
    );
    draw_text(
        "Score",
        cx + 70.0 * scale,
        y_pos,
        col_size,
        Color::new(0.6, 0.8, 1.0, 1.0),
    );
    draw_text(
        "Date",
        cx + 180.0 * scale,
        y_pos,
        col_size,
        Color::new(0.6, 0.8, 1.0, 1.0),
    );
    y_pos += 26.0 * scale;

    for (i, entry) in scoreboard.get_scores().iter().enumerate() {
        let rank_str = format!("{}.", i + 1);
        let score_str = entry.score.to_string();
        draw_text(&rank_str, cx - 240.0 * scale, y_pos, col_size, text_col);
        draw_text(&entry.name, cx - 140.0 * scale, y_pos, col_size, text_col);
        draw_text(&score_str, cx + 70.0 * scale, y_pos, col_size, text_col);
        draw_text(&entry.date, cx + 180.0 * scale, y_pos, col_size, text_col);
        y_pos += 24.0 * scale;
    }

    let inst_size = 18.0 * scale;
    let inst_dim = measure_text(&config.scoreboard_instructions, None, inst_size as u16, 1.0);
    draw_text(
        &config.scoreboard_instructions,
        cx - inst_dim.width * 0.5,
        offset.y + sh - 30.0 * scale,
        inst_size,
        Color::new(0.7, 0.7, 0.8, 1.0),
    );
}

pub fn render_game_over_screen(
    config: &Config,
    message: &str,
    final_score: i32,
    player_name: &str,
    entering_name: bool,
    scale: f32,
    offset: Vec2,
) {
    let sw = config.screen_width as f32 * scale;
    let sh = config.screen_height as f32 * scale;
    let cx = sw * 0.5 + offset.x;
    let cy = sh * 0.5 + offset.y;

    draw_rectangle(offset.x, offset.y, sw, sh, Color::new(0.0, 0.0, 0.0, 0.75));

    if entering_name {
        let prompt_size = 32.0 * scale;
        let prompt_dim = measure_text(&config.enter_name_prompt, None, prompt_size as u16, 1.0);
        draw_text(
            &config.enter_name_prompt,
            cx - prompt_dim.width * 0.5,
            cy - 80.0 * scale,
            prompt_size,
            config.ui_text_color.to_macroquad(),
        );

        let score_str = format!("{}{}", config.final_score_text, final_score);
        let score_dim = measure_text(&score_str, None, (24.0 * scale) as u16, 1.0);
        draw_text(
            &score_str,
            cx - score_dim.width * 0.5,
            cy - 40.0 * scale,
            24.0 * scale,
            Color::new(0.4, 0.9, 0.5, 1.0),
        );

        let display_name = format!("{}_", player_name);
        let name_dim = measure_text(&display_name, None, (30.0 * scale) as u16, 1.0);
        draw_text(
            &display_name,
            cx - name_dim.width * 0.5,
            cy + 10.0 * scale,
            30.0 * scale,
            WHITE,
        );

        let hint = "Press Enter to save, ESC to skip";
        let hint_dim = measure_text(hint, None, (18.0 * scale) as u16, 1.0);
        draw_text(
            hint,
            cx - hint_dim.width * 0.5,
            cy + 50.0 * scale,
            18.0 * scale,
            Color::new(0.7, 0.7, 0.8, 1.0),
        );
    } else {
        let msg_size = 40.0 * scale;
        let msg_dim = measure_text(message, None, msg_size as u16, 1.0);
        draw_text(
            message,
            cx - msg_dim.width * 0.5,
            cy - 40.0 * scale,
            msg_size,
            config.ui_text_color.to_macroquad(),
        );

        let inst_size = 20.0 * scale;
        let inst_dim = measure_text(&config.game_over_instructions, None, inst_size as u16, 1.0);
        draw_text(
            &config.game_over_instructions,
            cx - inst_dim.width * 0.5,
            cy + 25.0 * scale,
            inst_size,
            Color::new(0.8, 0.8, 0.9, 1.0),
        );
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ui_level_text_formatter() {
        let config = Config::load("").unwrap_or_default();
        assert_eq!(
            get_level_text(1, 2, 5, &config),
            "Level: 1 (3 checkpoints to next level)"
        );
    }

    #[test]
    fn test_ui_player_size_formatter() {
        let config = Config::load("").unwrap_or_default();
        assert_eq!(
            get_player_size_text(40, 200, &config),
            "Player Size: 20% of gap size"
        );
    }

    #[test]
    fn test_ui_dash_status_formatter() {
        let config = Config::load("").unwrap_or_default();
        assert_eq!(get_dash_status_text(false, 0, &config), "dash -> ready");
        assert_eq!(
            get_dash_status_text(true, 1500, &config),
            "dash -> cooldown (1.5s)"
        );
    }

    #[test]
    fn test_cooldown_circle_points_count() {
        let pts = calculate_cooldown_circle_points(1.0, 50, 50, 20);
        assert_eq!(pts.len(), 31);
    }
}
