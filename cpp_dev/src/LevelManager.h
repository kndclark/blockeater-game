#pragma once

#include <map>
#include <SDL2/SDL_stdinc.h> // For Uint32
#include "../config/Config.h"
#include "Obstacle.h"

/// @brief Manages level-specific game parameters.
/// This class holds the effective configuration for the current level,
/// falling back to the base configuration for any values not specified
/// for the current level.
class LevelManager {
public:
    static constexpr int MAX_LEVEL = 10;

    explicit LevelManager(const Config& base_config);

    void updateForLevel(int level);

    Uint32 getSpawnInterval() const { return effective_spawn_interval_; }
    int getObstacleSpeed() const { return effective_obstacle_speed_; }
    int getGrowChance() const { return effective_grow_chance_; }
    int getShrinkChance() const { return effective_shrink_chance_; }
    int getBaseCheckpointGap() const { return effective_base_checkpoint_gap_; }
    int getCheckpointsPerLevel() const { return effective_checkpoints_per_level_; }
    Uint32 getCheckpointInterval() const { return effective_checkpoint_interval_ms_; }
    int getHurtChance() const { return effective_hurt_chance_; }

protected:
    const Config& base_config_;
    Uint32 effective_spawn_interval_;
    int effective_obstacle_speed_;
    int effective_grow_chance_;
    int effective_shrink_chance_;
    int effective_hurt_chance_;
    int effective_base_checkpoint_gap_;
    int effective_checkpoints_per_level_;
    Uint32 effective_checkpoint_interval_ms_;
};