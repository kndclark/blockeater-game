.PHONY: all clean test

CXX = g++
CXXFLAGS = -Wall -g
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)
SRC = src/game.cpp
BIN = build/game

all: $(BIN)

$(BIN): $(SRC)
	mkdir build && $(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) -o $(BIN) $(SRC) $(SDL2_LIBS)

test:
	cmake -S test -B test/build
	cmake --build test/build
	ctest --test-dir test/build --verbose

clean:
	rm -f $(BIN) 
	rm -rf build/

clean_test: clean test
