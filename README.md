<!-- markdownlint-disable MD033 -->
<p align="center">
  <h1 align="center">Blockeater Game</h1>
  <p align="center">
    A simple 2D game created with C++ and SDL2.
  </p>
  <p align="center">
    <a href="https://github.com/kndclark/blockeater-game/actions/workflows/ci.yml">
      <img src="https://github.com/kndclark/blockeater-game/actions/workflows/ci.yml/badge.svg" alt="Build and Test">
    </a>
    <a href="https://trello.com/b/Lb3KjuAS/blockeater-game"><img src="https://img.shields.io/badge/Trello-board-blue.svg?logo=trello" alt="Trello Board"></a>
    <a href="https://www.gnu.org/licenses/gpl-3.0"><img src="https://img.shields.io/github/license/kndclark/blockeater-game" alt="License: GPL v3"></a>
  </p>
</p>

<!-- TODO: Add a screenshot or GIF of the gameplay -->
<!-- <p align="center">
  <img src="docs/gameplay.gif" alt="Blockeater Gameplay" width="600"/>
</p> -->

## About The Game

Blockeater is a 2D arcade-style game where the player controls a character to consume blocks that appear on the screen.

**Features:**
*   Written in C++ with the SDL2 library for graphics and input.
*   Includes a suite of unit tests using Google Test.
*   Track your score with a local offline leaderboard!

## How to Play

*   **Objective:** Reach level 10 with the highest score possible!

*   **Gameplay Mechanics:**
    *   Score points by eating blocks and passing through checkpoints.
    *   Checkpoint gaps become smaller as you advance through levels.
    *   Different blocks have different effects:
        *   🟩 **Green Blocks:** Make your character grow.
        *   🟨 **Yellow Blocks:** Make your character shrink.
        *   🟥 **Red Blocks:** You lose points.

*   **Controls:**
    *   **Arrow Keys:** Move the player character.
    *   **Shift:** Dash (earns bonus points for dashing through a checkpoint!).
    *   **ESC:** Pause / Quit the game.

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine for development and testing.

### Prerequisites

This project is developed for a Linux environment. You will need `git`, `g++`, and `cmake`. You will also need the SDL2 and Google Test development libraries.

**On Debian/Ubuntu-based systems:**
```bash
sudo apt-get update && sudo apt-get install -y build-essential git cmake libsdl2-dev libgtest-dev
```

### Cloning the Repository

Clone the repository to your local machine using either HTTPS or SSH.

**Using HTTPS:**
```bash
git clone https://github.com/kndclark/blockeater-game.git
cd blockeater-game
```

**Using SSH:**
```bash
git clone git@github.com:kndclark/blockeater-game.git
cd blockeater-game
```

## Building and Running (C++)

The C++ version of the game can be built using the provided Makefile. First, navigate to the C++ development directory:

```bash
cd cpp_dev
```

### Building the Game

To build the game, run the following command. (NOTE: exclude the RELEASE=1 flag to create a debug build)

```bash
make clean && make RELEASE=1
```

### Running the Game

After a successful build, the executable will be in the `build/` directory (relative to `cpp_dev`).

```bash
./build/game
```
*(Note: The executable name might differ. Check the contents of the `cpp_dev/build` directory after building.)*

### Building and Running the Tests

The tests also use CMake and can be built and run separately.

1.  Configure and run the tests using `ctest`:
    ```bash
    make clean_test
    ```