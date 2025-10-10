#include "GameLogic.h"
#include "GameState.h"
#include <SDL2/SDL_ttf.h>

std::vector<Obstacle>::iterator handleCollision(Player& player, std::vector<Obstacle>::iterator it, std::vector<Obstacle>& obstacles, bool& running, int player_size_change_amount) {
    switch (it->type) {
        case ObstacleType::Checkpoint:
            // fallthrough
        case ObstacleType::Hurt:
            SDL_Log("Collision with Hurt obstacle! Game Over.");
            running = false; // End the game
            return ++it; // Advance iterator to avoid re-processing in the game loop
        case ObstacleType::Grow:
            SDL_Log("Collision with Grow obstacle! Player grows.");
            player.grow(player_size_change_amount);
            return obstacles.erase(it); // Erase and get next valid iterator
        case ObstacleType::Shrink:
            SDL_Log("Collision with Shrink obstacle! Player shrinks.");
            player.shrink(player_size_change_amount);
            return obstacles.erase(it); // Erase and get next valid iterator
    }
    // Should not be reached, but some compilers might complain.
    return ++it;
}

int ObstacleSpawner::calculateCheckpointGapSize() const {
    size_t shrink_count = shrink_powerups_since_checkpoint.size();

    const int base_gap_height = level_manager.getBaseCheckpointGap();
    // The minimum gap should be the player's smallest possible size plus a margin.
    const int min_gap_height = Player::MIN_SIZE + 5;
    // Adjust the gap based on how much the player's size changes.
    const int gap_adjustment_per_powerup = player_size_change_amount;

    // The gap size is only decreased by shrink blocks collected. Grow blocks have no effect.
    int shrink_effect = -static_cast<int>(shrink_count) * gap_adjustment_per_powerup;
    int calculated_gap = base_gap_height + shrink_effect;

    // Clamp the gap size to be within reasonable bounds.
    int final_gap_height = std::max(min_gap_height, calculated_gap);
    final_gap_height = std::min(final_gap_height, screen_height - 20); // Ensure walls are at least 10px thick.

    return final_gap_height;
}

void ObstacleSpawner::spawn_obstacles(Uint32 current_time, GameState& game_state) {
    // Prioritize spawning checkpoints.
    if (current_time >= last_checkpoint_spawn_time + checkpoint_spawn_interval) {
        last_checkpoint_spawn_time = current_time;
        // Also reset the regular spawn timer to avoid spawning a regular obstacle immediately after.
        last_spawn_time = current_time;

        const int gap_height = calculateCheckpointGapSize();
        int gap_y;
        game_state.obstacles.push_back(Obstacle::createCheckpoint(screen_width, screen_height, level_manager.getObstacleSpeed(), gap_height, gap_y));
        last_checkpoint_gap_y = {{gap_y, gap_height}};

        // Reset the trackers for the next interval.
        shrink_powerups_since_checkpoint.clear();
        // After spawning a checkpoint, calculate the size for the *next* one.
        // The UI should display the size of the checkpoint that was just spawned.
        game_state.ui_next_checkpoint_gap_size = gap_height;
        game_state.next_checkpoint_gap_size = calculateCheckpointGapSize();
    }
    // Only spawn a regular obstacle if a checkpoint was not spawned.
    else if (current_time >= last_spawn_time + level_manager.getSpawnInterval()) {
        // If we are outside the safe zone duration, clear the gap information
        // so the next obstacle can spawn anywhere.
        if (last_checkpoint_gap_y.has_value() && current_time > last_checkpoint_spawn_time + checkpoint_safe_zone_duration) {
            last_checkpoint_gap_y.reset();
        }

        last_spawn_time = current_time;
        Obstacle new_obstacle = Obstacle::createRegular(screen_width, screen_height,
                                                      level_manager.getObstacleSpeed(),
                                                      level_manager.getGrowChance(), level_manager.getShrinkChance(),
                                                      grow_dims, shrink_dims, hurt_dims, last_checkpoint_gap_y);
        if (new_obstacle.type == ObstacleType::Shrink) {
            shrink_powerups_since_checkpoint.push_back(1);
            // A shrink power-up was collected, so the next gap will be smaller.
            game_state.next_checkpoint_gap_size = calculateCheckpointGapSize();
        }
        game_state.obstacles.push_back(new_obstacle);
    }
}

void handleCheckpointPassing(Player& player, Obstacle& obstacle, GameState& game_state) {
    if (obstacle.type == ObstacleType::Checkpoint && !obstacle.passed) {
        // Check if the player's front has passed the obstacle's back
        if (player.rect.x > obstacle.rect.x + obstacle.rect.w) {
            obstacle.passed = true;
            game_state.score += game_state.config.getScorePerCheckpoint();
            game_state.checkpoints_passed++;
            player.resetSize();
            // Level up every CHECKPOINTS_PER_LEVEL checkpoints
            if (game_state.checkpoints_passed > 0 && game_state.checkpoints_passed % game_state.level_manager.getCheckpointsPerLevel() == 0) {
                game_state.level++;
                game_state.level_manager.updateForLevel(game_state.level);
                SDL_Log("Level up! You are now on level %d.", game_state.level);
            }
            SDL_Log("Checkpoint passed! Score: %d. Level: %d. Checkpoints: %d. Player size reset.", game_state.score, game_state.level, game_state.checkpoints_passed);
        }
    }
}

std::optional<float> calculateFps(Uint32& frame_count, Uint32& last_fps_update_time, Uint32 current_time) {
    frame_count++;
    if (current_time - last_fps_update_time >= 1000) {
        float fps = static_cast<float>(frame_count) / ((current_time - last_fps_update_time) / 1000.0f);
        frame_count = 0;
        last_fps_update_time = current_time;
        return fps;
    }
    return std::nullopt;
}

void batchRenderObstacles(SDL_Renderer* renderer, const std::vector<Obstacle>& obstacles, const Config& config,
                                 std::vector<SDL_Rect>& hurt_rects, std::vector<SDL_Rect>& grow_rects, std::vector<SDL_Rect>& shrink_rects,
                                 std::vector<SDL_Rect>& checkpoint_rects) {
    prepareObstacleBatches(obstacles, hurt_rects, grow_rects, shrink_rects, checkpoint_rects);

    if (!hurt_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Hurt);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, hurt_rects.data(), static_cast<int>(hurt_rects.size()));
    }
    if (!grow_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Grow);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, grow_rects.data(), static_cast<int>(grow_rects.size()));
    }
    if (!shrink_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Shrink);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, shrink_rects.data(), static_cast<int>(shrink_rects.size()));
    }
    if (!checkpoint_rects.empty()) {
        Color c = config.getObstacleColor(ObstacleType::Checkpoint);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRects(renderer, checkpoint_rects.data(), static_cast<int>(checkpoint_rects.size()));
    }
}

/// @brief Processes all pending SDL events and player keyboard input.
/// @param game_state The current state of the game. Will be modified if a quit event is detected.
/// @param SCREEN_WIDTH The width of the screen for boundary checks.
/// @param SCREEN_HEIGHT The height of the screen for boundary checks.
void processInput(GameState& game_state, const int SCREEN_WIDTH, const int SCREEN_HEIGHT) {
    SDL_Event event;
    // Process all pending events in SDL's event queue.
    while (SDL_PollEvent(&event)) {
        // Check if the event is a request to quit the application.
        if (event.type == SDL_QUIT) {
            game_state.running = false;
        }
    }

    // Handle player movement from keyboard state
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    game_state.player.handle_input(keystate, SCREEN_WIDTH, SCREEN_HEIGHT);
}

/// @brief Updates the state of all game objects and handles game logic.
/// @param game_state The current state of the game to be updated.
void updateGame(GameState& game_state) {
    Uint32 current_time = SDL_GetTicks();

    // Update player state (e.g., for dash cooldown)
    game_state.player.update(current_time);

    game_state.spawner.spawn_obstacles(current_time, game_state);

    // Update obstacle positions and remove off-screen ones
    Obstacle::updateAndRemove(game_state.obstacles);

    // Collision detection and game logic
    for (auto it = game_state.obstacles.begin(); it != game_state.obstacles.end(); ) {
        bool collision_detected = SDL_HasIntersection(&game_state.player.rect, &it->rect);
        // For checkpoints, check collision with the second rectangle as well.
        if (it->rect2) {
            collision_detected = collision_detected || SDL_HasIntersection(&game_state.player.rect, &*(it->rect2));
        }

        if (collision_detected) {
            it = handleCollision(game_state.player, it, game_state.obstacles, game_state.running, game_state.config.getPlayerSizeChangeAmount());
        } else {
            handleCheckpointPassing(game_state.player, *it, game_state); // This can change game_state.level
            ++it;
        }
        if (!game_state.running) break; // Exit loop immediately if game is over
    }
}

/// @brief Renders all game objects to the screen.
/// @param renderer The SDL renderer to draw with.
/// @param game_state The current state of the game to be rendered.
/// @param config The game configuration, needed for rendering details.
void renderGame(SDL_Renderer* renderer, const GameState& game_state, const Config& config) { // NOLINT(readability-non-const-parameter)
    // Clear the screen with a dark gray color
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Create temporary vectors for batch rendering. These are cleared and refilled each frame.
    std::vector<SDL_Rect> hurt_rects;
    std::vector<SDL_Rect> grow_rects;
    std::vector<SDL_Rect> shrink_rects;
    std::vector<SDL_Rect> checkpoint_rects;

    // Batch render all obstacles
    batchRenderObstacles(renderer, game_state.obstacles, config, hurt_rects, grow_rects, shrink_rects, checkpoint_rects);

    // Draw the player
    game_state.player.draw(renderer);

}

/// @brief Runs a single iteration of the main game loop, processing input and updating game state.
/// @param game_state The current state of the game to be updated.
/// @param config The game configuration.
void gameLoopIteration(GameState& game_state, const Config& config) {
    processInput(game_state, config.getScreenWidth(), config.getScreenHeight());

    updateGame(game_state);

    // Only check for victory if the game is still running after the update phase
    if (game_state.running) {
        if (game_state.level > LevelManager::MAX_LEVEL) {
            SDL_Log("VICTORY! You have completed all levels!");
            game_state.running = false; // End the game
        }
    }
}

void showGameOverScreen(SDL_Renderer* renderer, const Config& config) {
    // Create a "Game Over" texture
    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    // Use a larger font
    TTF_Font* font = TTF_OpenFont(config.getFontPath().c_str(), 48);
    if (!font) {
        SDL_Log("Failed to load font for game over: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(font, config.getGameOverText().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_FreeSurface(surface);

        const int screen_width = config.getScreenWidth();
        const int screen_height = config.getScreenHeight();
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height - text_height) / 2, text_width, text_height };

        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(texture);
        TTF_CloseFont(font);
    }

    // --- Game Over Loop ---
    bool game_over_loop = true;
    SDL_Event event;
    while (game_over_loop) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                game_over_loop = false;
            }
        }
        SDL_Delay(100); // Prevent the loop from running at full speed
    }
}