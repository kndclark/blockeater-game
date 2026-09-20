//! Rendering pipeline and letterboxed viewport management.

use crate::config::Config;
use crate::game::GameState;
use crate::particles::ParticleSystem;
use crate::player::Player;
use crate::scoreboard::ScoreboardManager;
use crate::ui::*;
use macroquad::prelude::*;

/// Computed viewport for letterboxed aspect-ratio scaling.
#[derive(Debug, Clone, Copy)]
pub struct Viewport {
    pub scale: f32,
    pub offset: Vec2,
    pub virtual_w: f32,
    pub virtual_h: f32,
}

impl Viewport {
    pub fn calculate_from_screen(
        screen_w: f32,
        screen_h: f32,
        virtual_w: f32,
        virtual_h: f32,
    ) -> Self {
        let scale = (screen_w / virtual_w).min(screen_h / virtual_h);
        let offset = Vec2::new(
            (screen_w - virtual_w * scale) * 0.5,
            (screen_h - virtual_h * scale) * 0.5,
        );
        Self {
            scale,
            offset,
            virtual_w,
            virtual_h,
        }
    }

    pub fn calculate(virtual_w: f32, virtual_h: f32) -> Self {
        Self::calculate_from_screen(screen_width(), screen_height(), virtual_w, virtual_h)
    }
}

/// Renders the active game world: play area, obstacles, particles, player, and HUD.
pub fn render_game_scene(
    viewport: Viewport,
    shake_offset: Vec2,
    game_state: &GameState,
    config: &Config,
    particle_system: &ParticleSystem,
    sim_time_ms: u32,
) {
    let play_area_bg = Color::new(0.12, 0.12, 0.16, 1.0);
    let scale = viewport.scale;

    let play_x = viewport.offset.x + shake_offset.x;
    let play_y = viewport.offset.y + shake_offset.y;

    // Draw play area background & border
    draw_rectangle(
        play_x,
        play_y,
        viewport.virtual_w * scale,
        viewport.virtual_h * scale,
        play_area_bg,
    );
    draw_rectangle_lines(
        play_x,
        play_y,
        viewport.virtual_w * scale,
        viewport.virtual_h * scale,
        2.0 * scale,
        Color::new(0.2, 0.2, 0.3, 1.0),
    );

    // Draw Obstacles
    for obs in &game_state.obstacles {
        let col = config.get_obstacle_color(obs.obstacle_type).to_macroquad();
        let rx = obs.rect.x as f32 * scale + play_x;
        let ry = obs.rect.y as f32 * scale + play_y;
        let rw = obs.rect.w as f32 * scale;
        let rh = obs.rect.h as f32 * scale;

        draw_rectangle(rx, ry, rw, rh, col);
        draw_rectangle_lines(rx, ry, rw, rh, 1.5 * scale, Color::new(1.0, 1.0, 1.0, 0.3));

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

    // Draw Player Ghosts
    if !game_state.player.ghosts.is_empty() {
        for ghost in &game_state.player.ghosts {
            let gx = ghost.rect.x as f32 * scale + play_x;
            let gy = ghost.rect.y as f32 * scale + play_y;
            let gw = ghost.rect.w as f32 * scale;
            let gh = ghost.rect.h as f32 * scale;
            let age_ratio = (sim_time_ms.saturating_sub(ghost.creation_time) as f32)
                / Player::GHOST_LIFETIME_MS as f32;
            let alpha = (Player::GHOST_INITIAL_ALPHA as f32 * (1.0 - age_ratio.min(1.0))) / 255.0;
            let mut ghost_col = config.player_color.to_macroquad();
            ghost_col.a = alpha;
            draw_rectangle(gx, gy, gw, gh, ghost_col);
        }
    }

    // Draw Player
    let px = game_state.player.rect.x as f32 * scale + play_x;
    let py = game_state.player.rect.y as f32 * scale + play_y;
    let pw = game_state.player.rect.w as f32 * scale;
    let ph = game_state.player.rect.h as f32 * scale;
    let p_col = config.player_color.to_macroquad();

    draw_rectangle(px, py, pw, ph, p_col);
    draw_rectangle_lines(px, py, pw, ph, 2.0 * scale, WHITE);

    // Render HUD
    let hud_data = game_state.get_scoreboard_render_data(sim_time_ms);
    render_hud(&hud_data, config, scale, viewport.offset);

    // Pause overlay if paused
    if game_state.paused {
        render_pause_menu(config, scale, viewport.offset);
    }
}

pub fn render_main_menu_screen(config: &Config, viewport: Viewport) {
    render_main_menu(config, viewport.scale, viewport.offset);
}

pub fn render_scoreboard_screen(
    config: &Config,
    scoreboard: &ScoreboardManager,
    viewport: Viewport,
) {
    render_scoreboard(config, scoreboard, viewport.scale, viewport.offset);
}

pub fn render_settings_screen(
    config: &Config,
    in_color_picker: bool,
    color_selection: usize,
    viewport: Viewport,
) {
    render_settings_menu(
        config,
        in_color_picker,
        color_selection,
        viewport.scale,
        viewport.offset,
    );
}

pub fn render_game_over_view(
    config: &Config,
    message: &str,
    final_score: i32,
    player_name: &str,
    entering_name: bool,
    viewport: Viewport,
) {
    render_game_over_screen(
        config,
        message,
        final_score,
        player_name,
        entering_name,
        viewport.scale,
        viewport.offset,
    );
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_viewport_exact_fit() {
        let vp = Viewport::calculate_from_screen(960.0, 720.0, 960.0, 720.0);
        assert!((vp.scale - 1.0).abs() < 0.001);
        assert!((vp.offset.x - 0.0).abs() < 0.001);
        assert!((vp.offset.y - 0.0).abs() < 0.001);
    }

    #[test]
    fn test_viewport_wide_screen_letterboxing() {
        // 1920x720 screen with 960x720 virtual: scale is 1.0, horizontal offset is (1920-960)/2 = 480
        let vp = Viewport::calculate_from_screen(1920.0, 720.0, 960.0, 720.0);
        assert!((vp.scale - 1.0).abs() < 0.001);
        assert!((vp.offset.x - 480.0).abs() < 0.001);
        assert!((vp.offset.y - 0.0).abs() < 0.001);
    }

    #[test]
    fn test_viewport_tall_screen_pillarboxing() {
        // 960x1440 screen with 960x720 virtual: scale is 1.0, vertical offset is (1440-720)/2 = 360
        let vp = Viewport::calculate_from_screen(960.0, 1440.0, 960.0, 720.0);
        assert!((vp.scale - 1.0).abs() < 0.001);
        assert!((vp.offset.x - 0.0).abs() < 0.001);
        assert!((vp.offset.y - 360.0).abs() < 0.001);
    }

    #[test]
    fn test_viewport_16_9_scaling() {
        // 1920x1080 screen with 960x720 virtual:
        // scale = min(1920/960=2.0, 1080/720=1.5) = 1.5
        // scaled_w = 960 * 1.5 = 1440
        // offset_x = (1920 - 1440) / 2 = 240
        // offset_y = 0
        let vp = Viewport::calculate_from_screen(1920.0, 1080.0, 960.0, 720.0);
        assert!((vp.scale - 1.5).abs() < 0.001);
        assert!((vp.offset.x - 240.0).abs() < 0.001);
        assert!((vp.offset.y - 0.0).abs() < 0.001);
    }

    #[test]
    fn test_viewport_downscaling() {
        // 480x360 screen with 960x720 virtual: scale = 0.5, offset = (0, 0)
        let vp = Viewport::calculate_from_screen(480.0, 360.0, 960.0, 720.0);
        assert!((vp.scale - 0.5).abs() < 0.001);
        assert!((vp.offset.x - 0.0).abs() < 0.001);
        assert!((vp.offset.y - 0.0).abs() < 0.001);
    }

    #[test]
    fn test_viewport_extreme_ultrawide() {
        // 3840x1080 screen with 960x720 virtual:
        // scale = 1.5
        // offset_x = (3840 - 1440) / 2 = 1200
        let vp = Viewport::calculate_from_screen(3840.0, 1080.0, 960.0, 720.0);
        assert!((vp.scale - 1.5).abs() < 0.001);
        assert!((vp.offset.x - 1200.0).abs() < 0.001);
        assert!((vp.offset.y - 0.0).abs() < 0.001);
    }
}
