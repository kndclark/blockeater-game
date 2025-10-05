# Blockeater Game

A simple 2D game created with C++ and SDL2.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

This project is developed for a Linux environment. You will need `git`, `g++`, and `cmake`. You will also need the SDL2 and Google Test development libraries.

On Debian/Ubuntu-based systems, you can install the dependencies with:

```bash
sudo apt-get update && sudo apt-get install -y build-essential git cmake libsdl2-dev libgtest-dev
```

TODO: add devcontainer instructions?

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

## Python Version

This repository also contains a Python version of the game. See the `python_dev` directory for more details. The CI workflow in `.github/workflows/ci.yml` contains instructions for installing dependencies and running the Python tests.