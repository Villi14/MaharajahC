#include "../../include/engine/EvalContext.h"

#include <stdio.h>

#include "../../include/engine/EngineConfig.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/NNUE.h"

EvalContext eval_context = {
  .requested_eval_mode = eval_mode_classic,
  .active_eval_mode = eval_mode_classic,
  .side_to_move = white,
  .weights_loaded = 0,
  .nnue_backend_ready = 0,
  .using_classic_fallback = 0,
  .accumulator_initialized = 0,
  .weights_version = {0},
  .loaded_path = {0}
};

void reset_eval_context(void) {
  eval_context.requested_eval_mode = eval_mode_classic;
  eval_context.active_eval_mode = eval_mode_classic;
  eval_context.side_to_move = white;
  eval_context.weights_loaded = 0;
  eval_context.nnue_backend_ready = 0;
  eval_context.using_classic_fallback = 0;
  eval_context.accumulator_initialized = 0;
  eval_context.weights_version[0] = '\0';
  eval_context.loaded_path[0] = '\0';
}

void update_eval_context(void) {
  const int requested_eval_mode = clamp_eval_mode(engine_eval_config.eval_mode);
  const int weights_loaded = nnue_has_loaded_weights();
  const int nnue_backend_ready = nnue_backend_is_ready();

  eval_context.requested_eval_mode = requested_eval_mode;
  eval_context.active_eval_mode =
      requested_eval_mode == eval_mode_nnue && nnue_backend_ready
          ? eval_mode_nnue
          : eval_mode_classic;
  eval_context.side_to_move = board.side;
  eval_context.weights_loaded = weights_loaded;
  eval_context.nnue_backend_ready = nnue_backend_ready;
  eval_context.using_classic_fallback =
      requested_eval_mode == eval_mode_nnue && eval_context.active_eval_mode != eval_mode_nnue;
  eval_context.accumulator_initialized = 0;
  snprintf(eval_context.weights_version, sizeof(eval_context.weights_version), "%s",
           nnue_weights_version());
  snprintf(eval_context.loaded_path, sizeof(eval_context.loaded_path), "%s",
           nnue_loaded_path());
}
