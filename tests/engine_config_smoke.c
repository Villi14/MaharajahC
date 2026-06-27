#include <stdio.h>
#include <string.h>

#include "../include/engine/EngineConfig.h"
#include "../include/engine/EvalContext.h"
#include "../include/engine/Globals.h"
#include "../include/engine/NNUE.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  return 1;
}

int main(void) {
  const unsigned char memory_weights[] = { 0x4e, 0x4e, 0x55, 0x45 };

  reset_engine_config();
  init_nnue();
  reset_eval_context();

  if (engine_search_config.ui_difficulty != 5 || engine_search_config.skill_level != 10)
    return fail("engine_config_smoke failed: reset_engine_config did not restore full-strength UI defaults.");
  if (engine_search_config.max_depth_cap != max_ply)
    return fail("engine_config_smoke failed: full-strength profile did not restore max depth.");
  if (engine_eval_config.eval_mode != eval_mode_classic)
    return fail("engine_config_smoke failed: reset_engine_config did not restore classic eval mode.");

  set_engine_ui_difficulty(0);
  if (engine_search_config.ui_difficulty != 1 || engine_search_config.skill_level != 2)
    return fail("engine_config_smoke failed: UI difficulty lower clamp or mapping was wrong.");

  set_engine_ui_difficulty(99);
  if (engine_search_config.ui_difficulty != 5 || engine_search_config.skill_level != 10)
    return fail("engine_config_smoke failed: UI difficulty upper clamp or mapping was wrong.");

  set_engine_skill_level(0);
  if (engine_search_config.ui_difficulty != 0 || engine_search_config.skill_level != 1)
    return fail("engine_config_smoke failed: skill level lower clamp was wrong.");
  if (engine_search_config.max_depth_cap != 1)
    return fail("engine_config_smoke failed: weakest skill profile did not apply its search cap.");

  set_engine_skill_level(99);
  if (engine_search_config.ui_difficulty != 0 || engine_search_config.skill_level != 10)
    return fail("engine_config_smoke failed: skill level upper clamp was wrong.");
  if (engine_search_config.max_depth_cap != max_ply)
    return fail("engine_config_smoke failed: strongest skill profile did not restore max depth.");

  set_engine_eval_mode(99);
  if (engine_eval_config.eval_mode != eval_mode_classic)
    return fail("engine_config_smoke failed: invalid eval mode did not clamp to classic.");

  set_engine_eval_mode(eval_mode_nnue);
  update_eval_context();
  if (eval_context.requested_eval_mode != eval_mode_nnue)
    return fail("engine_config_smoke failed: requested eval mode was not tracked.");
  if (eval_context.active_eval_mode != eval_mode_classic)
    return fail("engine_config_smoke failed: active eval mode should stay classic before NNUE backend is ready.");
  if (!eval_context.using_classic_fallback)
    return fail("engine_config_smoke failed: classic fallback flag was not enabled for staged NNUE mode.");
  if (eval_context.weights_loaded)
    return fail("engine_config_smoke failed: weights should not be marked loaded before any load call.");

  if (!nnue_load_weights_from_bytes(memory_weights, sizeof(memory_weights), "memory-v1"))
    return fail("engine_config_smoke failed: bytes weights load failed.");

  update_eval_context();
  if (!eval_context.weights_loaded)
    return fail("engine_config_smoke failed: weights load was not reflected in eval context.");
  if (eval_context.nnue_backend_ready)
    return fail("engine_config_smoke failed: NNUE backend should not report ready before inference exists.");
  if (!eval_context.using_classic_fallback)
    return fail("engine_config_smoke failed: staged NNUE mode should still report classic fallback after weights load.");
  if (strcmp(eval_context.weights_version, "memory-v1") != 0)
    return fail("engine_config_smoke failed: eval context did not preserve weights version.");
  if (strcmp(eval_context.loaded_path, "<memory>") != 0)
    return fail("engine_config_smoke failed: eval context did not preserve memory path marker.");

  nnue_unload_weights();
  update_eval_context();
  if (eval_context.weights_loaded)
    return fail("engine_config_smoke failed: weights load flag was not cleared after unload.");
  if (!eval_context.using_classic_fallback)
    return fail("engine_config_smoke failed: fallback flag should remain active while NNUE is requested.");

  return 0;
}
