use blockeater_game::config::Config;
use blockeater_game::level::LevelManager;

#[test]
fn test_loads_level_specific_config() {
    let config = Config::load("").unwrap_or_default();
    let mut lm = LevelManager::new(&config);

    // Expected values from levels.json
    let expected_levels = [
        (1, 3, 2000, 200, 12000, 20, 50, 30, 5),
        (2, 4, 1850, 150, 11000, 25, 45, 30, 6),
        (3, 5, 1700, 125, 10500, 30, 40, 30, 6),
        (4, 6, 1550, 110, 10000, 35, 35, 30, 7),
        (5, 7, 1400, 105, 9500, 40, 30, 30, 7),
        (6, 8, 1250, 100, 9000, 45, 25, 30, 8),
        (7, 9, 1100, 95, 8500, 50, 20, 30, 8),
        (8, 10, 950, 90, 8000, 55, 15, 30, 9),
        (9, 11, 800, 85, 7500, 60, 10, 30, 10),
        (10, 12, 650, 80, 7000, 65, 5, 30, 10),
    ];

    for (lvl, speed, spawn_int, gap, cp_int, grow, shrink, hurt, cp_per_lvl) in expected_levels {
        lm.update_for_level(lvl, &config);

        assert_eq!(lm.effective_obstacle_speed, speed, "Lvl {} speed", lvl);
        assert_eq!(
            lm.effective_spawn_interval, spawn_int,
            "Lvl {} spawn interval",
            lvl
        );
        assert_eq!(lm.effective_base_checkpoint_gap, gap, "Lvl {} gap", lvl);
        assert_eq!(
            lm.effective_checkpoint_interval_ms, cp_int,
            "Lvl {} cp interval",
            lvl
        );
        assert_eq!(lm.effective_grow_chance, grow, "Lvl {} grow", lvl);
        assert_eq!(lm.effective_shrink_chance, shrink, "Lvl {} shrink", lvl);
        assert_eq!(lm.effective_hurt_chance, hurt, "Lvl {} hurt", lvl);
        assert_eq!(
            lm.effective_checkpoints_per_level, cp_per_lvl,
            "Lvl {} cp per level",
            lvl
        );
    }
}
