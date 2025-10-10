#include "GameState.h"

GameState::GameState(const Config& config, int screen_width, int screen_height)
    : config(config),
      player(config.getPlayerInitialX(),                               // initial x position
             screen_height / 2 - config.getPlayerHeight() / 2,         // initial y position (centered)
             config.getPlayerWidth(),                                  // width
             config.getPlayerHeight(),                                 // height
             config.getPlayerSpeed(),                                  // speed
             config.getPlayerColor()),                                 // color
      level_manager(config),                                          // must be initialized before spawner
      spawner(level_manager,
              config.getCheckpointInterval(),
              config.getCheckpointSafeZoneDuration(),
              screen_width,                                           // screen width
              screen_height,                                          // screen height
              config.getPlayerSizeChangeAmount(),
              config.getGrowDimensions(),                             // dimensions for grow blocks
              config.getShrinkDimensions(),                           // dimensions for shrink blocks
              config.getHurtDimensions()
             ),
      last_fps_update_time(SDL_GetTicks())                            // initialize FPS timer
{
    next_checkpoint_gap_size = spawner.calculateCheckpointGapSize();
}