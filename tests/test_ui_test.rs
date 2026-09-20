use blockeater_game::config::Config;
use blockeater_game::ui::*;
use std::path::Path;

#[test]
fn test_config_font_path_resolution() {
    let config = Config::load("").unwrap_or_default();
    assert!(config.font_path.contains("assets/font.ttf"));
    assert!(Path::new(&config.font_path).exists());
}

#[test]
fn test_calculates_gaps_to_next_level_correctly() {
    let config = Config::load("").unwrap_or_default();

    assert_eq!(
        get_level_text(1, 0, 5, &config),
        "Level: 1 (5 checkpoints to next level)"
    );
    assert_eq!(
        get_level_text(1, 1, 5, &config),
        "Level: 1 (4 checkpoints to next level)"
    );
    assert_eq!(
        get_level_text(1, 4, 5, &config),
        "Level: 1 (1 checkpoints to next level)"
    );
    assert_eq!(
        get_level_text(2, 5, 5, &config),
        "Level: 2 (5 checkpoints to next level)"
    );
    assert_eq!(
        get_level_text(7, 34, 8, &config),
        "Level: 7 (6 checkpoints to next level)"
    );
}

#[test]
fn test_calculates_player_size_text_correctly() {
    let config = Config::load("").unwrap_or_default();

    assert_eq!(
        get_player_size_text(40, 200, &config),
        "Player Size: 20% of gap size"
    );
    assert_eq!(
        get_player_size_text(60, 150, &config),
        "Player Size: 40% of gap size"
    );
    assert_eq!(
        get_player_size_text(150, 150, &config),
        "Player Size: 100% of gap size"
    );
    assert_eq!(get_player_size_text(50, 0, &config), "Player Size: N/A");
}

#[test]
fn test_scoreboard_calculates_dash_status_text_correctly() {
    let config = Config::load("").unwrap_or_default();

    assert_eq!(
        get_dash_status_text(false, 0, &config),
        config.dash_ready_text
    );
    assert_eq!(
        get_dash_status_text(true, 0, &config),
        config.dash_ready_text
    );
    assert_eq!(
        get_dash_status_text(true, 2000, &config),
        "dash -> cooldown (2.0s)"
    );
    assert_eq!(
        get_dash_status_text(true, 1540, &config),
        "dash -> cooldown (1.5s)"
    );
    assert_eq!(
        get_dash_status_text(true, 1550, &config),
        "dash -> cooldown (1.6s)"
    );
    assert_eq!(
        get_dash_status_text(true, 999, &config),
        "dash -> cooldown (1.0s)"
    );
}

#[test]
fn test_scoreboard_calculates_cooldown_circle_points() {
    // 0% progress -> 1 point
    let p0 = calculate_cooldown_circle_points(0.0, 0, 0, 10);
    assert_eq!(p0.len(), 1);

    // 50% progress -> 16 points (30 * 0.5 + 1)
    let p50 = calculate_cooldown_circle_points(0.5, 0, 0, 10);
    assert_eq!(p50.len(), 16);

    // 100% progress -> 31 points (30 + 1)
    let p100 = calculate_cooldown_circle_points(1.0, 0, 0, 10);
    assert_eq!(p100.len(), 31);
}
