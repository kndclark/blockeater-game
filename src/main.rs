//! Game entrypoint, CLI smoke test harness, and Macroquad window bootstrap.

use blockeater_game::app::App;
use blockeater_game::config::Config;
use blockeater_game::game::GameState;
use macroquad::prelude::*;

fn window_conf() -> Conf {
    Conf {
        window_title: "THE BLOCKEATER".to_string(),
        window_width: 960,
        window_height: 720,
        high_dpi: true,
        window_resizable: true,
        ..Default::default()
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.iter().any(|arg| arg == "--headless-smoke-test") {
        println!("Running headless smoke test...");
        let config = Config::load("").unwrap_or_default();
        let mut game_state = GameState::new(config, 640, 480);
        for t in 0..100 {
            game_state.update(t * 16);
        }
        println!("Headless smoke test completed successfully!");
        return;
    }

    macroquad::Window::from_config(window_conf(), run());
}

async fn run() {
    let mut app = App::new();
    while app.step().await {}
}
