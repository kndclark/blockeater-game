#include "GameLogic.h"
#include "GameState.h"

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

void ObstacleSpawner::spawn_obstacles(Uint32 current_time, std::vector<Obstacle>& obstacles) {
    // Prioritize spawning checkpoints.
    if (current_time >= last_checkpoint_spawn_time + checkpoint_spawn_interval) {
        last_checkpoint_spawn_time = current_time;
        // Also reset the regular spawn timer to avoid spawning a regular obstacle immediately after.
        last_spawn_time = current_time;

        int final_gap_height = calculateCheckpointGapSize();
        obstacles.push_back(Obstacle::createCheckpoint(screen_width, screen_height, level_manager.getObstacleSpeed(), final_gap_height));

        // Reset the trackers for the next interval.
        shrink_powerups_since_checkpoint.clear();
    }
    // Only spawn a regular obstacle if a checkpoint was not spawned.
    else if (current_time >= last_spawn_time + level_manager.getSpawnInterval()) {
        last_spawn_time = current_time;
        Obstacle new_obstacle = Obstacle::createRegular(screen_width, screen_height, level_manager.getObstacleSpeed(), level_manager.getGrowChance(), level_manager.getShrinkChance(), grow_dims, shrink_dims, hurt_dims);
        if (new_obstacle.type == ObstacleType::Shrink) {
            shrink_powerups_since_checkpoint.push_back(1);
        }
        obstacles.push_back(new_obstacle);
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
                SDL_Log("Level up! You are now level %d.", game_state.level);
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