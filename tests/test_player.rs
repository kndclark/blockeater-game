use blockeater_game::config::Config;
use blockeater_game::player::Player;
use blockeater_game::types::{PlayerDef, PlayerState};

#[test]
fn test_player_movement() {
    let config = Config::load("").unwrap_or_default();
    let screen_width = 640;
    let screen_height = 480;

    struct MovementCase {
        left: bool,
        right: bool,
        up: bool,
        down: bool,
        start_x: i32,
        start_y: i32,
        expected_x: i32,
        expected_y: i32,
        desc: &'static str,
    }

    let cases = vec![
        MovementCase {
            left: false,
            right: false,
            up: true,
            down: false,
            start_x: 100,
            start_y: 100,
            expected_x: 100,
            expected_y: 95,
            desc: "MoveUp",
        },
        MovementCase {
            left: false,
            right: false,
            up: false,
            down: true,
            start_x: 100,
            start_y: 100,
            expected_x: 100,
            expected_y: 105,
            desc: "MoveDown",
        },
        MovementCase {
            left: true,
            right: false,
            up: false,
            down: false,
            start_x: 100,
            start_y: 100,
            expected_x: 95,
            expected_y: 100,
            desc: "MoveLeft",
        },
        MovementCase {
            left: false,
            right: true,
            up: false,
            down: false,
            start_x: 100,
            start_y: 100,
            expected_x: 105,
            expected_y: 100,
            desc: "MoveRight",
        },
        MovementCase {
            left: false,
            right: true,
            up: true,
            down: false,
            start_x: 100,
            start_y: 100,
            expected_x: 105,
            expected_y: 95,
            desc: "MoveDiagonal",
        },
        MovementCase {
            left: true,
            right: false,
            up: false,
            down: false,
            start_x: 2,
            start_y: 100,
            expected_x: 0,
            expected_y: 100,
            desc: "ClampLeft",
        },
        MovementCase {
            left: false,
            right: true,
            up: false,
            down: false,
            start_x: screen_width - 42,
            start_y: 100,
            expected_x: screen_width - 40,
            expected_y: 100,
            desc: "ClampRight",
        },
        MovementCase {
            left: false,
            right: false,
            up: true,
            down: false,
            start_x: 100,
            start_y: 2,
            expected_x: 100,
            expected_y: 0,
            desc: "ClampTop",
        },
        MovementCase {
            left: false,
            right: false,
            up: false,
            down: true,
            start_x: 100,
            start_y: screen_height - 42,
            expected_x: 100,
            expected_y: screen_height - 40,
            desc: "ClampBottom",
        },
    ];

    for c in cases {
        let mut player = Player::new(
            c.start_x,
            c.start_y,
            40,
            40,
            5,
            config.player_color,
            config.dash_speed_multiplier,
            config.dash_duration_ms,
            config.dash_cooldown_ms,
        );
        player.handle_input(
            c.left,
            c.right,
            c.up,
            c.down,
            false,
            screen_width,
            screen_height,
            0,
        );
        assert_eq!(player.rect.x, c.expected_x, "{}: x mismatch", c.desc);
        assert_eq!(player.rect.y, c.expected_y, "{}: y mismatch", c.desc);
    }
}

#[test]
fn test_player_dash_activates_and_increases_speed() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    // Shift + Right
    player.handle_input(false, true, false, false, true, 800, 600, 100);
    assert!(player.is_dashing);
    assert_eq!(player.dash_start_time, 100);
    // Speed 5 * 2.5 = 12
    assert_eq!(player.rect.x, 112);
}

#[test]
fn test_dash_forward_with_no_directional_input() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    // Shift with NO direction keys
    player.handle_input(false, false, false, false, true, 800, 600, 100);
    assert!(player.is_dashing);
    assert_eq!(player.rect.x, 112);
}

#[test]
fn test_dash_ends_and_enters_cooldown() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    player.handle_input(false, true, false, false, true, 800, 600, 1000);
    assert!(player.is_dashing);

    // After 499ms: still dashing
    player.update(1499);
    assert!(player.is_dashing);
    assert!(!player.on_cooldown);

    // After 500ms: dash ends, cooldown begins
    player.update(1500);
    assert!(!player.is_dashing);
    assert!(player.on_cooldown);
    assert_eq!(player.dash_cooldown_start_time, 1500);
}

#[test]
fn test_cannot_dash_while_dashing() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    player.handle_input(false, true, false, false, true, 800, 600, 1000);
    let original_dash_time = player.dash_start_time;

    // Second shift attempt while dashing
    player.handle_input(false, true, false, false, true, 800, 600, 1200);
    assert_eq!(player.dash_start_time, original_dash_time);
}

#[test]
fn test_cooldown_ends_after_duration() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    player.handle_input(false, true, false, false, true, 800, 600, 1000);
    player.update(1500); // Enters cooldown at 1500ms
    assert!(player.on_cooldown);

    // At 3499ms (1999ms into cooldown): still on cooldown
    player.update(3499);
    assert!(player.on_cooldown);

    // At 3500ms (2000ms cooldown elapsed): cooldown ends
    player.update(3500);
    assert!(!player.on_cooldown);
}

#[test]
fn test_cannot_dash_on_cooldown() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    player.handle_input(false, true, false, false, true, 800, 600, 1000);
    player.update(1500); // Enters cooldown
    assert!(player.on_cooldown);

    // Attempt to dash during cooldown
    player.handle_input(false, true, false, false, true, 800, 600, 2000);
    assert!(!player.is_dashing);
    assert_eq!(player.rect.x, 112 + 5); // Normal speed of 5
}

#[test]
fn test_player_grow_shrink_reset_size() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        5,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );

    player.grow(10);
    assert_eq!(player.rect.w, 50);
    assert_eq!(player.rect.h, 50);

    player.shrink(10);
    assert_eq!(player.rect.w, 40);
    assert_eq!(player.rect.h, 40);

    // Shrinking past MIN_SIZE (20) clamps to MIN_SIZE
    player.shrink(30);
    assert_eq!(player.rect.w, Player::MIN_SIZE);
    assert_eq!(player.rect.h, Player::MIN_SIZE);

    player.reset_size();
    assert_eq!(player.rect.w, 40);
    assert_eq!(player.rect.h, 40);
}

#[test]
fn test_player_creation_from_def() {
    let config = Config::load("").unwrap_or_default();
    let def = PlayerDef {
        x: 10,
        y: 20,
        w: 30,
        h: 40,
        speed: 5,
        color: config.player_color,
        dash_speed_multiplier: config.dash_speed_multiplier,
        dash_duration_ms: config.dash_duration_ms,
        dash_cooldown_ms: config.dash_cooldown_ms,
    };
    let player = Player::from_def(&def);
    assert_eq!(player.rect.x, 10);
    assert_eq!(player.rect.y, 20);
    assert_eq!(player.rect.w, 30);
    assert_eq!(player.rect.h, 40);
    assert_eq!(player.speed, 5);
}

#[test]
fn test_spawns_ghosts_while_dashing() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        10,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );
    player.state = PlayerState::Dashing;
    let mut current_time = 1000;

    // First spawn
    player.update_ghosts(current_time);
    assert_eq!(player.ghosts.len(), 1);
    assert_eq!(player.ghosts.last().unwrap().rect.x, player.rect.x);
    assert_eq!(player.ghosts.last().unwrap().creation_time, current_time);

    // No spawn before interval
    player.update_ghosts(current_time + Player::GHOST_SPAWN_INTERVAL_MS - 1);
    assert_eq!(player.ghosts.len(), 1);

    // Second spawn after interval
    current_time += Player::GHOST_SPAWN_INTERVAL_MS + 1;
    player.update_ghosts(current_time);
    assert_eq!(player.ghosts.len(), 2);
}

#[test]
fn test_does_not_spawn_ghosts_when_not_dashing() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        10,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );
    player.state = PlayerState::Ready;
    let current_time = 1000;

    player.update_ghosts(current_time);
    assert!(player.ghosts.is_empty());

    player.update_ghosts(current_time + Player::GHOST_SPAWN_INTERVAL_MS + 1);
    assert!(player.ghosts.is_empty());
}

#[test]
fn test_removes_old_ghosts() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        10,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );
    player.state = PlayerState::Dashing;
    let spawn_time = 1000;

    // Spawn a ghost
    player.update_ghosts(spawn_time);
    assert_eq!(player.ghosts.len(), 1);

    // Update just before it expires
    player.update_ghosts(spawn_time + Player::GHOST_SPAWN_INTERVAL_MS - 1);
    assert_eq!(player.ghosts.len(), 1);

    // Set state to not dashing to prevent new ghosts from spawning
    player.state = PlayerState::Ready;

    // Update just after it expires
    player.update_ghosts(spawn_time + Player::GHOST_LIFETIME_MS + 1);
    assert!(player.ghosts.is_empty());
}

#[test]
fn test_spawns_and_removes_in_same_update() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        10,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );
    player.state = PlayerState::Dashing;
    let first_spawn_time = 1000;

    // 1. Spawn the first ghost.
    player.update_ghosts(first_spawn_time);
    assert_eq!(player.ghosts.len(), 1);
    assert_eq!(
        player.ghosts.first().unwrap().creation_time,
        first_spawn_time
    );

    // 2. Simulate a long delay that is greater than both the spawn interval and the ghost lifetime.
    let update_time = first_spawn_time + Player::GHOST_LIFETIME_MS + 1;
    player.update_ghosts(update_time);

    // 3. Assert that the old ghost was removed AND a new one was spawned.
    assert_eq!(player.ghosts.len(), 1);
    assert_ne!(
        player.ghosts.first().unwrap().creation_time,
        first_spawn_time
    );
    assert_eq!(player.ghosts.first().unwrap().creation_time, update_time);
}

#[test]
fn test_clears_ghosts_when_dash_ends() {
    let config = Config::load("").unwrap_or_default();
    let mut player = Player::new(
        100,
        100,
        40,
        40,
        10,
        config.player_color,
        config.dash_speed_multiplier,
        config.dash_duration_ms,
        config.dash_cooldown_ms,
    );
    player.state = PlayerState::Dashing;
    player.update_ghosts(1000);
    assert!(!player.ghosts.is_empty());

    player.dash_start_time = 1000;
    player.update(1000 + player.dash_duration_ms + 1);

    assert_eq!(player.state, PlayerState::Cooldown);
    assert!(player.ghosts.is_empty());
}
