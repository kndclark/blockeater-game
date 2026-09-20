use blockeater_game::config::Config;
use blockeater_game::obstacle::{determine_obstacle_type, prepare_obstacle_batches, Obstacle};
use blockeater_game::types::{IntRect, ObstacleType};

#[test]
fn test_obstacle_creation() {
    let obstacle = Obstacle::new_regular(50, 60, 70, 80, 3, ObstacleType::Hurt, 100);
    assert_eq!(obstacle.rect.x, 50);
    assert_eq!(obstacle.rect.y, 60);
    assert_eq!(obstacle.rect.w, 70);
    assert_eq!(obstacle.rect.h, 80);
    assert_eq!(obstacle.speed, 3);
    assert_eq!(obstacle.obstacle_type, ObstacleType::Hurt);
    assert_eq!(obstacle.points, 100);
    assert!(obstacle.rect2.is_none());
}

#[test]
fn test_create_checkpoint() {
    let mut dummy_gap_y = 0;
    let nearby = Vec::new();
    let o = Obstacle::create_checkpoint(800, 600, 3, 150, 50, &nearby, &mut dummy_gap_y);
    assert_eq!(o.obstacle_type, ObstacleType::Checkpoint);
    assert_eq!(o.points, 50);
    assert!(o.rect2.is_some());
    assert_eq!(o.speed, 3);
    assert_eq!(o.rect.x, 800);
}

#[test]
fn test_obstacle_update() {
    let mut obstacle = Obstacle::new_regular(100, 100, 50, 50, 3, ObstacleType::Hurt, 0);
    obstacle.update();
    assert_eq!(obstacle.rect.x, 97);
    obstacle.update();
    assert_eq!(obstacle.rect.x, 94);
}

#[test]
fn test_obstacle_is_offscreen() {
    // Fully on screen
    assert!(!Obstacle::new_regular(10, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Touching left edge
    assert!(!Obstacle::new_regular(0, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Partially offscreen
    assert!(!Obstacle::new_regular(-10, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
    // Fully offscreen (right edge at x=0)
    assert!(Obstacle::new_regular(-20, 10, 20, 20, 1, ObstacleType::Hurt, 0).is_offscreen());
}

#[test]
fn test_obstacle_type_assignment() {
    let hurt = Obstacle::new_regular(0, 0, 10, 10, 1, ObstacleType::Hurt, 0);
    assert_eq!(hurt.obstacle_type, ObstacleType::Hurt);
    assert_eq!(hurt.points, 0);

    let grow = Obstacle::new_regular(0, 0, 10, 10, 1, ObstacleType::Grow, 200);
    assert_eq!(grow.obstacle_type, ObstacleType::Grow);
    assert_eq!(grow.points, 200);

    let shrink = Obstacle::new_regular(0, 0, 10, 10, 1, ObstacleType::Shrink, 100);
    assert_eq!(shrink.obstacle_type, ObstacleType::Shrink);
    assert_eq!(shrink.points, 100);

    let cp = Obstacle::new_checkpoint(IntRect::default(), IntRect::default(), 1, 10);
    assert_eq!(cp.obstacle_type, ObstacleType::Checkpoint);
    assert_eq!(cp.points, 10);
}

#[test]
fn test_create_regular_with_points() {
    let config = Config::load("").unwrap_or_default();
    let mut obs_config = config.get_obstacle_config();
    assert_eq!(obs_config.grow_points, 200);
    assert_eq!(obs_config.shrink_points, 100);

    // Force Grow
    obs_config.grow_chance = 100;
    obs_config.shrink_chance = 0;
    let nearby = Vec::new();
    let grow_obs = Obstacle::create_regular(800, 600, 3, &obs_config, &nearby);
    assert_eq!(grow_obs.obstacle_type, ObstacleType::Grow);
    assert_eq!(grow_obs.points, 200);

    // Force Shrink
    obs_config.grow_chance = 0;
    obs_config.shrink_chance = 100;
    let shrink_obs = Obstacle::create_regular(800, 600, 3, &obs_config, &nearby);
    assert_eq!(shrink_obs.obstacle_type, ObstacleType::Shrink);
    assert_eq!(shrink_obs.points, 100);
}

#[test]
fn test_determine_obstacle_type() {
    let cases = vec![
        // 40/40/20 distribution
        (40, 40, 0, ObstacleType::Grow),
        (40, 40, 39, ObstacleType::Grow),
        (40, 40, 40, ObstacleType::Shrink),
        (40, 40, 79, ObstacleType::Shrink),
        (40, 40, 80, ObstacleType::Hurt),
        (40, 40, 99, ObstacleType::Hurt),
        // 10/10/80 distribution
        (10, 10, 9, ObstacleType::Grow),
        (10, 10, 19, ObstacleType::Shrink),
        (10, 10, 20, ObstacleType::Hurt),
    ];

    for (grow, shrink, roll, expected) in cases {
        assert_eq!(
            determine_obstacle_type(grow, shrink, roll),
            expected,
            "Failed for grow={}, shrink={}, roll={}",
            grow,
            shrink,
            roll
        );
    }
}

#[test]
fn test_prepare_obstacle_batches() {
    struct BatchCase {
        types: Vec<ObstacleType>,
        expected_hurt: usize,
        expected_grow: usize,
        expected_shrink: usize,
        expected_checkpoints: usize,
    }

    let cases = vec![
        BatchCase {
            types: vec![
                ObstacleType::Hurt,
                ObstacleType::Grow,
                ObstacleType::Grow,
                ObstacleType::Shrink,
                ObstacleType::Shrink,
                ObstacleType::Shrink,
            ],
            expected_hurt: 1,
            expected_grow: 2,
            expected_shrink: 3,
            expected_checkpoints: 0,
        },
        BatchCase {
            types: vec![],
            expected_hurt: 0,
            expected_grow: 0,
            expected_shrink: 0,
            expected_checkpoints: 0,
        },
        BatchCase {
            types: vec![ObstacleType::Hurt, ObstacleType::Hurt, ObstacleType::Hurt],
            expected_hurt: 3,
            expected_grow: 0,
            expected_shrink: 0,
            expected_checkpoints: 0,
        },
        BatchCase {
            types: vec![ObstacleType::Grow, ObstacleType::Grow],
            expected_hurt: 0,
            expected_grow: 2,
            expected_shrink: 0,
            expected_checkpoints: 0,
        },
        BatchCase {
            types: vec![ObstacleType::Shrink],
            expected_hurt: 0,
            expected_grow: 0,
            expected_shrink: 1,
            expected_checkpoints: 0,
        },
        BatchCase {
            types: vec![ObstacleType::Checkpoint],
            expected_hurt: 0,
            expected_grow: 0,
            expected_shrink: 0,
            expected_checkpoints: 1,
        },
        BatchCase {
            types: vec![
                ObstacleType::Checkpoint,
                ObstacleType::Hurt,
                ObstacleType::Checkpoint,
            ],
            expected_hurt: 1,
            expected_grow: 0,
            expected_shrink: 0,
            expected_checkpoints: 2,
        },
    ];

    for c in cases {
        let mut obstacles = Vec::new();
        for typ in c.types {
            if typ == ObstacleType::Checkpoint {
                obstacles.push(Obstacle::new_checkpoint(
                    IntRect::new(0, 0, 10, 10),
                    IntRect::new(0, 0, 10, 10),
                    1,
                    10,
                ));
            } else {
                obstacles.push(Obstacle::new_regular(0, 0, 10, 10, 1, typ, 0));
            }
        }

        let mut hurt = Vec::new();
        let mut grow = Vec::new();
        let mut shrink = Vec::new();
        let mut cp = Vec::new();

        prepare_obstacle_batches(&obstacles, &mut hurt, &mut grow, &mut shrink, &mut cp);

        assert_eq!(hurt.len(), c.expected_hurt);
        assert_eq!(grow.len(), c.expected_grow);
        assert_eq!(shrink.len(), c.expected_shrink);
        assert_eq!(cp.len(), c.expected_checkpoints * 2);
    }
}

#[test]
fn test_calculate_safe_y_avoids_gap() {
    let screen_height = 600;
    let obstacle_height = 50;
    // Checkpoint with gap from y=200 to y=400. Walls are [0, 200] and [400, 600].
    let nearby = vec![Obstacle::new_checkpoint(
        IntRect::new(0, 0, 10, 200),
        IntRect::new(0, 400, 10, 200),
        1,
        0,
    )];

    for _ in 0..1000 {
        let y = Obstacle::calculate_safe_y(screen_height, obstacle_height, &nearby, None);
        let overlaps =
            (y < 200 && y + obstacle_height > 0) || (y < 600 && y + obstacle_height > 400);
        assert!(!overlaps, "Obstacle at y={} overlaps checkpoint walls", y);
    }
}

#[test]
fn test_calculate_safe_y_avoids_last_obstacle() {
    let screen_height = 600;
    let obstacle_height = 50;
    let clearance = 50;

    let last_obstacle_rect = IntRect::new(0, 275, 50, 50);
    let nearby = vec![Obstacle::new_regular(
        last_obstacle_rect.x,
        last_obstacle_rect.y,
        last_obstacle_rect.w,
        last_obstacle_rect.h,
        1,
        ObstacleType::Hurt,
        0,
    )];

    for _ in 0..1000 {
        let y = Obstacle::calculate_safe_y(screen_height, obstacle_height, &nearby, None);
        let overlaps = (y < last_obstacle_rect.y + last_obstacle_rect.h + clearance)
            && (y + obstacle_height > last_obstacle_rect.y - clearance);
        assert!(
            !overlaps,
            "Obstacle spawned at y={} overlaps last obstacle clearance zone",
            y
        );
    }
}

#[test]
fn test_checkpoint_walls_avoid_nearby_obstacles() {
    let screen_width = 800;
    let screen_height = 600;
    let speed = 3;
    let points = 10;
    let gap_height = 150;

    let test_obstacle_sets: Vec<Vec<IntRect>> = vec![
        vec![IntRect::new(0, 0, 50, 50)],
        vec![IntRect::new(0, 50, 50, 50)],
        vec![IntRect::new(0, 275, 50, 50)],
        vec![IntRect::new(0, 500, 50, 50)],
        vec![IntRect::new(0, 550, 50, 50)],
        vec![IntRect::new(0, 200, 50, 200)],
        vec![IntRect::new(0, 275, 40, 40)],
        vec![IntRect::new(0, 275, 20, 20)],
        vec![IntRect::new(0, 100, 50, 50), IntRect::new(0, 400, 50, 50)],
        vec![IntRect::new(0, 50, 40, 40), IntRect::new(0, 500, 20, 20)],
    ];

    for obstacle_rects in test_obstacle_sets {
        let nearby: Vec<Obstacle> = obstacle_rects
            .iter()
            .map(|r| Obstacle::new_regular(r.x, r.y, r.w, r.h, 1, ObstacleType::Hurt, 0))
            .collect();

        for _ in 0..100 {
            let mut dummy_gap_y = 0;
            let cp = Obstacle::create_checkpoint(
                screen_width,
                screen_height,
                speed,
                gap_height,
                points,
                &nearby,
                &mut dummy_gap_y,
            );

            for rect in &obstacle_rects {
                // Check 2D intersection
                assert!(!cp.rect.intersects(rect));
                assert!(!cp.rect2.unwrap().intersects(rect));

                // Also check vertical clearance: gap should cover the obstacle
                let gap_top = dummy_gap_y;
                let gap_bottom = dummy_gap_y + gap_height;
                let clearance = 50;
                let top_wall_safe =
                    gap_top >= rect.y + rect.h + clearance || gap_top <= rect.y - clearance;
                let bottom_wall_safe =
                    gap_bottom <= rect.y - clearance || gap_bottom >= rect.y + rect.h + clearance;
                assert!(top_wall_safe || bottom_wall_safe);
            }
        }
    }
}

#[test]
fn test_update_and_remove() {
    let mut obstacles = vec![
        Obstacle::new_regular(100, 100, 20, 20, 5, ObstacleType::Hurt, 0),
        Obstacle::new_regular(-30, 100, 20, 20, 5, ObstacleType::Hurt, 0),
        // 10 - 30 = -20; -20 + 20 <= 0 -> removed!
        Obstacle::new_regular(10, 100, 20, 20, 30, ObstacleType::Grow, 0),
        Obstacle::new_checkpoint(
            IntRect::new(200, 0, 20, 200),
            IntRect::new(200, 300, 20, 300),
            2,
            10,
        ),
    ];

    Obstacle::update_and_remove(&mut obstacles);

    assert_eq!(obstacles.len(), 2);
    assert_eq!(obstacles[0].obstacle_type, ObstacleType::Hurt);
    assert_eq!(obstacles[1].obstacle_type, ObstacleType::Checkpoint);
    assert_eq!(obstacles[0].rect.x, 95);
    assert_eq!(obstacles[1].rect.x, 198);
}
