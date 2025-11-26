#include "GameState.h"

GameState::GameState(const Config& config, int screen_width, int screen_height)
    : config(config),
      player(config.getPlayerInitialX(),
             screen_height / 2 - config.getPlayerHeight() / 2,
             config.getPlayerWidth(),
             config.getPlayerHeight(),
             config.getPlayerSpeed(),
             config.getPlayerColor(),
             config.getDashSpeedMultiplier(),
             config.getDashDurationMs(),
             config.getDashCooldownMs()),
      level_manager(config),                                          // must be initialized before spawner
      score_manager(config),
      spawner(level_manager,
              config.getCheckpointSafeZoneDuration(),
              screen_width,                                           // screen width
              screen_height,                                          // screen height
              config.getPlayerSizeChangeAmount(),
              SDL_GetTicks()),                                        // start time
      last_fps_update_time(SDL_GetTicks())                            // initialize FPS timer
    {
        ui_next_checkpoint_gap_size = spawner.calculateCheckpointGapSize();
        next_checkpoint_gap_size = spawner.calculateCheckpointGapSize();
}