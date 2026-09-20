use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct GameColor {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    #[serde(default = "default_alpha")]
    pub a: u8,
}

fn default_alpha() -> u8 {
    255
}

impl GameColor {
    pub const fn new(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self { r, g, b, a }
    }

    pub fn to_macroquad(&self) -> macroquad::prelude::Color {
        macroquad::prelude::Color::new(
            self.r as f32 / 255.0,
            self.g as f32 / 255.0,
            self.b as f32 / 255.0,
            self.a as f32 / 255.0,
        )
    }
}

impl From<[u8; 4]> for GameColor {
    fn from(arr: [u8; 4]) -> Self {
        Self::new(arr[0], arr[1], arr[2], arr[3])
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct IntRect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

impl IntRect {
    pub const fn new(x: i32, y: i32, w: i32, h: i32) -> Self {
        Self { x, y, w, h }
    }

    /// Fast AABB intersection check matching SDL_HasIntersection semantics.
    #[inline(always)]
    pub fn intersects(&self, other: &IntRect) -> bool {
        self.x < other.x + other.w
            && self.x + self.w > other.x
            && self.y < other.y + other.h
            && self.y + self.h > other.y
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct ObstacleSize {
    pub w: i32,
    pub h: i32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum ObstacleType {
    Hurt,
    Grow,
    Shrink,
    Checkpoint,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AppStatus {
    ShowingMainMenu,
    Running,
    ShowingScoreboard,
    Restarting,
    Quitting,
    ShowingSettingsMenu,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MainMenuAction {
    StartGame,
    ShowScoreboard,
    Settings,
    Quit,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SettingsMenuAction {
    ChangePlayerColor,
    ToggleFullscreen,
    Back,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PauseMenuAction {
    Resume,
    Restart,
    MainMenu,
    Quit,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GameOverAction {
    Restart,
    MainMenu,
    Quit,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize, Default)]
pub enum SizeBoostLevel {
    #[default]
    None,
    Good,
    Great,
    Perfect,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct SizeBoostTier {
    pub threshold_percent: i32,
    pub multiplier: f32,
    pub tier: String,
}

impl SizeBoostTier {
    pub fn level(&self) -> SizeBoostLevel {
        match self.tier.as_str() {
            "Perfect" => SizeBoostLevel::Perfect,
            "Great" => SizeBoostLevel::Great,
            "Good" => SizeBoostLevel::Good,
            _ => SizeBoostLevel::None,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum PlayerState {
    #[default]
    Ready,
    Dashing,
    Cooldown,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Ghost {
    pub rect: IntRect,
    pub creation_time: u32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CheckpointDef {
    pub screen_width: i32,
    pub screen_height: i32,
    pub speed: i32,
    pub gap_height: i32,
    pub points: i32,
}

#[derive(Debug, Clone, PartialEq)]
pub struct PlayerDef {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
    pub speed: i32,
    pub color: GameColor,
    pub dash_speed_multiplier: f32,
    pub dash_duration_ms: u32,
    pub dash_cooldown_ms: u32,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ScoreboardRenderData {
    pub score: i32,
    pub level: i32,
    pub current_gap_size: i32,
    pub checkpoints_passed: i32,
    pub checkpoints_per_level: i32,
    pub player_size: i32,
    pub on_cooldown: bool,
    pub cooldown_remaining: u32,
    pub last_boost_level: SizeBoostLevel,
    pub time_since_boost: u32,
    pub dash_boost_active: bool,
    pub time_since_dash_boost: u32,
}

#[derive(Debug, Clone, PartialEq, Default)]
pub struct ScoreCalculationResult {
    pub score: i32,
    pub dash_boost_applied: bool,
    pub size_boost_level: SizeBoostLevel,
}

impl ScoreCalculationResult {
    pub fn size_boost_applied(&self) -> bool {
        self.size_boost_level != SizeBoostLevel::None
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_intrect_intersects_overlapping() {
        let r1 = IntRect::new(10, 10, 20, 20);
        let r2 = IntRect::new(15, 15, 20, 20);
        assert!(r1.intersects(&r2));
        assert!(r2.intersects(&r1));
    }

    #[test]
    fn test_intrect_intersects_disjoint() {
        let r1 = IntRect::new(10, 10, 20, 20);
        let r2 = IntRect::new(40, 40, 20, 20);
        assert!(!r1.intersects(&r2));
        assert!(!r2.intersects(&r1));
    }

    #[test]
    fn test_intrect_adjacent_touching_does_not_intersect() {
        let r1 = IntRect::new(10, 10, 20, 20);
        let r2 = IntRect::new(30, 10, 20, 20); // Right edge of r1 touches left edge of r2
        assert!(!r1.intersects(&r2));
        assert!(!r2.intersects(&r1));
    }

    #[test]
    fn test_gamecolor_conversion_and_from_array() {
        let arr = [255, 128, 64, 200];
        let gc = GameColor::from(arr);
        assert_eq!(gc.r, 255);
        assert_eq!(gc.g, 128);
        assert_eq!(gc.b, 64);
        assert_eq!(gc.a, 200);

        let mq = gc.to_macroquad();
        assert!((mq.r - 1.0).abs() < 0.001);
        assert!((mq.g - (128.0 / 255.0)).abs() < 0.001);
        assert!((mq.b - (64.0 / 255.0)).abs() < 0.001);
        assert!((mq.a - (200.0 / 255.0)).abs() < 0.001);
    }

    #[test]
    fn test_gamecolor_serde_default_alpha() {
        let json = r#"{"r": 100, "g": 150, "b": 200}"#;
        let gc: GameColor = serde_json::from_str(json).unwrap();
        assert_eq!(gc.r, 100);
        assert_eq!(gc.g, 150);
        assert_eq!(gc.b, 200);
        assert_eq!(gc.a, 255);
    }

    #[test]
    fn test_intrect_fully_contained() {
        let outer = IntRect::new(0, 0, 100, 100);
        let inner = IntRect::new(20, 20, 30, 30);
        assert!(outer.intersects(&inner));
        assert!(inner.intersects(&outer));
    }

    #[test]
    fn test_intrect_corner_touching_does_not_intersect() {
        let r1 = IntRect::new(0, 0, 10, 10);
        let r2 = IntRect::new(10, 10, 10, 10);
        assert!(!r1.intersects(&r2));
        assert!(!r2.intersects(&r1));
    }

    #[test]
    fn test_intrect_vertical_and_horizontal_separation() {
        let r1 = IntRect::new(50, 50, 20, 20);
        let r_above = IntRect::new(50, 10, 20, 20);
        let r_below = IntRect::new(50, 90, 20, 20);
        let r_left = IntRect::new(10, 50, 20, 20);
        let r_right = IntRect::new(90, 50, 20, 20);

        assert!(!r1.intersects(&r_above));
        assert!(!r1.intersects(&r_below));
        assert!(!r1.intersects(&r_left));
        assert!(!r1.intersects(&r_right));
    }

    #[test]
    fn test_intrect_zero_dimensions() {
        let r1 = IntRect::new(10, 10, 0, 0);
        let r2 = IntRect::new(10, 10, 20, 20);
        assert!(!r1.intersects(&r2));
        assert!(!r2.intersects(&r1));
    }

    #[test]
    fn test_obstacle_type_serde_roundtrip() {
        let types = [
            ObstacleType::Hurt,
            ObstacleType::Grow,
            ObstacleType::Shrink,
            ObstacleType::Checkpoint,
        ];
        for ot in types {
            let serialized = serde_json::to_string(&ot).unwrap();
            let deserialized: ObstacleType = serde_json::from_str(&serialized).unwrap();
            assert_eq!(ot, deserialized);
        }
    }

    #[test]
    fn test_score_calculation_result_default() {
        let res = ScoreCalculationResult::default();
        assert_eq!(res.score, 0);
        assert!(!res.dash_boost_applied);
        assert_eq!(res.size_boost_level, SizeBoostLevel::None);
        assert!(!res.size_boost_applied());
    }
}
