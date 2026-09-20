use blockeater_game::config::Config;
use blockeater_game::types::{GameColor, ObstacleType, SizeBoostLevel};
use std::io::Write;
use tempfile::NamedTempFile;

#[test]
fn test_loads_game_config_from_file() {
    let config = Config::load("").expect("Should load default configs");

    // Check colors
    assert_eq!(config.player_color, GameColor::new(128, 0, 128, 255));
    assert_eq!(config.get_obstacle_color(ObstacleType::Hurt).r, 255);
    assert_eq!(config.get_obstacle_color(ObstacleType::Grow).g, 200);
    assert_eq!(config.get_obstacle_color(ObstacleType::Shrink).b, 0);

    // Check UI text color
    let ui_color = config.ui_text_color;
    assert_eq!(ui_color.r, 255);
    assert_eq!(ui_color.g, 255);
    assert_eq!(ui_color.b, 255);

    // Check game settings
    assert_eq!(config.base_checkpoint_gap, 200);
    assert_eq!(config.spawn_interval_ms, 1500);
    assert_eq!(config.checkpoint_interval_ms, 10000);
    assert_eq!(config.grow_chance_percent, 40);
    assert_eq!(config.shrink_chance_percent, 40);
    assert_eq!(config.hurt_chance_percent, 20);

    // Check obstacle dimensions
    assert_eq!(config.grow_dims.w, 50);
    assert_eq!(config.grow_dims.h, 50);
    assert_eq!(config.shrink_dims.w, 10);
    assert_eq!(config.shrink_dims.h, 10);
    assert_eq!(config.hurt_dims.w, 30);
    assert_eq!(config.hurt_dims.h, 30);

    // Check player settings
    assert_eq!(config.player_initial_x, 100);
    assert_eq!(config.player_width, 40);
    assert_eq!(config.player_height, 40);
    assert_eq!(config.player_speed, 5);
    assert_eq!(config.screen_width, 640);
    assert_eq!(config.screen_height, 480);
}

#[test]
fn test_throws_on_invalid_spawn_chances() {
    let mut temp_file = NamedTempFile::new().unwrap();
    let invalid_json = r#"{
        "game": { "obstacle_spawn_chances": { "grow": 10, "shrink": 10, "hurt": 10 } }
    }"#;
    temp_file.write_all(invalid_json.as_bytes()).unwrap();

    let res = Config::load_from_single_file(temp_file.path().to_str().unwrap());
    assert!(res.is_err());
    assert_eq!(
        res.unwrap_err(),
        "Obstacle spawn chances in config must sum to 100."
    );
}

#[test]
fn test_fallback_on_malformed_file() {
    let mut temp_file = NamedTempFile::new().unwrap();
    let malformed_json = r#"{ "player": { "r": 10, "#; // broken JSON
    temp_file.write_all(malformed_json.as_bytes()).unwrap();

    let config = Config::load_from_single_file(temp_file.path().to_str().unwrap())
        .expect("Should fallback on malformed json");

    assert_eq!(config.player_color, GameColor::new(100, 100, 100, 255));
    assert_eq!(config.screen_width, 640);
    assert_eq!(config.screen_height, 480);
}

#[test]
fn test_fallback_on_partially_missing_keys() {
    let mut temp_file = NamedTempFile::new().unwrap();
    let partial_json = r#"{
        "colors": {
            "player": { "r": 1, "g": 2, "b": 3, "a": 255 }
        },
        "game": {
            "base_checkpoint_gap": 99
        }
    }"#;
    temp_file.write_all(partial_json.as_bytes()).unwrap();

    let config = Config::load_from_single_file(temp_file.path().to_str().unwrap())
        .expect("Should load partial config");

    assert_eq!(config.player_color, GameColor::new(1, 2, 3, 255));
    assert_eq!(config.base_checkpoint_gap, 99);
    // Other values fall back to defaults
    assert_eq!(config.player_speed, 5);
    assert_eq!(config.screen_width, 640);
}

#[test]
fn test_loads_size_boost_ui_texts() {
    let config = Config::load("").expect("Should load default configs");
    assert_eq!(
        config.get_size_boost_text(SizeBoostLevel::Good),
        "Good size boost!"
    );
    assert_eq!(
        config.get_size_boost_text(SizeBoostLevel::Great),
        "Great size boost!"
    );
    assert_eq!(
        config.get_size_boost_text(SizeBoostLevel::Perfect),
        "Perfect size boost!"
    );
}
