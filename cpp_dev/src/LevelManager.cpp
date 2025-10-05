#include "LevelManager.h"
#include "../config/Config.h"

// Definition for the static member.
constexpr int LevelManager::MAX_LEVEL;

LevelManager::LevelManager(const Config& base_config)
    : base_config_(base_config),
      effective_spawn_interval_(base_config.getSpawnInterval()),
      effective_obstacle_speed_(base_config.getObstacleSpeed()),
      effective_grow_chance_(base_config.getGrowChance()),
      effective_shrink_chance_(base_config.getShrinkChance()),
      effective_hurt_chance_(base_config.getHurtChance()),
      effective_base_checkpoint_gap_(base_config.getBaseCheckpointGap()),
      effective_checkpoints_per_level_(base_config.getCheckpointsPerLevel())
{
    updateForLevel(1);
}

void LevelManager::updateForLevel(int level) {
    const LevelConfig* level_cfg = base_config_.getLevelConfig(level);
    if (!level_cfg) {
        // If no specific config for this level, potentially fallback to last known or base.
        // For now, we just use the existing effective values which are from the previous level or base.
        return;
    }

    effective_spawn_interval_ = level_cfg->spawn_interval_ms.value_or(effective_spawn_interval_);
    effective_obstacle_speed_ = level_cfg->obstacle_speed.value_or(effective_obstacle_speed_);
    effective_grow_chance_ = level_cfg->grow_chance_percent.value_or(effective_grow_chance_);
    effective_shrink_chance_ = level_cfg->shrink_chance_percent.value_or(effective_shrink_chance_);
    effective_base_checkpoint_gap_ = level_cfg->base_checkpoint_gap.value_or(effective_base_checkpoint_gap_);
    effective_checkpoints_per_level_ = level_cfg->checkpoints_per_level.value_or(effective_checkpoints_per_level_);
    // Note: We assume the chances in the config sum to 100. The base config validates this,
    // but per-level overrides currently do not. The logic in determineObstacleType handles it gracefully.
    effective_hurt_chance_ = level_cfg->hurt_chance_percent.value_or(effective_hurt_chance_);
}