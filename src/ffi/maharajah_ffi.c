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

// FEN piece characters indexed by the engine piece enum
// (P, N, B, R, Q, A, C, M, K, p, n, b, r, q, a, c, m, k).
static const char fen_piece_chars[piece_count] = {
    'P', 'N', 'B', 'R', 'Q', 'A', 'C', 'M', 'K',
    'p', 'n', 'b', 'r', 'q', 'a', 'c', 'm', 'k'};

// Serialize the live board position to a FEN with the six standard fields
// plus field 7 (per-side variant rights, always emitted) and field 8
// (unmoved-pawn squares, emitted only when the engine carries per-pawn state)
// so a get_fen -> set_fen round-trip is lossless. Squares are indexed
// rank-major from a8 (square 0) to h1 (square 63), matching parse_fen. The
// fullmove number is not tracked by the engine and is supplied by the caller.
static int board_to_full_fen(char *out, int out_len, int fullmove_number) {
  if (out == NULL || out_len <= 0)
    return 0;

  size_t offset = 0;
  const size_t capacity = (size_t)out_len;
  out[0] = '\0';

#define FEN_APPEND(...)                                                        \
  do {                                                                         \
    if (offset >= capacity)                                                    \
      return 0;                                                                \
    const int written =                                                        \
        snprintf(out + offset, capacity - offset, __VA_ARGS__);                \
    if (written < 0 || (size_t)written >= capacity - offset)                   \
      return 0;                                                                \
    offset += (size_t)written;                                                 \
  } while (0)

  for (int rank = 0; rank < 8; ++rank) {
    int empty = 0;
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      int piece = -1;
      for (int bb = P; bb <= k; ++bb) {
        if (get_bit(board.bitboards[bb], square)) {
          piece = bb;
          break;
        }
      }

      if (piece < 0) {
        ++empty;
        continue;
      }

      if (empty > 0) {
        FEN_APPEND("%d", empty);
        empty = 0;
      }
      FEN_APPEND("%c", fen_piece_chars[piece]);
    }

    if (empty > 0)
      FEN_APPEND("%d", empty);
    if (rank < 7)
      FEN_APPEND("/");
  }

  FEN_APPEND(" %c ", board.side == white ? 'w' : 'b');

  if (board.castle == 0) {
    FEN_APPEND("-");
  } else {
    if (board.castle & wk)
      FEN_APPEND("K");
    if (board.castle & wq)
      FEN_APPEND("Q");
    if (board.castle & bk)
      FEN_APPEND("k");
    if (board.castle & bq)
      FEN_APPEND("q");
  }

  if (board.enpassant != no_sq)
    FEN_APPEND(" %s", square_to_coordinates[board.enpassant]);
  else
    FEN_APPEND(" -");

  if (fullmove_number < 1)
    fullmove_number = 1;
  FEN_APPEND(" %d %d", board.halfmove_clock, fullmove_number);

  // Field 7: per-side variant rights (V = White variant, v = Black variant,
  // - = both classic). Lets a mixed game round-trip its per-side promotion
  // rules through the FEN; see parse_fen.
  FEN_APPEND(" ");
  if (board.side_variant[white])
    FEN_APPEND("V");
  if (board.side_variant[black])
    FEN_APPEND("v");
  if (!board.side_variant[white] && !board.side_variant[black])
    FEN_APPEND("-");

  // Field 8: unmoved-pawn squares (see parse_fen / Board.pawn_unmoved).
  // Emitted only when the engine actually carries per-pawn state, so legacy
  // 7-field output is unchanged for callers that never sent field 8. Masking
  // with the live pawns keeps squares whose pawn is long gone out of the
  // round-trip.
  if (board.has_pawn_state) {
    const u64 unmoved_pawns =
        board.pawn_unmoved & (board.bitboards[P] | board.bitboards[p]);
    FEN_APPEND(" ");
    if (unmoved_pawns == 0ULL) {
      FEN_APPEND("-");
    } else {
      u64 remaining = unmoved_pawns;
      while (remaining) {
        const int square = get_ls1b_index(remaining);
        FEN_APPEND("%s", square_to_coordinates[square]);
        pop_bit(remaining, square);
      }
    }
  }

#undef FEN_APPEND

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
  // mah_rules_auto leaves whatever parse_fen derived in place, so the optional
  // per-side FEN field (side_variant) is authoritative — this is the path a
  // mixed game uses. The explicit profiles pin a whole-game nature on both
  // sides regardless of current material (e.g. a variant game that has lost all
  // its compound pieces stays variant).
  if (rules_profile == mah_rules_variant) {
    board.standard_rules = 0;
    board.castle = 0;
    board.side_variant[white] = 1;
    board.side_variant[black] = 1;
    board.hash_key = generate_hash_key();
    return;
  }

  if (rules_profile == mah_rules_standard && !board_has_compound_pieces()) {
    board.standard_rules = 1;
    board.side_variant[white] = 0;
    board.side_variant[black] = 0;
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

  // Every downstream path (make_move legality, mah_game_status, search)
  // locates the kings with get_ls1b_index, which is undefined on an empty
  // bitboard — so a FEN without exactly one king per side is rejected and a
  // safe startpos is left behind.
  if (count_bits(board.bitboards[K]) != 1 ||
      count_bits(board.bitboards[k]) != 1) {
    parse_fen(start_position);
    clear_hash_table();
    return 0;
  }

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

FFI_PLUGIN_EXPORT int mah_get_fen(char *out_fen, int out_len,
                                  int fullmove_number) {
  if (!engine_initialized)
    mah_init();

  return board_to_full_fen(out_fen, out_len, fullmove_number);
}

FFI_PLUGIN_EXPORT int mah_game_status(void) {
  if (!engine_initialized)
    mah_init();

  // The side to move has a legal move iff some pseudo-legal move survives
  // make_move (which self-restores and returns 0 when a move would leave its
  // own king in check). Probe each with a snapshot/restore around it.
  MoveList move_list;
  generate_moves(&move_list);
  for (int i = 0; i < move_list.count; ++i) {
    copy_board();
    if (make_move(move_list.moves[i], all_moves) != 0) {
      take_back();
      return mah_status_ongoing;
    }
    take_back();
  }

  // No legal reply: checkmate if the side to move sits in check, else stalemate.
  const int side = board.side;
  const u64 king_bb =
      (side == white) ? board.bitboards[K] : board.bitboards[k];
  // Unreachable through the FFI (set-position validates both kings), but a
  // kingless mover is a lost/invalid position, not UB.
  if (king_bb == 0ULL)
    return mah_status_checkmate;
  const int king_square = get_ls1b_index(king_bb);
  if (is_square_attacked(king_square, side ^ 1))
    return mah_status_checkmate;
  return mah_status_stalemate;
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
  if (mb > 1024)
    mb = 1024;
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
