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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct ScoreCalculationResult {
    pub score: i32,
    pub dash_boost_applied: bool,
    pub size_boost_applied: bool,
}
