#include <iostream>
#include <cassert>
#include <vector>
#include "../src/Player.h"
#include "../src/Obstacle.h"

// Helper function to check rect equality
bool rects_are_equal(const SDL_Rect& r1, const SDL_Rect& r2) {
    return r1.x == r2.x && r1.y == r2.y && r1.w == r2.w && r1.h == r2.h;
}

void test_player_creation() {
    std::cout << "Running test: Player Creation... ";
    Player player(10, 20, 30, 40, 5);
    SDL_Rect expected = {10, 20, 30, 40};
    assert(rects_are_equal(player.rect, expected));
    assert(player.speed == 5);
    std::cout << "PASSED" << std::endl;
}

void test_player_movement() {
    std::cout << "Running test: Player Movement... ";
    Player player(100, 100, 40, 40, 5);

    // Mock keystate array
    Uint8 keystate[SDL_NUM_SCANCODES] = {0};

    // Test moving left
    keystate[SDL_SCANCODE_LEFT] = 1;
    player.handle_input(keystate);
    assert(player.rect.x == 95);
    keystate[SDL_SCANCODE_LEFT] = 0; // Reset

    // Test moving right
    keystate[SDL_SCANCODE_RIGHT] = 1;
    player.handle_input(keystate);
    assert(player.rect.x == 100);
    keystate[SDL_SCANCODE_RIGHT] = 0;

    // Test moving up
    keystate[SDL_SCANCODE_UP] = 1;
    player.handle_input(keystate);
    assert(player.rect.y == 95);
    keystate[SDL_SCANCODE_UP] = 0;

    // Test moving down
    keystate[SDL_SCANCODE_DOWN] = 1;
    player.handle_input(keystate);
    assert(player.rect.y == 100);
    keystate[SDL_SCANCODE_DOWN] = 0;

    std::cout << "PASSED" << std::endl;
}

void test_obstacle_creation() {
    std::cout << "Running test: Obstacle Creation... ";
    Obstacle obstacle(50, 60, 70, 80, 3, ObstacleType::Hurt);
    SDL_Rect expected = {50, 60, 70, 80};
    assert(rects_are_equal(obstacle.rect, expected));
    assert(obstacle.speed == 3);
    assert(obstacle.type == ObstacleType::Hurt);
    std::cout << "PASSED" << std::endl;
}

void test_obstacle_update() {
    std::cout << "Running test: Obstacle Update... ";
    Obstacle obstacle(100, 100, 50, 50, 3, ObstacleType::Hurt);
    obstacle.update();
    assert(obstacle.rect.x == 97);
    obstacle.update();
    assert(obstacle.rect.x == 94);
    std::cout << "PASSED" << std::endl;
}

void test_obstacle_type_assignment() {
    std::cout << "Running test: Obstacle Type Assignment... ";
    Obstacle hurt_obstacle(0, 0, 10, 10, 1, ObstacleType::Hurt);
    assert(hurt_obstacle.type == ObstacleType::Hurt);

    Obstacle grow_obstacle(0, 0, 10, 10, 1, ObstacleType::Grow);
    assert(grow_obstacle.type == ObstacleType::Grow);

    Obstacle shrink_obstacle(0, 0, 10, 10, 1, ObstacleType::Shrink);
    assert(shrink_obstacle.type == ObstacleType::Shrink);
    std::cout << "PASSED" << std::endl;
}

void test_obstacle_offscreen() {
    std::cout << "Running test: Obstacle Offscreen... ";
    // Obstacle fully on screen
    assert(!Obstacle(10, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle touching left edge
    assert(!Obstacle(0, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle partially offscreen
    assert(!Obstacle(-10, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    // Obstacle fully offscreen (right edge at x=0)
    assert(Obstacle(-20, 10, 20, 20, 1, ObstacleType::Hurt).is_offscreen());
    std::cout << "PASSED" << std::endl;
}

void test_collision_detection() {
    std::cout << "Running test: Collision Detection... ";
    Player player(100, 100, 40, 40, 5);

    // No collision
    Obstacle no_collision(200, 200, 20, 20, 3, ObstacleType::Hurt);
    assert(SDL_HasIntersection(&player.rect, &no_collision.rect) == SDL_FALSE);

    // Collision
    Obstacle collision(110, 110, 40, 40, 3, ObstacleType::Hurt);
    assert(SDL_HasIntersection(&player.rect, &collision.rect) == SDL_TRUE);

    // Edge collision (intersecting by 1 pixel)
    Obstacle edge_collision(139, 100, 20, 20, 3, ObstacleType::Hurt);
    assert(SDL_HasIntersection(&player.rect, &edge_collision.rect) == SDL_TRUE);

    std::cout << "PASSED" << std::endl;
}


int main(int argc, char* argv[]) {
    // We need to initialize SDL to use its functions like SDL_HasIntersection
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL for testing: " << SDL_GetError() << std::endl;
        return 1;
    }

    test_player_creation();
    test_player_movement();
    test_obstacle_creation();
    test_obstacle_update();
    test_obstacle_type_assignment();
    test_obstacle_offscreen();
    test_collision_detection();

    std::cout << "\nAll tests passed successfully!" << std::endl;

    SDL_Quit();
    return 0;
}