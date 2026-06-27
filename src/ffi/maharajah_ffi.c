#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/board/Fen.h"
#include "../../include/engine/CustomSetup.h"
#include "../../include/engine/EngineConfig.h"
#include "../../include/engine/EvalContext.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Inits.h"
#include "../../include/engine/Moves.h"
#include "../../include/engine/NNUE.h"
#include "../../include/engine/Search.h"
#include "../../include/engine/Transposition.h"
#include "../../include/ffi/maharajah_ffi.h"
#include "../../include/parse/Parse.h"
#include "../../include/perft/Perft.h"
#include "../../include/util/Defines.h"
#include "../../include/util/Utils.h"

static int engine_initialized = 0;

static int move_to_coords(int move, char *out_move, int out_len) {
  if (out_move == NULL || out_len <= 0)
    return 0;

  if (get_move_promoted(move)) {
    if (out_len < 6)
      return 0;
  } else if (out_len < 5) {
    return 0;
  }

  const char *source = square_to_coordinates[get_move_source(move)];
  const char *target = square_to_coordinates[get_move_target(move)];

  out_move[0] = source[0];
  out_move[1] = source[1];
  out_move[2] = target[0];
  out_move[3] = target[1];

  if (get_move_promoted(move)) {
    out_move[4] = promoted_pieces[get_move_promoted(move)];
    out_move[5] = '\0';
  } else {
    out_move[4] = '\0';
  }

  return 1;
}

static int copy_out_string(const char *value, char *out, int out_len) {
  if (value == NULL || out == NULL || out_len <= 0)
    return 0;

  const size_t required_len = strlen(value) + 1;
  if ((size_t)out_len < required_len)
    return 0;

  memcpy(out, value, required_len);
  return 1;
}

static const char *eval_mode_name(int eval_mode) {
  return eval_mode == eval_mode_nnue ? "nnue" : "classic";
}

static const char *eval_status_string(void) {
  update_eval_context();

  if (eval_context.active_eval_mode == eval_mode_nnue)
    return "nnue_ready";

  if (eval_context.requested_eval_mode != eval_mode_nnue)
    return "classic_ready";

  if (eval_context.weights_loaded)
    return "nnue_stub_fallback";

  /* Requested NNUE, active still classic, no loaded weights: staged / offline. */
  return "classic_ready";
}

static int search_best_move_ffi(int depth) {
  const int previous_stdin_polling_enabled =
      time_controls.stdin_polling_enabled;
  time_controls.stdin_polling_enabled = 0;
  const int best_move = search_root(depth, 0);
  time_controls.stdin_polling_enabled = previous_stdin_polling_enabled;
  return best_move;
}

static void set_time_limit(int movetime_ms) {
  reset_time_control();
  if (movetime_ms <= 0) {
    time_controls.timeset = 0;
    return;
  }

  time_controls.movetime = movetime_ms;
  time_controls.uci_time = movetime_ms;
  time_controls.movestogo = 1;
  time_controls.starttime = get_time_ms();
  time_controls.timeset = 1;
  time_controls.stoptime = time_controls.starttime + movetime_ms;
}

static void apply_rules_profile(int rules_profile) {
  if (rules_profile == mah_rules_variant) {
    board.standard_rules = 0;
    board.castle = 0;
    board.hash_key = generate_hash_key();
    return;
  }

  if (rules_profile == mah_rules_standard && !board_has_compound_pieces()) {
    board.standard_rules = 1;
    board.hash_key = generate_hash_key();
  }
}

FFI_PLUGIN_EXPORT int mah_init(void) {
  if (!engine_initialized) {
    init_all();
    engine_initialized = 1;
  }

  reset_engine_config();
  parse_fen(start_position);
  clear_hash_table();
  return 1;
}

FFI_PLUGIN_EXPORT int mah_set_position_fen(const char *fen) {
  return mah_set_position_fen_with_rules(fen, mah_rules_auto);
}

FFI_PLUGIN_EXPORT int mah_set_position_fen_with_rules(const char *fen,
                                                      int rules_profile) {
  if (fen == NULL || *fen == '\0')
    return 0;
  if (!engine_initialized)
    mah_init();

  parse_fen(fen);
  apply_rules_profile(rules_profile);
  clear_hash_table();
  return 1;
}

FFI_PLUGIN_EXPORT int mah_set_position_startpos(void) {
  if (!engine_initialized)
    mah_init();
  parse_fen(start_position);
  clear_hash_table();
  return 1;
}

FFI_PLUGIN_EXPORT int mah_apply_move(const char *move) {
  if (move == NULL || *move == '\0')
    return 0;
  if (!engine_initialized)
    mah_init();

  int parsed_move = parse_move(move);
  if (parsed_move == 0)
    return 0;

  ++search_context.repetition_index;
  search_context.repetition_table[search_context.repetition_index] =
      board.hash_key;

  if (make_move(parsed_move, all_moves) == 0) {
    --search_context.repetition_index;
    return 0;
  }

  return 1;
}

FFI_PLUGIN_EXPORT int mah_best_move_depth(int depth, char *out_move,
                                          int out_len) {
  if (!engine_initialized)
    mah_init();

  if (depth <= 0)
    depth = 1;

  reset_time_control();
  int best_move = search_best_move_ffi(depth);
  if (best_move == 0)
    return 0;

  return move_to_coords(best_move, out_move, out_len);
}

FFI_PLUGIN_EXPORT int mah_best_move_time(int movetime_ms, char *out_move,
                                         int out_len) {
  if (!engine_initialized)
    mah_init();

  set_time_limit(movetime_ms);
  int best_move = search_best_move_ffi(max_ply);
  if (best_move == 0)
    return 0;

  return move_to_coords(best_move, out_move, out_len);
}

FFI_PLUGIN_EXPORT int mah_generate_custom_position_fen(int side_to_move,
                                                       unsigned int seed,
                                                       char *out_fen,
                                                       int out_len) {
  if (out_fen == NULL || out_len <= 0)
    return 0;
  if (!engine_initialized)
    mah_init();

  if (!generate_reasonable_custom_position(side_to_move, seed))
    return 0;

  return custom_position_board_to_fen(out_fen, out_len);
}

FFI_PLUGIN_EXPORT int mah_set_hash_mb(int mb) {
  if (!engine_initialized)
    mah_init();
  if (mb < 4)
    mb = 4;
  init_hash_table(mb);
  return 1;
}

FFI_PLUGIN_EXPORT int mah_set_skill_level(int skill_level) {
  if (!engine_initialized)
    mah_init();

  set_engine_skill_level(skill_level);
  return 1;
}

FFI_PLUGIN_EXPORT int mah_set_difficulty_level(int difficulty_level) {
  if (!engine_initialized)
    mah_init();

  set_engine_ui_difficulty(difficulty_level);
  return 1;
}

FFI_PLUGIN_EXPORT int mah_set_eval_mode(int eval_mode) {
  if (!engine_initialized)
    mah_init();

  set_engine_eval_mode(eval_mode);
  update_eval_context();
  return 1;
}

FFI_PLUGIN_EXPORT int mah_load_weights(const char *path) {
  if (!engine_initialized)
    mah_init();

  const int loaded = nnue_load_weights(path);
  update_eval_context();
  return loaded;
}

FFI_PLUGIN_EXPORT int
mah_load_weights_from_bytes(const unsigned char *bytes, int len,
                            const char *weights_version_name) {
  if (!engine_initialized)
    mah_init();
  if (len <= 0)
    return 0;

  const int loaded =
      nnue_load_weights_from_bytes(bytes, (size_t)len, weights_version_name);
  update_eval_context();
  return loaded;
}

FFI_PLUGIN_EXPORT int mah_unload_weights(void) {
  if (!engine_initialized)
    mah_init();

  nnue_unload_weights();
  update_eval_context();
  return 1;
}

FFI_PLUGIN_EXPORT int mah_get_eval_status(char *out_status, int out_len) {
  if (!engine_initialized)
    mah_init();

  return copy_out_string(eval_status_string(), out_status, out_len);
}

FFI_PLUGIN_EXPORT int mah_get_weights_version(char *out_version, int out_len) {
  if (!engine_initialized)
    mah_init();

  return copy_out_string(nnue_weights_version(), out_version, out_len);
}

FFI_PLUGIN_EXPORT int mah_get_nnue_info(char *out_info, int out_len) {
  if (!engine_initialized)
    mah_init();
  if (out_info == NULL || out_len <= 0)
    return 0;

  update_eval_context();
  const char *status = eval_status_string();

  const int written = snprintf(
      out_info, (size_t)out_len,
      "{\"requestedEvalMode\":\"%s\",\"activeEvalMode\":\"%s\",\"status\":\"%"
      "s\",\"weightsVersion\":\"%s\",\"loadedPath\":\"%s\",\"weightsLoaded\":%"
      "d,\"nnueBackendReady\":%d,\"usingClassicFallback\":%d,\"uiDifficulty\":%"
      "d,\"skillLevel\":%d,\"accumulatorInitialized\":%d}",
      eval_mode_name(eval_context.requested_eval_mode),
      eval_mode_name(eval_context.active_eval_mode), status,
      eval_context.weights_version, eval_context.loaded_path,
      eval_context.weights_loaded, eval_context.nnue_backend_ready,
      eval_context.using_classic_fallback, engine_search_config.ui_difficulty,
      engine_search_config.skill_level, eval_context.accumulator_initialized);
  if (written < 0 || written >= out_len)
    return 0;
  return 1;
}

FFI_PLUGIN_EXPORT int mah_shutdown(void) {
  if (transposition_table.table != NULL) {
    free(transposition_table.table);
    transposition_table.table = NULL;
    transposition_table.entries = 0;
  }

  reset_board();
  reset_time_control();
  reset_engine_config();
  reset_eval_context();
  engine_initialized = 0;
  return 1;
}
