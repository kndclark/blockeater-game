use blockeater_game::config::Config;
use blockeater_game::types::{GameOverAction, MainMenuAction, PauseMenuAction, SettingsMenuAction};
use blockeater_game::ui::*;
use macroquad::prelude::KeyCode;

#[test]
fn test_main_menu_transitions() {
    assert_eq!(
        handle_main_menu_key(KeyCode::S),
        Some(MainMenuAction::StartGame)
    );
    assert_eq!(
        handle_main_menu_key(KeyCode::C),
        Some(MainMenuAction::ShowScoreboard)
    );
    assert_eq!(
        handle_main_menu_key(KeyCode::E),
        Some(MainMenuAction::Settings)
    );
    assert_eq!(handle_main_menu_key(KeyCode::Q), Some(MainMenuAction::Quit));
    assert_eq!(
        handle_main_menu_key(KeyCode::Escape),
        Some(MainMenuAction::Quit)
    );
}

#[test]
fn test_pause_menu_transitions() {
    assert_eq!(
        handle_pause_menu_key(KeyCode::Escape),
        Some(PauseMenuAction::Resume)
    );
    assert_eq!(
        handle_pause_menu_key(KeyCode::R),
        Some(PauseMenuAction::Restart)
    );
    assert_eq!(
        handle_pause_menu_key(KeyCode::M),
        Some(PauseMenuAction::MainMenu)
    );
    assert_eq!(
        handle_pause_menu_key(KeyCode::Q),
        Some(PauseMenuAction::Quit)
    );
}

#[test]
fn test_game_over_transitions() {
    assert_eq!(
        handle_game_over_key(KeyCode::R),
        Some(GameOverAction::Restart)
    );
    assert_eq!(
        handle_game_over_key(KeyCode::M),
        Some(GameOverAction::MainMenu)
    );
    assert_eq!(handle_game_over_key(KeyCode::Q), Some(GameOverAction::Quit));
    assert_eq!(
        handle_game_over_key(KeyCode::Escape),
        Some(GameOverAction::Quit)
    );
}

#[test]
fn test_settings_menu_color_change() {
    let mut config = Config::load("").unwrap_or_default();
    let initial_color = config.player_color;
    let new_color = config.player_color_choices[1];
    assert_ne!(initial_color, new_color);

    let mut in_color_picker = false;
    let mut selection = 0;
    let choices_len = config.player_color_choices.len();

    // 1. Press 'C' to enter color picker
    let a1 = handle_settings_menu_key(
        KeyCode::C,
        &mut in_color_picker,
        &mut selection,
        choices_len,
    );
    assert_eq!(a1, None);
    assert!(in_color_picker);

    // 2. Press Right Arrow to select next color (index 1)
    let a2 = handle_settings_menu_key(
        KeyCode::Right,
        &mut in_color_picker,
        &mut selection,
        choices_len,
    );
    assert_eq!(a2, None);
    assert_eq!(selection, 1);

    // 3. Press Enter to confirm selection
    let a3 = handle_settings_menu_key(
        KeyCode::Enter,
        &mut in_color_picker,
        &mut selection,
        choices_len,
    );
    assert_eq!(a3, Some(SettingsMenuAction::ChangePlayerColor));
    assert!(!in_color_picker);

    // Apply change
    config.player_color = config.player_color_choices[selection];
    assert_eq!(config.player_color, new_color);

    // 4. Press 'B' to go back
    let a4 = handle_settings_menu_key(
        KeyCode::B,
        &mut in_color_picker,
        &mut selection,
        choices_len,
    );
    assert_eq!(a4, Some(SettingsMenuAction::Back));

    // 5. Press 'T' to toggle fullscreen
    let a5 = handle_settings_menu_key(
        KeyCode::T,
        &mut in_color_picker,
        &mut selection,
        choices_len,
    );
    assert_eq!(a5, Some(SettingsMenuAction::ToggleFullscreen));
}
