use blockeater_game::config::Config;
use blockeater_game::obstacle::Obstacle;
use blockeater_game::player::Player;
use blockeater_game::types::{IntRect, ObstacleType};

#[test]
fn test_collision_detection() {
    let config = Config::load("").unwrap_or_default();
    let player = Player::new(
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

    // No collision
    let no_collision = Obstacle::new_regular(200, 200, 20, 20, 3, ObstacleType::Hurt, 0);
    assert!(!player.rect.intersects(&no_collision.rect));

    // Collision
    let collision = Obstacle::new_regular(110, 110, 40, 40, 3, ObstacleType::Hurt, 0);
    assert!(player.rect.intersects(&collision.rect));

    // Edge collision (intersecting by 1 pixel: x in [100..140), obstacle at 139)
    let edge_collision = Obstacle::new_regular(139, 100, 20, 20, 3, ObstacleType::Hurt, 0);
    assert!(player.rect.intersects(&edge_collision.rect));

    // Exactly touching edge (not intersecting: obstacle at 140)
    let non_intersecting_edge = Obstacle::new_regular(140, 100, 20, 20, 3, ObstacleType::Hurt, 0);
    assert!(!player.rect.intersects(&non_intersecting_edge.rect));
}

#[test]
fn test_checkpoint_collision_detection() {
    let config = Config::load("").unwrap_or_default();
    let player = Player::new(
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

    // Checkpoint with a gap the player can fit through [90..150]
    let top_wall = IntRect::new(110, 0, 20, 90);
    let bottom_wall = IntRect::new(110, 150, 20, 330);
    let checkpoint_no_collide = Obstacle::new_checkpoint(top_wall, bottom_wall, 3, 0);

    assert!(!player.rect.intersects(&checkpoint_no_collide.rect));
    assert!(checkpoint_no_collide.rect2.is_some());
    assert!(!player
        .rect
        .intersects(&checkpoint_no_collide.rect2.unwrap()));

    // Player collides with top wall (top wall height = 110, player y = 100)
    let top_wall_collide = IntRect::new(110, 0, 20, 110);
    let bottom_wall_no_collide = IntRect::new(110, 150, 20, 330);
    let checkpoint_collide_top =
        Obstacle::new_checkpoint(top_wall_collide, bottom_wall_no_collide, 3, 0);

    assert!(player.rect.intersects(&checkpoint_collide_top.rect));
    assert!(checkpoint_collide_top.rect2.is_some());
    assert!(!player
        .rect
        .intersects(&checkpoint_collide_top.rect2.unwrap()));

    // Player collides with bottom wall (bottom wall y = 130, player y+h = 140)
    let top_wall_no_collide_2 = IntRect::new(110, 0, 20, 90);
    let bottom_wall_collide = IntRect::new(110, 130, 20, 350);
    let checkpoint_collide_bottom =
        Obstacle::new_checkpoint(top_wall_no_collide_2, bottom_wall_collide, 3, 0);

    assert!(!player.rect.intersects(&checkpoint_collide_bottom.rect));
    assert!(checkpoint_collide_bottom.rect2.is_some());
    assert!(player
        .rect
        .intersects(&checkpoint_collide_bottom.rect2.unwrap()));
}
