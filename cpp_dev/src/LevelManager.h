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
    LevelManager(const Config& base_config)
        : base_config_(base_config),
          effective_spawn_interval_(base_config.getSpawnInterval()),
          effective_obstacle_speed_(base_config.getObstacleSpeed()),
          effective_grow_chance_(base_config.getGrowChance()),
          effective_shrink_chance_(base_config.getShrinkChance()),
          effective_base_checkpoint_gap_(base_config.getBaseCheckpointGap())
    {
        updateForLevel(1);
    }

    void updateForLevel(int level) {
        const LevelConfig* level_cfg = base_config_.getLevelConfig(level);
        if (!level_cfg) {
            // If no specific config for this level, potentially fallback to last known or base.
            // For now, we just use the existing effective values which are from the previous level or base.
            return;
        }

        effective_spawn_interval_ = level_cfg->spawn_interval_ms.value_or(base_config_.getSpawnInterval());
        effective_obstacle_speed_ = level_cfg->obstacle_speed.value_or(base_config_.getObstacleSpeed());
        effective_grow_chance_ = level_cfg->grow_chance_percent.value_or(base_config_.getGrowChance());
        effective_shrink_chance_ = level_cfg->shrink_chance_percent.value_or(base_config_.getShrinkChance());
        effective_base_checkpoint_gap_ = level_cfg->base_checkpoint_gap.value_or(base_config_.getBaseCheckpointGap());
        // Note: We assume the chances in the config sum to 100. The base config validates this,
        // but per-level overrides currently do not. The logic in determineObstacleType handles it gracefully.
        effective_hurt_chance_ = level_cfg->hurt_chance_percent.value_or(base_config_.getHurtChance());
    }

    Uint32 getSpawnInterval() const { return effective_spawn_interval_; }
    int getObstacleSpeed() const { return effective_obstacle_speed_; }
    int getGrowChance() const { return effective_grow_chance_; }
    int getShrinkChance() const { return effective_shrink_chance_; }
    int getBaseCheckpointGap() const { return effective_base_checkpoint_gap_; }
    int getHurtChance() const { return effective_hurt_chance_; }

private:
    const Config& base_config_;
    Uint32 effective_spawn_interval_;
    int effective_obstacle_speed_;
    int effective_grow_chance_;
    int effective_shrink_chance_;
    int effective_hurt_chance_;
    int effective_base_checkpoint_gap_;
};