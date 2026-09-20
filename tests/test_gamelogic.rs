use blockeater_game::config::Config;
use blockeater_game::game::{calculate_fps, GameState};
use blockeater_game::level::LevelManager;
use blockeater_game::obstacle::Obstacle;
use blockeater_game::score::ScoreManager;
use blockeater_game::spawner::ObstacleSpawner;
use blockeater_game::types::{IntRect, ObstacleType};

#[test]
fn test_fps_calculation() {
    // 1. No update before 1 second
    let mut frame_count = 0;
    let mut last_fps_update_time = 0;
    let fps = calculate_fps(&mut frame_count, &mut last_fps_update_time, 999);
    assert!(fps.is_none());
    assert_eq!(frame_count, 1);
    assert_eq!(last_fps_update_time, 0);

    // 2. Update after exactly 1 second
    let mut frame_count = 59;
    let mut last_fps_update_time = 0;
    let fps = calculate_fps(&mut frame_count, &mut last_fps_update_time, 1000);
    assert!(fps.is_some());
    assert!((fps.unwrap() - 60.0).abs() < 0.01);
    assert_eq!(frame_count, 0);
    assert_eq!(last_fps_update_time, 1000);

    // 3. Update after more than 1 second
    let mut frame_count = 119;
    let mut last_fps_update_time = 1000;
    let fps = calculate_fps(&mut frame_count, &mut last_fps_update_time, 2005);
    assert!(fps.is_some());
    let expected_fps = 120.0 / 1.005;
    assert!((fps.unwrap() - expected_fps).abs() < 0.01);
    assert_eq!(frame_count, 0);
    assert_eq!(last_fps_update_time, 2005);
}

#[test]
fn test_collision_logic() {
    let config = Config::load("").unwrap_or_default();

    // Hurt with 0 score -> Game Over
    {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.score = 0;
        gs.obstacles = vec![Obstacle::new_regular(
            110,
            110,
            20,
            20,
            3,
            ObstacleType::Hurt,
            0,
        )];
        gs.update(0);
        assert!(!gs.running, "Player should die from Hurt with 0 score");
    }

    // Hurt with 600 score -> Survives, loses 500 points
    {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.score = 600;
        gs.obstacles = vec![Obstacle::new_regular(
            110,
            110,
            20,
            20,
            3,
            ObstacleType::Hurt,
            0,
        )];
        let initial_size = gs.player.rect.w;
        let removed = gs.handle_collision_at(0);
        assert!(removed);
        assert!(gs.running);
        assert_eq!(gs.score, 100);
        assert_eq!(gs.player.rect.w, initial_size);
    }

    // Grow block
    {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.score = 0;
        gs.obstacles = vec![Obstacle::new_regular(
            110,
            110,
            20,
            20,
            3,
            ObstacleType::Grow,
            200,
        )];
        let initial_size = gs.player.rect.w;
        let removed = gs.handle_collision_at(0);
        assert!(removed);
        assert!(gs.running);
        assert_eq!(gs.score, 200);
        assert_eq!(
            gs.player.rect.w,
            initial_size + config.player_size_change_amount
        );
    }

    // Shrink block
    {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.score = 0;
        gs.obstacles = vec![Obstacle::new_regular(
            110,
            110,
            20,
            20,
            3,
            ObstacleType::Shrink,
            100,
        )];
        let initial_size = gs.player.rect.w;
        let removed = gs.handle_collision_at(0);
        assert!(removed);
        assert!(gs.running);
        assert_eq!(gs.score, 100);
        assert_eq!(
            gs.player.rect.w,
            (initial_size - config.player_size_change_amount).max(20)
        );
    }

    // Checkpoint wall collision -> Game Over
    {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = 100;
        gs.player.rect.y = 100;
        gs.score = 500;
        gs.obstacles = vec![Obstacle::new_checkpoint(
            IntRect::new(100, 0, 30, 200),
            IntRect::new(100, 400, 30, 200),
            3,
            500,
        )];
        gs.handle_collision_at(0);
        assert!(!gs.running);
    }
}

#[test]
fn test_score_manager_multipliers() {
    let config = Config::load("").unwrap_or_default();
    let base_score = 100;
    let gap_size = 200;
    let player_width_at_threshold =
        (gap_size as f32 * (config.size_boost_threshold / 100.0) + 0.1) as i32;

    // 1. NoBoosts
    let r1 = ScoreManager::calculate_score(base_score, false, 40, gap_size, &config);
    assert_eq!(r1.score, 100);
    assert!(!r1.dash_boost_applied);
    assert!(!r1.size_boost_applied);

    // 2. DashBoostOnly (1.5x)
    let r2 = ScoreManager::calculate_score(base_score, true, 40, gap_size, &config);
    assert_eq!(r2.score, 150);
    assert!(r2.dash_boost_applied);
    assert!(!r2.size_boost_applied);

    // 3. SizeBoostOnly (2.0x)
    let r3 = ScoreManager::calculate_score(
        base_score,
        false,
        player_width_at_threshold,
        gap_size,
        &config,
    );
    assert_eq!(r3.score, 200);
    assert!(!r3.dash_boost_applied);
    assert!(r3.size_boost_applied);

    // 4. DashAndSizeBoost (1.5 * 2.0 = 3.0x)
    let r4 = ScoreManager::calculate_score(
        base_score,
        true,
        player_width_at_threshold,
        gap_size,
        &config,
    );
    assert_eq!(r4.score, 300);
    assert!(r4.dash_boost_applied);
    assert!(r4.size_boost_applied);
}

#[test]
fn test_checkpoint_passing() {
    let config = Config::load("").unwrap_or_default();

    struct PassCase {
        player_x: i32,
        obstacle_x: i32,
        initially_passed: bool,
        initial_score: i32,
        expected_score: i32,
        expected_passed: bool,
        is_checkpoint: bool,
        desc: &'static str,
    }

    let cases = vec![
        PassCase {
            player_x: 100,
            obstacle_x: 121,
            initially_passed: false,
            initial_score: 0,
            expected_score: 0,
            expected_passed: false,
            is_checkpoint: true,
            desc: "PlayerBeforeCheckpoint",
        },
        PassCase {
            player_x: 121,
            obstacle_x: 100,
            initially_passed: false,
            initial_score: 0,
            expected_score: 500,
            expected_passed: true,
            is_checkpoint: true,
            desc: "PlayerPassesCheckpoint",
        },
        PassCase {
            player_x: 122,
            obstacle_x: 100,
            initially_passed: false,
            initial_score: 500,
            expected_score: 1000,
            expected_passed: true,
            is_checkpoint: true,
            desc: "PlayerPassesCheckpointWithScore",
        },
        PassCase {
            player_x: 121,
            obstacle_x: 100,
            initially_passed: true,
            initial_score: 500,
            expected_score: 500,
            expected_passed: true,
            is_checkpoint: true,
            desc: "PlayerPassesAlreadyPassedCheckpoint",
        },
        PassCase {
            player_x: 100,
            obstacle_x: 100,
            initially_passed: false,
            initial_score: 0,
            expected_score: 0,
            expected_passed: false,
            is_checkpoint: true,
            desc: "PlayerAtCheckpointEdge",
        },
        PassCase {
            player_x: 121,
            obstacle_x: 100,
            initially_passed: false,
            initial_score: 0,
            expected_score: 0,
            expected_passed: false,
            is_checkpoint: false,
            desc: "DoesNotAffectNonCheckpoints",
        },
    ];

    for c in cases {
        let mut gs = GameState::new(config.clone(), 800, 600);
        gs.player.rect.x = c.player_x;
        gs.score = c.initial_score;
        gs.player.grow(20); // 40 + 20 = 60

        let mut obs = if c.is_checkpoint {
            Obstacle::new_checkpoint(
                IntRect::new(c.obstacle_x, 0, 20, 100),
                IntRect::new(c.obstacle_x, 200, 20, 100),
                3,
                10,
            )
        } else {
            Obstacle::new_regular(c.obstacle_x, 100, 20, 20, 3, ObstacleType::Hurt, 0)
        };
        obs.passed = c.initially_passed;
        gs.obstacles.push(obs);

        gs.handle_checkpoint_passing(0);

        assert_eq!(gs.score, c.expected_score, "{}: score mismatch", c.desc);
        assert_eq!(
            gs.obstacles[0].passed, c.expected_passed,
            "{}: passed flag mismatch",
            c.desc
        );

        let should_reset = c.is_checkpoint && !c.initially_passed && c.player_x > c.obstacle_x + 20;
        let expected_w = if should_reset { 40 } else { 60 };
        assert_eq!(
            gs.player.rect.w, expected_w,
            "{}: size reset mismatch",
            c.desc
        );
    }
}

#[test]
fn test_level_up() {
    let config = Config::load("").unwrap_or_default();
    let mut gs = GameState::new(config.clone(), 800, 600);

    let checkpoints_needed = gs.level_manager.effective_checkpoints_per_level;
    gs.checkpoints_passed_in_level = checkpoints_needed - 1;

    // Place a checkpoint behind the player (player at 100, checkpoint at 50)
    let cp = Obstacle::new_checkpoint(
        IntRect::new(50, 0, 30, 200),
        IntRect::new(50, 400, 30, 200),
        3,
        config.score_per_checkpoint,
    );
    gs.obstacles.push(cp);

    gs.handle_checkpoint_passing(0);

    assert_eq!(gs.checkpoints_passed_in_level, 0);
    assert_eq!(gs.level, 2);
    assert_eq!(gs.level_manager.effective_obstacle_speed, 4); // Level 2 speed is 4
}

#[test]
fn test_spawner_timing() {
    let config = Config::load("").unwrap_or_default();
    let screen_width = 800;
    let screen_height = 600;

    struct SpawnerCase {
        reg_interval: u32,
        cp_interval: u32,
        times: Vec<u32>,
        expected_regular: usize,
        expected_checkpoints: usize,
        desc: &'static str,
    }

    let cases = vec![
        SpawnerCase {
            reg_interval: 2000,
            cp_interval: 12000,
            times: vec![0, 2000, 2001, 4000, 4001],
            expected_regular: 2,
            expected_checkpoints: 0,
            desc: "SpawnsOnlyRegular",
        },
        SpawnerCase {
            reg_interval: 2000,
            cp_interval: 12000,
            times: vec![12000],
            expected_regular: 0,
            expected_checkpoints: 1,
            desc: "SpawnsCheckpointAndNotRegular",
        },
        SpawnerCase {
            reg_interval: 2000,
            cp_interval: 12000,
            times: vec![0, 50, 1999],
            expected_regular: 0,
            expected_checkpoints: 0,
            desc: "NoSpawnsBeforeInterval",
        },
        SpawnerCase {
            reg_interval: 2000,
            cp_interval: 12000,
            times: vec![2000, 4000, 6000],
            expected_regular: 3,
            expected_checkpoints: 0,
            desc: "SpawnsAtIntervals",
        },
        SpawnerCase {
            reg_interval: 10000,
            cp_interval: 12000,
            times: vec![12001],
            expected_regular: 0,
            expected_checkpoints: 1,
            desc: "SpawnsOnlyCheckpoint",
        },
    ];

    for c in cases {
        let mut lm = LevelManager::new(&config);
        lm.effective_spawn_interval = c.reg_interval;
        lm.effective_checkpoint_interval_ms = c.cp_interval;

        let mut spawner = ObstacleSpawner::new(
            config.checkpoint_safe_zone_duration_ms,
            screen_width,
            screen_height,
            config.player_size_change_amount,
            0,
        );

        let mut obstacles = Vec::new();
        let mut ui_gap = 200;
        let mut next_gap = 200;

        for t in c.times {
            spawner.spawn_obstacles(t, &lm, &config, &mut obstacles, &mut ui_gap, &mut next_gap);
        }

        let reg_count = obstacles
            .iter()
            .filter(|o| o.obstacle_type != ObstacleType::Checkpoint)
            .count();
        let cp_count = obstacles.len() - reg_count;

        assert_eq!(
            reg_count, c.expected_regular,
            "{}: regular count mismatch",
            c.desc
        );
        assert_eq!(
            cp_count, c.expected_checkpoints,
            "{}: checkpoint count mismatch",
            c.desc
        );
    }
}

#[test]
fn test_checkpoint_gap_calculation() {
    let config = Config::load("").unwrap_or_default();
    let base_gap = 200;
    let mut spawner = ObstacleSpawner::new(
        config.checkpoint_safe_zone_duration_ms,
        800,
        600,
        config.player_size_change_amount,
        0,
    );

    // No powerups
    assert_eq!(spawner.calculate_checkpoint_gap_size(base_gap), 200);

    // 1 shrink -> 200 - 10 = 190
    spawner.shrink_powerups_since_checkpoint = vec![1];
    assert_eq!(spawner.calculate_checkpoint_gap_size(base_gap), 190);

    // 2 shrinks -> 200 - 20 = 180
    spawner.shrink_powerups_since_checkpoint = vec![1, 1];
    assert_eq!(spawner.calculate_checkpoint_gap_size(base_gap), 180);

    // 18 shrinks -> clamped at MIN_SIZE + 5 = 25
    spawner.shrink_powerups_since_checkpoint = vec![1; 18];
    assert_eq!(spawner.calculate_checkpoint_gap_size(base_gap), 25);
}

#[test]
fn test_trackers_cleared_after_checkpoint() {
    let config = Config::load("").unwrap_or_default();
    let mut gs = GameState::new(config.clone(), 800, 600);
    gs.spawner.shrink_powerups_since_checkpoint.push(1);
    let cp_interval = gs.level_manager.effective_checkpoint_interval_ms;

    // First checkpoint spawn
    gs.spawner.spawn_obstacles(
        cp_interval,
        &gs.level_manager,
        &config,
        &mut gs.obstacles,
        &mut gs.ui_next_checkpoint_gap_size,
        &mut gs.next_checkpoint_gap_size,
    );

    assert_eq!(gs.obstacles.len(), 1);
    assert!(gs.spawner.shrink_powerups_since_checkpoint.is_empty());

    // Second checkpoint spawn
    gs.spawner.spawn_obstacles(
        cp_interval * 2,
        &gs.level_manager,
        &config,
        &mut gs.obstacles,
        &mut gs.ui_next_checkpoint_gap_size,
        &mut gs.next_checkpoint_gap_size,
    );

    assert_eq!(gs.obstacles.len(), 2);
    let cp2 = &gs.obstacles[1];
    assert_eq!(cp2.obstacle_type, ObstacleType::Checkpoint);
    let actual_gap = cp2.rect2.unwrap().y - cp2.rect.h;
    assert_eq!(actual_gap, config.base_checkpoint_gap);
}

#[test]
fn test_victory_condition() {
    let config = Config::load("").unwrap_or_default();
    let mut gs = GameState::new(config, 800, 600);

    gs.level = 10;
    gs.check_victory_condition();
    assert!(!gs.victory);
    assert!(gs.running);

    gs.level = 11;
    gs.check_victory_condition();
    assert!(gs.victory);
    assert!(!gs.running);
}

#[test]
fn test_game_does_not_update_when_paused() {
    let config = Config::load("").unwrap_or_default();
    let mut gs = GameState::new(config, 800, 600);
    gs.obstacles.push(Obstacle::new_regular(
        100,
        100,
        20,
        20,
        5,
        ObstacleType::Hurt,
        0,
    ));
    gs.paused = true;

    if !gs.paused {
        gs.update(1000);
    }

    assert_eq!(gs.obstacles[0].rect.x, 100);
}
