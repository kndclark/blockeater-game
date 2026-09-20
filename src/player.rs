use crate::types::{GameColor, IntRect};

#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct PlayerInput {
    pub left: bool,
    pub right: bool,
    pub up: bool,
    pub down: bool,
    pub dash: bool,
}

impl PlayerInput {
    pub const fn new(left: bool, right: bool, up: bool, down: bool, dash: bool) -> Self {
        Self {
            left,
            right,
            up,
            down,
            dash,
        }
    }
}

#[derive(Debug, Clone)]
pub struct PlayerBuilder {
    rect: IntRect,
    speed: i32,
    color: GameColor,
    dash_speed_multiplier: f32,
    dash_duration_ms: u32,
    dash_cooldown_ms: u32,
}

impl Default for PlayerBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl PlayerBuilder {
    pub fn new() -> Self {
        Self {
            rect: IntRect::new(0, 0, 30, 30),
            speed: 5,
            color: GameColor::new(255, 255, 255, 255),
            dash_speed_multiplier: 2.0,
            dash_duration_ms: 200,
            dash_cooldown_ms: 1000,
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

    pub fn speed(mut self, speed: i32) -> Self {
        self.speed = speed;
        self
    }

    pub fn color(mut self, color: GameColor) -> Self {
        self.color = color;
        self
    }

    pub fn dash_settings(mut self, multiplier: f32, duration_ms: u32, cooldown_ms: u32) -> Self {
        self.dash_speed_multiplier = multiplier;
        self.dash_duration_ms = duration_ms;
        self.dash_cooldown_ms = cooldown_ms;
        self
    }

    pub fn build(self) -> Player {
        Player {
            rect: self.rect,
            speed: self.speed,
            color: self.color,
            default_w: self.rect.w,
            default_h: self.rect.h,
            is_dashing: false,
            on_cooldown: false,
            dash_start_time: 0,
            dash_cooldown_start_time: 0,
            dash_speed_multiplier: self.dash_speed_multiplier,
            dash_duration_ms: self.dash_duration_ms,
            dash_cooldown_ms: self.dash_cooldown_ms,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Player {
    pub rect: IntRect,
    pub speed: i32,
    pub color: GameColor,
    pub default_w: i32,
    pub default_h: i32,

    pub is_dashing: bool,
    pub on_cooldown: bool,
    pub dash_start_time: u32,
    pub dash_cooldown_start_time: u32,

    pub dash_speed_multiplier: f32,
    pub dash_duration_ms: u32,
    pub dash_cooldown_ms: u32,
}

impl Player {
    pub const MIN_SIZE: i32 = 20;

    pub fn builder() -> PlayerBuilder {
        PlayerBuilder::new()
    }

    #[allow(clippy::too_many_arguments)]
    pub fn new(
        x: i32,
        y: i32,
        w: i32,
        h: i32,
        speed: i32,
        color: GameColor,
        dash_speed_multiplier: f32,
        dash_duration_ms: u32,
        dash_cooldown_ms: u32,
    ) -> Self {
        Self::builder()
            .position(x, y)
            .size(w, h)
            .speed(speed)
            .color(color)
            .dash_settings(dash_speed_multiplier, dash_duration_ms, dash_cooldown_ms)
            .build()
    }

    pub fn update(&mut self, current_time: u32) {
        if self.is_dashing
            && (current_time.saturating_sub(self.dash_start_time) >= self.dash_duration_ms)
        {
            self.is_dashing = false;
            self.on_cooldown = true;
            self.dash_cooldown_start_time = current_time;
        }

        if self.on_cooldown
            && (current_time.saturating_sub(self.dash_cooldown_start_time) >= self.dash_cooldown_ms)
        {
            self.on_cooldown = false;
        }
    }

    pub fn apply_input(
        &mut self,
        input: &PlayerInput,
        screen_width: i32,
        screen_height: i32,
        current_time: u32,
    ) {
        if input.dash && !self.is_dashing && !self.on_cooldown {
            self.is_dashing = true;
            self.dash_start_time = current_time;
        }

        let current_speed = if self.is_dashing {
            (self.speed as f32 * self.dash_speed_multiplier) as i32
        } else {
            self.speed
        };

        let any_direction_pressed = input.left || input.right || input.up || input.down;
        if self.is_dashing && !any_direction_pressed {
            self.rect.x += current_speed;
        }

        if input.left {
            self.rect.x -= current_speed;
        }
        if input.right {
            self.rect.x += current_speed;
        }
        if input.up {
            self.rect.y -= current_speed;
        }
        if input.down {
            self.rect.y += current_speed;
        }

        self.rect.x = self.rect.x.clamp(0, (screen_width - self.rect.w).max(0));
        self.rect.y = self.rect.y.clamp(0, (screen_height - self.rect.h).max(0));
    }

    #[allow(clippy::too_many_arguments)]
    pub fn handle_input(
        &mut self,
        left: bool,
        right: bool,
        up: bool,
        down: bool,
        shift: bool,
        screen_width: i32,
        screen_height: i32,
        current_time: u32,
    ) {
        self.apply_input(
            &PlayerInput::new(left, right, up, down, shift),
            screen_width,
            screen_height,
            current_time,
        );
    }

    pub fn grow(&mut self, amount: i32) {
        self.rect.w += amount;
        self.rect.h += amount;
    }

    pub fn shrink(&mut self, amount: i32) {
        self.rect.w = (self.rect.w - amount).max(Self::MIN_SIZE);
        self.rect.h = (self.rect.h - amount).max(Self::MIN_SIZE);
    }

    pub fn reset_size(&mut self) {
        self.rect.w = self.default_w;
        self.rect.h = self.default_h;
    }

    pub fn get_dash_cooldown_remaining(&self, current_time: u32) -> u32 {
        if !self.on_cooldown {
            return 0;
        }
        let elapsed = current_time.saturating_sub(self.dash_cooldown_start_time);
        self.dash_cooldown_ms.saturating_sub(elapsed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn create_test_player() -> Player {
        Player::builder()
            .position(100, 100)
            .size(40, 40)
            .speed(5)
            .color(GameColor::new(128, 0, 128, 255))
            .dash_settings(2.5, 500, 2000)
            .build()
    }

    #[test]
    fn test_player_builder() {
        let p = Player::builder()
            .position(50, 60)
            .size(25, 35)
            .speed(7)
            .color(GameColor::new(10, 20, 30, 255))
            .dash_settings(3.0, 300, 1500)
            .build();

        assert_eq!(p.rect.x, 50);
        assert_eq!(p.rect.y, 60);
        assert_eq!(p.rect.w, 25);
        assert_eq!(p.rect.h, 35);
        assert_eq!(p.speed, 7);
        assert_eq!(p.color, GameColor::new(10, 20, 30, 255));
        assert_eq!(p.dash_speed_multiplier, 3.0);
        assert_eq!(p.dash_duration_ms, 300);
        assert_eq!(p.dash_cooldown_ms, 1500);
    }

    #[test]
    fn test_player_input_struct() {
        let mut p = create_test_player();
        let input = PlayerInput::new(true, false, false, false, false);
        p.apply_input(&input, 200, 200, 0);
        assert_eq!(p.rect.x, 95);
    }

    #[test]
    fn test_player_initial_state() {
        let p = create_test_player();
        assert_eq!(p.rect.x, 100);
        assert_eq!(p.rect.y, 100);
        assert_eq!(p.rect.w, 40);
        assert_eq!(p.rect.h, 40);
        assert!(!p.is_dashing);
        assert!(!p.on_cooldown);
        assert_eq!(p.get_dash_cooldown_remaining(0), 0);
    }

    #[test]
    fn test_player_grow_shrink_limits() {
        let mut p = create_test_player();
        p.grow(15);
        assert_eq!(p.rect.w, 55);
        assert_eq!(p.rect.h, 55);

        p.shrink(20);
        assert_eq!(p.rect.w, 35);
        assert_eq!(p.rect.h, 35);

        p.shrink(50); // Clamps to MIN_SIZE = 20
        assert_eq!(p.rect.w, Player::MIN_SIZE);
        assert_eq!(p.rect.h, Player::MIN_SIZE);

        p.reset_size();
        assert_eq!(p.rect.w, 40);
        assert_eq!(p.rect.h, 40);
    }

    #[test]
    fn test_player_dash_state_lifecycle() {
        let mut p = create_test_player();

        // Trigger dash at t=100
        p.handle_input(false, false, false, false, true, 800, 600, 100);
        assert!(p.is_dashing);
        assert!(!p.on_cooldown);

        // At t=599 (duration 500), still dashing
        p.update(599);
        assert!(p.is_dashing);
        assert!(!p.on_cooldown);

        // At t=600, dash finishes, enters cooldown
        p.update(600);
        assert!(!p.is_dashing);
        assert!(p.on_cooldown);
        assert_eq!(p.get_dash_cooldown_remaining(600), 2000);
        assert_eq!(p.get_dash_cooldown_remaining(1600), 1000);

        // At t=2600, cooldown finishes
        p.update(2600);
        assert!(!p.on_cooldown);
        assert_eq!(p.get_dash_cooldown_remaining(2600), 0);
    }

    #[test]
    fn test_player_bounds_clamping() {
        let mut p = create_test_player();
        // Move far left
        p.handle_input(true, false, false, false, false, 200, 200, 0);
        p.rect.x = -50;
        p.handle_input(true, false, false, false, false, 200, 200, 0);
        assert_eq!(p.rect.x, 0);

        // Move far right
        p.rect.x = 250;
        p.handle_input(false, true, false, false, false, 200, 200, 0);
        assert_eq!(p.rect.x, 200 - 40);
    }
}
