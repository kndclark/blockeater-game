use blockeater_game::config::Config;
use blockeater_game::game::GameState;

#[test]
fn test_game_state_initialization() {
    let screen_width = 800;
    let screen_height = 600;
    let config = Config::load("").unwrap_or_default();

    let game_state = GameState::new(config.clone(), screen_width, screen_height);

    // Check player initialization
    assert_eq!(game_state.player.rect.x, config.player_initial_x);
    assert_eq!(
        game_state.player.rect.y,
        screen_height / 2 - config.player_height / 2
    );
    assert_eq!(game_state.player.rect.w, config.player_width);
    assert_eq!(game_state.player.rect.h, config.player_height);
    assert_eq!(game_state.player.speed, config.player_speed);
    assert_eq!(game_state.player.color, config.player_color);

    // Check initial state variables
    assert!(game_state.obstacles.is_empty());
    assert_eq!(game_state.score, 0);
    assert!(game_state.running);
    assert_eq!(game_state.frame_count, 0);
    assert_ne!(game_state.last_fps_update_time, 0);

    // Level 1 manager checks
    assert_eq!(game_state.level_manager.effective_obstacle_speed, 3);
    assert_eq!(game_state.level_manager.effective_grow_chance, 20);
    assert_eq!(game_state.level_manager.effective_shrink_chance, 50);
    assert_eq!(game_state.level_manager.effective_base_checkpoint_gap, 200);
}
