use blockeater_game::config::Config;
use blockeater_game::types::SizeBoostLevel;
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

    let build_expected_text = |player_size: i32, gap_size: i32| {
        if gap_size <= 0 {
            return format!("{}N/A", config.gap_size_prefix);
        }
        let percentage = ((player_size as f64 / gap_size as f64) * 100.0).round() as i32;
        format!(
            "{}{}{}",
            config.gap_size_prefix, percentage, config.gap_size_suffix
        )
    };

    assert_eq!(
        get_player_size_text(40, 200, &config),
        build_expected_text(40, 200)
    ); // 20%
    assert_eq!(
        get_player_size_text(60, 150, &config),
        build_expected_text(60, 150)
    ); // 40%
    assert_eq!(
        get_player_size_text(150, 150, &config),
        build_expected_text(150, 150)
    ); // 100%
    assert_eq!(
        get_player_size_text(89, 100, &config),
        build_expected_text(89, 100)
    ); // 89%
    assert_eq!(
        get_player_size_text(90, 100, &config),
        build_expected_text(90, 100)
    ); // 90%
    assert_eq!(
        get_player_size_text(89, 99, &config),
        build_expected_text(89, 99)
    ); // 89.89... -> 90%
    assert_eq!(
        get_player_size_text(50, 0, &config),
        build_expected_text(50, 0)
    ); // N/A
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

#[test]
fn test_selects_correct_flash_color_for_boost_message() {
    let config = Config::load("").unwrap_or_default();
    let good_color = config.get_size_boost_tier_color(SizeBoostLevel::Good);
    let great_color = config.get_size_boost_tier_color(SizeBoostLevel::Great);
    let default_color = config.ui_text_color;
    let rainbow_colors = config.get_rainbow_colors();

    struct FlashParam {
        level: SizeBoostLevel,
        time_since_boost: u32,
        expected: blockeater_game::types::GameColor,
        desc: &'static str,
    }

    let params = vec![
        // Good tier flashes between green and default
        FlashParam {
            level: SizeBoostLevel::Good,
            time_since_boost: 100,
            expected: good_color,
            desc: "Good_FirstFlash",
        },
        FlashParam {
            level: SizeBoostLevel::Good,
            time_since_boost: 250,
            expected: default_color,
            desc: "Good_SecondFlash",
        },
        FlashParam {
            level: SizeBoostLevel::Good,
            time_since_boost: 400,
            expected: good_color,
            desc: "Good_ThirdFlash",
        },
        // Great tier flashes between yellow and default
        FlashParam {
            level: SizeBoostLevel::Great,
            time_since_boost: 100,
            expected: great_color,
            desc: "Great_FirstFlash",
        },
        FlashParam {
            level: SizeBoostLevel::Great,
            time_since_boost: 250,
            expected: default_color,
            desc: "Great_SecondFlash",
        },
        // Perfect tier cycles through rainbow colors
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 50,
            expected: rainbow_colors[0],
            desc: "Perfect_Rainbow_0",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 100,
            expected: rainbow_colors[1],
            desc: "Perfect_Rainbow_1",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 150,
            expected: rainbow_colors[2],
            desc: "Perfect_Rainbow_2",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 200,
            expected: rainbow_colors[3],
            desc: "Perfect_Rainbow_3",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 250,
            expected: rainbow_colors[4],
            desc: "Perfect_Rainbow_4",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 300,
            expected: rainbow_colors[5],
            desc: "Perfect_Rainbow_5",
        },
        FlashParam {
            level: SizeBoostLevel::Perfect,
            time_since_boost: 350,
            expected: rainbow_colors[0],
            desc: "Perfect_Rainbow_WrapAround",
        },
    ];

    for p in params {
        let color = get_flash_color_for_boost_message(p.level, p.time_since_boost, &config);
        assert_eq!(color, p.expected, "Failed on: {}", p.desc);
    }
}

#[test]
fn test_get_color_for_size_boost_tier() {
    let config = Config::load("").unwrap_or_default();
    let good_color = config.get_size_boost_tier_color(SizeBoostLevel::Good);
    let great_color = config.get_size_boost_tier_color(SizeBoostLevel::Great);
    let perfect_color = config.get_size_boost_tier_color(SizeBoostLevel::Perfect);
    let default_color = config.ui_text_color;

    // gap_size <= 0 -> default
    assert_eq!(get_color_for_size_boost_tier(50, 0, &config), default_color);

    // Below 30% -> default
    assert_eq!(
        get_color_for_size_boost_tier(29, 100, &config),
        default_color
    );

    // Good tier [30%..50%)
    assert_eq!(get_color_for_size_boost_tier(30, 100, &config), good_color);
    assert_eq!(get_color_for_size_boost_tier(49, 100, &config), good_color);

    // Great tier [50%..80%)
    assert_eq!(get_color_for_size_boost_tier(50, 100, &config), great_color);
    assert_eq!(get_color_for_size_boost_tier(79, 100, &config), great_color);

    // Perfect tier [80%..]
    assert_eq!(
        get_color_for_size_boost_tier(80, 100, &config),
        perfect_color
    );
    assert_eq!(
        get_color_for_size_boost_tier(100, 100, &config),
        perfect_color
    );
}
