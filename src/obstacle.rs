use crate::config::ObstacleConfig;
use crate::types::{IntRect, ObstacleSize, ObstacleType};
use rand::Rng;

pub fn determine_obstacle_type(percent_grow: i32, percent_shrink: i32, roll: i32) -> ObstacleType {
    if roll < percent_grow {
        ObstacleType::Grow
    } else if roll < percent_grow + percent_shrink {
        ObstacleType::Shrink
    } else {
        ObstacleType::Hurt
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Obstacle {
    pub rect: IntRect,
    pub rect2: Option<IntRect>,
    pub speed: i32,
    pub obstacle_type: ObstacleType,
    pub points: i32,
    pub passed: bool,
}

#[derive(Debug, Clone)]
pub struct ObstacleBuilder {
    rect: IntRect,
    rect2: Option<IntRect>,
    speed: i32,
    obstacle_type: ObstacleType,
    points: i32,
    passed: bool,
}

impl ObstacleBuilder {
    pub fn new(obstacle_type: ObstacleType) -> Self {
        Self {
            rect: IntRect::default(),
            rect2: None,
            speed: 3,
            obstacle_type,
            points: 0,
            passed: false,
        }
    }

    pub fn position(mut self, x: i32, y: i32) -> Self {
        self.rect.x = x;
        self.rect.y = y;
        self
    }

    pub fn size(mut self, w: i32, h: i32) -> Self {
        self.rect.w = w;
        self.rect.h = h;
        self
    }

    pub fn rect(mut self, x: i32, y: i32, w: i32, h: i32) -> Self {
        self.rect = IntRect::new(x, y, w, h);
        self
    }

    pub fn checkpoint_walls(mut self, top_wall: IntRect, bottom_wall: IntRect) -> Self {
        self.rect = top_wall;
        self.rect2 = Some(bottom_wall);
        self.obstacle_type = ObstacleType::Checkpoint;
        self
    }

    pub fn speed(mut self, speed: i32) -> Self {
        self.speed = speed;
        self
    }

    pub fn points(mut self, points: i32) -> Self {
        self.points = points;
        self
    }

    pub fn passed(mut self, passed: bool) -> Self {
        self.passed = passed;
        self
    }

    pub fn build(self) -> Obstacle {
        Obstacle {
            rect: self.rect,
            rect2: self.rect2,
            speed: self.speed,
            obstacle_type: self.obstacle_type,
            points: self.points,
            passed: self.passed,
        }
    }
}

impl Obstacle {
    pub fn builder(obstacle_type: ObstacleType) -> ObstacleBuilder {
        ObstacleBuilder::new(obstacle_type)
    }

    pub fn new_regular(
        x: i32,
        y: i32,
        w: i32,
        h: i32,
        speed: i32,
        obstacle_type: ObstacleType,
        points: i32,
    ) -> Self {
        Self::builder(obstacle_type)
            .rect(x, y, w, h)
            .speed(speed)
            .points(points)
            .build()
    }

    pub fn new_checkpoint(r1: IntRect, r2: IntRect, speed: i32, points: i32) -> Self {
        Self::builder(ObstacleType::Checkpoint)
            .checkpoint_walls(r1, r2)
            .speed(speed)
            .points(points)
            .build()
    }

    pub fn update(&mut self) {
        self.rect.x -= self.speed;
        if let Some(r2) = &mut self.rect2 {
            r2.x -= self.speed;
        }
    }

    pub fn is_offscreen(&self) -> bool {
        self.rect.x + self.rect.w <= 0
    }

    pub fn update_and_remove(obstacles: &mut Vec<Obstacle>) {
        for obs in obstacles.iter_mut() {
            obs.update();
        }
        obstacles.retain(|obs| !obs.is_offscreen());
    }

    pub fn calculate_safe_y(
        screen_height: i32,
        entity_height: i32,
        nearby_obstacles: &[Obstacle],
        gap_height: Option<i32>,
    ) -> i32 {
        let clearance = 50;
        let mut forbidden_zones: Vec<(i32, i32)> = Vec::with_capacity(nearby_obstacles.len() * 2);

        for obs in nearby_obstacles {
            if obs.obstacle_type == ObstacleType::Checkpoint && obs.rect2.is_some() {
                forbidden_zones.push((0, obs.rect.h));
                if let Some(r2) = obs.rect2 {
                    forbidden_zones.push((r2.y, screen_height - r2.y));
                }
            } else {
                let start_y = (obs.rect.y - clearance).max(0);
                let end_y = (obs.rect.y + obs.rect.h + clearance).min(screen_height);
                forbidden_zones.push((start_y, end_y - start_y));
            }
        }

        forbidden_zones.sort_by_key(|z| z.0);

        let mut merged: Vec<(i32, i32)> = Vec::with_capacity(forbidden_zones.len());
        for zone in forbidden_zones {
            if merged.is_empty() || zone.0 > merged.last().unwrap().0 + merged.last().unwrap().1 {
                merged.push(zone);
            } else {
                let last = merged.last_mut().unwrap();
                last.1 = last.1.max(zone.0 + zone.1 - last.0);
            }
        }

        let mut safe_zones: Vec<(i32, i32)> = Vec::new();
        let mut last_y = 0;
        for zone in &merged {
            if zone.0 > last_y {
                safe_zones.push((last_y, zone.0 - last_y));
            }
            last_y = zone.0 + zone.1;
        }
        if last_y < screen_height {
            safe_zones.push((last_y, screen_height - last_y));
        }

        let required_height = gap_height.unwrap_or(entity_height);
        safe_zones.retain(|zone| zone.1 >= required_height);

        let mut rng = rand::thread_rng();
        if safe_zones.is_empty() {
            let max_span = (screen_height - required_height).max(1);
            return rng.gen_range(0..max_span);
        }

        let chosen = safe_zones[rng.gen_range(0..safe_zones.len())];
        let max_offset = (chosen.1 - required_height).max(0);
        if max_offset == 0 {
            chosen.0
        } else {
            chosen.0 + rng.gen_range(0..=max_offset)
        }
    }

    pub fn create_checkpoint(
        screen_width: i32,
        screen_height: i32,
        speed: i32,
        gap_height: i32,
        points: i32,
        nearby_obstacles: &[Obstacle],
        out_gap_y: &mut i32,
    ) -> Self {
        *out_gap_y = Self::calculate_safe_y(screen_height, 0, nearby_obstacles, Some(gap_height));
        let checkpoint_width = 30;
        let top_wall = IntRect::new(screen_width, 0, checkpoint_width, *out_gap_y);
        let bottom_wall = IntRect::new(
            screen_width,
            *out_gap_y + gap_height,
            checkpoint_width,
            screen_height - (*out_gap_y + gap_height),
        );
        Self::new_checkpoint(top_wall, bottom_wall, speed, points)
    }

    pub fn create_checkpoint_from_def(
        def: &crate::types::CheckpointDef,
        nearby_obstacles: &[Obstacle],
        out_gap_y: &mut i32,
    ) -> Self {
        Self::create_checkpoint(
            def.screen_width,
            def.screen_height,
            def.speed,
            def.gap_height,
            def.points,
            nearby_obstacles,
            out_gap_y,
        )
    }

    pub fn get_obstacle_type_and_size(
        obs_cfg: &ObstacleConfig,
    ) -> (ObstacleType, ObstacleSize, i32) {
        let mut rng = rand::thread_rng();
        let roll = rng.gen_range(0..100);
        let typ = determine_obstacle_type(obs_cfg.grow_chance, obs_cfg.shrink_chance, roll);
        match typ {
            ObstacleType::Grow => (typ, obs_cfg.grow_dims, obs_cfg.grow_points),
            ObstacleType::Shrink => (typ, obs_cfg.shrink_dims, obs_cfg.shrink_points),
            _ => (typ, obs_cfg.hurt_dims, 0),
        }
    }

    pub fn create_regular(
        screen_width: i32,
        screen_height: i32,
        speed: i32,
        obs_cfg: &ObstacleConfig,
        nearby_obstacles: &[Obstacle],
    ) -> Self {
        let (typ, dims, points) = Self::get_obstacle_type_and_size(obs_cfg);
        let y = Self::calculate_safe_y(screen_height, dims.h, nearby_obstacles, None);
        Self::new_regular(screen_width, y, dims.w, dims.h, speed, typ, points)
    }
}

pub fn prepare_obstacle_batches(
    obstacles: &[Obstacle],
    hurt_rects: &mut Vec<IntRect>,
    grow_rects: &mut Vec<IntRect>,
    shrink_rects: &mut Vec<IntRect>,
    checkpoint_rects: &mut Vec<IntRect>,
) {
    hurt_rects.clear();
    grow_rects.clear();
    shrink_rects.clear();
    checkpoint_rects.clear();

    for obs in obstacles {
        match obs.obstacle_type {
            ObstacleType::Hurt => hurt_rects.push(obs.rect),
            ObstacleType::Grow => grow_rects.push(obs.rect),
            ObstacleType::Shrink => shrink_rects.push(obs.rect),
            ObstacleType::Checkpoint => {
                checkpoint_rects.push(obs.rect);
                if let Some(r2) = obs.rect2 {
                    checkpoint_rects.push(r2);
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_determine_obstacle_type_unit() {
        assert_eq!(determine_obstacle_type(40, 40, 20), ObstacleType::Grow);
        assert_eq!(determine_obstacle_type(40, 40, 50), ObstacleType::Shrink);
        assert_eq!(determine_obstacle_type(40, 40, 85), ObstacleType::Hurt);
    }

    #[test]
    fn test_obstacle_update_and_offscreen() {
        let mut obs = Obstacle::new_regular(10, 10, 20, 20, 5, ObstacleType::Hurt, 0);
        assert!(!obs.is_offscreen());
        obs.update();
        assert_eq!(obs.rect.x, 5);
        obs.update();
        assert_eq!(obs.rect.x, 0);
        assert!(!obs.is_offscreen());
        obs.update();
        assert_eq!(obs.rect.x, -5);
        assert!(!obs.is_offscreen()); // -5 + 20 = 15 > 0
        obs.update();
        obs.update();
        obs.update();
        obs.update(); // x = -25, -25 + 20 = -5 <= 0
        assert!(obs.is_offscreen());
    }

    #[test]
    fn test_safe_y_within_screen_bounds() {
        let nearby = Vec::new();
        for _ in 0..100 {
            let y = Obstacle::calculate_safe_y(600, 50, &nearby, None);
            assert!(y >= 0);
            assert!(y + 50 <= 600);
        }
    }

    #[test]
    fn test_checkpoint_gap_creation() {
        let mut gap_y = 0;
        let nearby = Vec::new();
        let cp = Obstacle::create_checkpoint(640, 480, 4, 120, 500, &nearby, &mut gap_y);
        assert_eq!(cp.rect.x, 640);
        assert_eq!(cp.rect.y, 0);
        assert_eq!(cp.rect.h, gap_y);
        assert!(cp.rect2.is_some());
        let r2 = cp.rect2.unwrap();
        assert_eq!(r2.y, gap_y + 120);
        assert_eq!(r2.h, 480 - (gap_y + 120));
    }

    #[test]
    fn test_obstacle_builder() {
        let obs = Obstacle::builder(ObstacleType::Grow)
            .rect(150, 200, 50, 50)
            .speed(5)
            .points(200)
            .passed(true)
            .build();

        assert_eq!(obs.obstacle_type, ObstacleType::Grow);
        assert_eq!(obs.rect.x, 150);
        assert_eq!(obs.rect.y, 200);
        assert_eq!(obs.rect.w, 50);
        assert_eq!(obs.rect.h, 50);
        assert_eq!(obs.speed, 5);
        assert_eq!(obs.points, 200);
        assert!(obs.passed);
        assert!(obs.rect2.is_none());

        let top = IntRect::new(100, 0, 30, 200);
        let bottom = IntRect::new(100, 350, 30, 250);
        let cp = Obstacle::builder(ObstacleType::Checkpoint)
            .checkpoint_walls(top, bottom)
            .speed(4)
            .points(500)
            .build();

        assert_eq!(cp.obstacle_type, ObstacleType::Checkpoint);
        assert_eq!(cp.rect, top);
        assert_eq!(cp.rect2, Some(bottom));
        assert_eq!(cp.speed, 4);
        assert_eq!(cp.points, 500);
    }
}
