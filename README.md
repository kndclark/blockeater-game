# Blockeater Game

A fast-paced, retro 2D arcade survival game written in idiomatic Rust and powered by [Macroquad](https://macroquad.rs/).

[![Rust CI](https://github.com/kndclark/blockeater-game/actions/workflows/ci.yml/badge.svg)](https://github.com/kndclark/blockeater-game/actions/workflows/ci.yml) [![Status: Work in Progress](https://img.shields.io/badge/status-work%20in%20progress-yellow.svg)](https://github.com/kndclark/blockeater-game) [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

---

## About The Game

**Blockeater** is an arcade survival game where players navigate a hazard-filled corridor, consuming beneficial blocks, avoiding hazards, and slipping through tightening checkpoint gates across 10 escalating levels.

Originally authored in C++ / SDL2, the game has been completely rewritten and aggressively optimized in modern Rust with a zero-allocation particle system, fixed 60 Hz physics simulation, and comprehensive test coverage.

### Key Features
*   **Pure Rust & Macroquad**: Zero C++ dependencies, modern hardware-accelerated 2D rendering with letterboxed isotropic scaling.
*   **Fixed 60 Hz Physics Engine**: Decoupled physics clock guarantees identical game speed, dash response, and obstacle pacing across all monitor refresh rates (60Hz, 144Hz, 240Hz+).
*   **Dynamic Visual FX**: Zero-allocation pooled particle emitter with collision bursts, screen shake, and trail effects.
*   **10-Level Difficulty Scaling**: Gap sizes, obstacle velocities, and spawn intervals adjust dynamically per level with fallback defaults.
*   **High-Score Leaderboard**: Persistent local top-10 leaderboard stored in `data/scores.json`.
*   **Builder Pattern Architecture**: Ergonomic and strictly-typed obstacle and geometry generation.
*   **Comprehensive Test Suite**: 77 unit and integration tests replicating 100% of original test specifications.

---

## How to Play

### Objective
Survive through Level 10 and achieve the highest score possible before running out of points!

### Gameplay Mechanics
*   **Eat Blocks:**
    *   🟩 **Green Blocks (Grow):** Increases player size and grants points.
    *   🟨 **Yellow Blocks (Shrink):** Decreases player size and grants points.
    *   🟥 **Red Blocks (Hurt):** Inflicts a score penalty. If your score drops to 0, it's Game Over!
*   **Checkpoints:**
    *   Pass through the moving gates to advance towards the next level.
    *   Checkpoint gaps become tighter each level.
    *   Passing a checkpoint resets player size back to baseline.
*   **Multipliers:**
    *   **Dash Boost:** Dashing through a checkpoint multiplies points earned (1.5x).
    *   **Size Boost:** Passing obstacles while substantially grown grants a score bonus (2.0x).

### Controls
| Action | Key |
| :--- | :--- |
| **Move Up / Down / Left / Right** | `Arrow Keys` or `W` / `A` / `S` / `D` |
| **Dash** | `Left Shift` or `Right Shift` |
| **Pause / Resume** | `ESC` |
| **Settings (from Menu)** | `C` (Change Color), `T` (Toggle Fullscreen), `B` (Back) |
| **Quit Game** | `Q` |

---

## Getting Started

### Prerequisites

You need the standard Rust toolchain (Rust 1.70+):
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

On Ubuntu / Debian systems, install the graphics and audio development packages:
```bash
sudo apt-get update && sudo apt-get install -y \
    libasound2-dev libx11-dev libxi-dev libgl1-mesa-dev pkg-config
```

On Windows:
No external system libraries are needed. The standard Rust toolchain (MSVC or GNU ABI) builds and runs the game out of the box.


### Building & Running

Clone the repository:
```bash
git clone https://github.com/kndclark/blockeater-game.git
cd blockeater-game
```

#### Run the Game
```bash
cargo run --release
```

#### Run All Tests
```bash
cargo test --all
```

#### Code Quality & Linter
```bash
cargo fmt --all -- --check
cargo clippy --all-targets -- -D warnings
```

#### Headless Smoke Test
Test engine initialization and headless simulation without opening a window:
```bash
cargo run --release -- --headless-smoke-test
```

---

## Project Structure

```
blockeater-game/
├── src/
│   ├── app.rs          # Application lifecycle, state machine, and fixed-step loop
│   ├── config.rs       # Serde JSON configuration parser with compiled-in defaults
│   ├── game.rs         # Pure headless game simulation engine and collision loop
│   ├── level.rs        # Level manager (levels 1-10 configs and overrides)
│   ├── lib.rs          # Module re-exports for integration test harness
│   ├── main.rs         # Lean entrypoint and Macroquad window bootstrap
│   ├── obstacle.rs     # Obstacle struct, builder pattern, and safe-Y gap solver
│   ├── particles.rs    # Zero-allocation particle pool system
│   ├── platform/       # Platform-specific abstractions (Linux EWMH fullscreen FFI)
│   ├── player.rs       # Player movement, PlayerBuilder, PlayerInput, and sizing
│   ├── render.rs       # Viewport scaling and play area scene rendering
│   ├── score.rs        # Score manager and multiplier calculations
│   ├── scoreboard.rs   # Leaderboard manager and JSON file persistence
│   ├── spawner.rs      # Obstacle spawner and shrink history tracker
│   ├── types.rs        # Core geometric primitives, color types, and state enums
│   └── ui.rs           # HUD formatters, text layout, and menu renderers
├── tests/              # 10 integration test suites (parities for legacy GoogleTest suites)
└── data/
    └── scores.json     # High score storage
```

---

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).