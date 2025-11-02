#include "GameLogic.h"
#include "GameState.h"
#include <SDL2/SDL_ttf.h>
#include <string>

std::vector<Obstacle>::iterator handleCollision(GameState& game_state, std::vector<Obstacle>::iterator it, std::vector<Obstacle>& obstacles) {
    switch (it->type) {
        case ObstacleType::Checkpoint: {
            SDL_Log("Collision with Checkpoint wall! Game Over.");
            game_state.running = false; // End the game
            return ++it; // Advance iterator to avoid re-processing in the game loop
        }
        case ObstacleType::Hurt: {
            if (game_state.score_manager.applyPenalty(game_state.score, game_state.config.getScorePerHurt())) {
                SDL_Log("Collision with Hurt obstacle! Player survives.");
                return obstacles.erase(it); // Erase and get next valid iterator
            }

            SDL_Log("Collision with Hurt obstacle! Not enough score. Game Over.");
            game_state.running = false; // End the game
            return ++it; // Advance iterator to avoid re-processing in the game loop
        }
        case ObstacleType::Grow:
        {
            auto score_result = game_state.score_manager.calculateScore(it->points, game_state);
            game_state.score += score_result.score;
            game_state.player.grow(game_state.config.getPlayerSizeChangeAmount());
            std::string log_message = "Collision with Grow obstacle! Player grows.";
            if (score_result.dash_boost_applied) log_message += " Dash boost!";
            if (score_result.size_boost_applied) log_message += " Size boost!";
            SDL_Log("%s", log_message.c_str());
            return obstacles.erase(it); // Erase and get next valid iterator
        }
        case ObstacleType::Shrink:
        {
            auto score_result = game_state.score_manager.calculateScore(it->points, game_state);
            game_state.score += score_result.score;
            game_state.player.shrink(game_state.config.getPlayerSizeChangeAmount());
            std::string log_message = "Collision with Shrink obstacle! Player shrinks.";
            if (score_result.dash_boost_applied) log_message += " Dash boost!";
            if (score_result.size_boost_applied) log_message += " Size boost!";
            SDL_Log("%s", log_message.c_str());
            return obstacles.erase(it); // Erase and get next valid iterator
        }
    }
    // Should not be reached, but some compilers might complain.
    return ++it;
}

ObstacleSpawner::ObstacleSpawner(const LevelManager& lm, Uint32 safe_zone_duration,
                  int width, int height, int size_change)
    : level_manager(lm), checkpoint_safe_zone_duration(safe_zone_duration),
      screen_width(width), screen_height(height),
      player_size_change_amount(size_change) {}


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
    // Find all obstacles in the "spawn zone" (e.g., right quarter of the screen)
    // to avoid spawning new obstacles on top of them.
    nearby_obstacles.clear();
    for (const auto& obs : game_state.obstacles) {
        if (obs.rect.x > screen_width * 3 / 4) {
            nearby_obstacles.push_back(obs);
        }
    }

    // Prioritize spawning checkpoints.
    if (current_time > 0 && current_time >= last_checkpoint_spawn_time + level_manager.getCheckpointInterval()) {
        last_checkpoint_spawn_time = current_time;

        const int gap_height = calculateCheckpointGapSize();
        int gap_y;
        // Add the new checkpoint and get a pointer to it.
        game_state.obstacles.push_back(Obstacle::createCheckpoint(screen_width, screen_height, level_manager.getObstacleSpeed(), gap_height, game_state.config.getScorePerCheckpoint(), nearby_obstacles, gap_y));
        last_checkpoint = &game_state.obstacles.back();

        // Reset the trackers for the next interval.
        shrink_powerups_since_checkpoint.clear();
        // After spawning a checkpoint, calculate the size for the *next* one.
        // The UI should display the size of the checkpoint that was just spawned.
        game_state.ui_next_checkpoint_gap_size = gap_height;
        game_state.next_checkpoint_gap_size = calculateCheckpointGapSize();
        // Also reset the regular spawn timer to avoid spawning a regular obstacle immediately after.
        last_spawn_time = current_time;
        return; // Return early to enforce the safe zone after a checkpoint.
    }
    // Check for regular obstacle spawns independently.
    if (current_time > 0 &&
        current_time >= last_spawn_time + level_manager.getSpawnInterval() &&
        current_time > last_checkpoint_spawn_time + checkpoint_safe_zone_duration) {

        last_spawn_time = current_time;
        Obstacle new_obstacle = Obstacle::createRegular(screen_width, screen_height, level_manager.getObstacleSpeed(),
                                                      game_state.config.getObstacleConfig(),
                                                      nearby_obstacles);
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
            auto score_result = game_state.score_manager.calculateScore(game_state.config.getScorePerCheckpoint(), game_state);
            game_state.score += score_result.score;
            game_state.checkpoints_passed_in_level++;
            game_state.checkpoints_passed++;
            player.resetSize();
            // Level up every CHECKPOINTS_PER_LEVEL checkpoints
            if (game_state.checkpoints_passed_in_level >= game_state.level_manager.getCheckpointsPerLevel()) {
                game_state.level++;
                game_state.checkpoints_passed_in_level = 0;
                game_state.level_manager.updateForLevel(game_state.level);
                SDL_Log("Level up! You are now on level %d.", game_state.level);
            }
            std::string log_message = "Checkpoint passed! Player size reset.";
            if (score_result.dash_boost_applied) log_message += " Dash boost!";
            if (score_result.size_boost_applied) log_message += " Size boost!";
            SDL_Log("%s Score: %d. Level: %d. Checkpoints: %d.", log_message.c_str(), game_state.score, game_state.level, game_state.checkpoints_passed);
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
        if (event.type == SDL_KEYDOWN) {
            // Toggle pause state on ESC key press
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                game_state.paused = !game_state.paused;
                SDL_Log("Game paused: %s", game_state.paused ? "true" : "false");
            }
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
            it = handleCollision(game_state, it, game_state.obstacles);
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

GameOverAction showGameOverScreen(SDL_Renderer* renderer, const Config& config, const std::string& message) {
    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // --- Main Message (Game Over / Victory) ---
    TTF_Font* main_font = TTF_OpenFont(config.getFontPath().c_str(), 48);
    if (!main_font) {
        SDL_Log("Failed to load main font for game over: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(main_font, message.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) - text_height, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(main_font);
    }

    // --- Instructions Text ---
    TTF_Font* instruction_font = TTF_OpenFont(config.getFontPath().c_str(), 24);
    if (!instruction_font) {
        SDL_Log("Failed to load instruction font for game over: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(instruction_font, config.getGameOverInstructions().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) + 20, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(instruction_font);
    }
 
    SDL_RenderPresent(renderer);

    // --- Game Over Event Loop ---
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return GameOverAction::Quit;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_r: return GameOverAction::Restart;
                    case SDLK_m: return GameOverAction::MainMenu; // Does nothing for now
                    case SDLK_q: case SDLK_ESCAPE: return GameOverAction::Quit;
                    default: break;
                }
            }
        }
        SDL_Delay(100);
    }
}

PauseMenuAction showPauseMenu(SDL_Renderer* renderer, const Config& config) {
    Color c = config.getUiTextColor();
    SDL_Color color = {c.r, c.g, c.b, c.a};
    const int screen_width = config.getScreenWidth();
    const int screen_height = config.getScreenHeight();

    // --- Main Message (Paused) ---
    TTF_Font* main_font = TTF_OpenFont(config.getFontPath().c_str(), 48);
    if (!main_font) {
        SDL_Log("Failed to load main font for pause menu: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(main_font, config.getPauseMenuTitle().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) - text_height, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(main_font);
    }

    // --- Instructions Text ---
    TTF_Font* instruction_font = TTF_OpenFont(config.getFontPath().c_str(), 24);
    if (!instruction_font) {
        SDL_Log("Failed to load instruction font for pause menu: %s", TTF_GetError());
    } else {
        SDL_Surface* surface = TTF_RenderText_Solid(instruction_font, config.getPauseMenuInstructions().c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_Rect dest_rect = { (screen_width - text_width) / 2, (screen_height / 2) + 20, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        TTF_CloseFont(instruction_font);
    }

    SDL_RenderPresent(renderer);

    // --- Pause Menu Event Loop ---
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return PauseMenuAction::Quit;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: return PauseMenuAction::Resume;
                    case SDLK_r: return PauseMenuAction::Restart;
                    case SDLK_m: return PauseMenuAction::MainMenu;
                    case SDLK_q: return PauseMenuAction::Quit;
                    default: break;
                }
            }
        }
        SDL_Delay(100);
    }
}

void handlePauseMenuAction(PauseMenuAction action, GameState& game_state, bool& restart_requested, bool& app_is_running) {
    switch (action) {
        case PauseMenuAction::Resume:
            game_state.paused = false;
            break;
        case PauseMenuAction::Restart:
            restart_requested = true;
            game_state.running = false; // Break inner loop to restart
            break;
        case PauseMenuAction::MainMenu:
            SDL_Log("Main Menu selected. Exiting for now.");
            game_state.running = false; // This will break the inner loop
            app_is_running = false;    // This will break the outer loop
            break;
        case PauseMenuAction::Quit:
            game_state.running = false;
            app_is_running = false;
            break;
    }
}

void checkVictoryCondition(GameState& game_state) {
    if (game_state.level > game_state.level_manager.getMaxLevel()) {
        game_state.victory = true;
        game_state.running = false; // End the game
    }
}

void handleGameLoop(SDL_Renderer* renderer, GameState& game_state, Scoreboard& scoreboard, const Config& config) {
    const int TARGET_FPS = config.getTargetFps();
    const Uint32 FRAME_DELAY = (TARGET_FPS > 0) ? 1000 / TARGET_FPS : 0;
    Uint32 frame_start_time = SDL_GetTicks();

    processInput(game_state, config.getScreenWidth(), config.getScreenHeight());

    if (!game_state.paused) {
        updateGame(game_state);
    }

    // Only render if the game is still running after the update phase
    if (game_state.running) {
        renderGame(renderer, game_state, config);
        scoreboard.render(game_state.score, game_state.level, game_state.ui_next_checkpoint_gap_size,
                           game_state.checkpoints_passed_in_level, game_state.level_manager.getCheckpointsPerLevel(),
                           game_state.player.rect.w, game_state.player.on_cooldown, game_state.player.getDashCooldownRemaining());
        SDL_RenderPresent(renderer);
    }

    // --- FPS Calculation and Capping ---
    if (auto fps_opt = calculateFps(game_state.frame_count, game_state.last_fps_update_time, SDL_GetTicks())) {
        SDL_Log("FPS: %.2f", *fps_opt);
    }

    Uint32 frame_time = SDL_GetTicks() - frame_start_time;
    if (frame_time < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frame_time);
    }
}