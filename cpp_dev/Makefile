.PHONY: all clean test

CXX = g++
# Common flags for all builds
CXXFLAGS_COMMON = -Wall -Isrc -Ithird_party

# By default, we do a debug build. Use `make RELEASE=1` for a release build.
ifeq ($(RELEASE), 1)
	BUILD_TYPE = Release
	CXXFLAGS = $(CXXFLAGS_COMMON) -O3 -DNDEBUG
else
	BUILD_TYPE = Debug
	CXXFLAGS = $(CXXFLAGS_COMMON) -g
endif

SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)
SRC = src/game.cpp config/Config.cpp src/GameState.cpp src/GameLogic.cpp
BIN = build/game

all: $(BIN)

test:
	cmake -S test -B test/build
	cmake --build test/build
	ctest --test-dir test/build --verbose

clean:
	@echo "Cleaning build files..."
	rm -f $(BIN) 
	rm -rf build/ test/build

clean_test: clean test

$(BIN): $(SRC)
	@echo "--- Compiling $(BUILD_TYPE) target ---"
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) -o $(BIN) $(SRC) $(SDL2_LIBS)
