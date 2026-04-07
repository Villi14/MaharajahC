#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "maharajah/board/Fen.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Inits.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/engine/Search.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/ffi/EngineFFI.h"
#include "maharajah/parse/Parse.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/util/Defines.h"
#include "maharajah/util/Utils.h"

static int engine_initialized = 0;

static int move_to_coords(int move, char* out_move, int out_len) {
  if (out_move == NULL || out_len <= 0)
    return 0;

  if (get_move_promoted(move)) {
    if (out_len < 6)
      return 0;
  } else if (out_len < 5) {
    return 0;
  }

  const char* source = square_to_coordinates[get_move_source(move)];
  const char* target = square_to_coordinates[get_move_target(move)];

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

static int search_best_move(int depth) {
  int score = 0;
  search_context.nodes = 0;
  time_controls.stopped = 0;
  search_context.follow_pv = 0;
  search_context.score_pv = 0;
  search_context.ply = 0;

  memset(search_context.killer_moves, 0, sizeof(search_context.killer_moves));
  memset(search_context.history_moves, 0, sizeof(search_context.history_moves));
  memset(search_context.pv_table, 0, sizeof(search_context.pv_table));
  memset(search_context.pv_length, 0, sizeof(search_context.pv_length));

  int alpha = -infinity;
  int beta = infinity;
  const int previous_stdin_polling_enabled = time_controls.stdin_polling_enabled;
  time_controls.stdin_polling_enabled = 0;

  for (int current_depth = 1; current_depth <= depth; ++current_depth) {
    if (time_controls.stopped == 1)
      break;
    search_context.follow_pv = 1;
    score = negamax(alpha, beta, current_depth);

    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;
      continue;
    }

    alpha = score - 50;
    beta = score + 50;
  }

  time_controls.stdin_polling_enabled = previous_stdin_polling_enabled;
  return search_context.pv_table[0][0];
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

int mah_init(void) {
  if (!engine_initialized) {
    init_all();
    engine_initialized = 1;
  }

  parse_fen(start_position);
  clear_hash_table();
  return 1;
}

int mah_set_position_fen(const char* fen) {
  if (fen == NULL || *fen == '\0')
    return 0;
  if (!engine_initialized)
    mah_init();

  parse_fen(fen);
  clear_hash_table();
  return 1;
}

int mah_set_position_startpos(void) {
  if (!engine_initialized)
    mah_init();
  parse_fen(start_position);
  clear_hash_table();
  return 1;
}

int mah_apply_move(const char* move) {
  if (move == NULL || *move == '\0')
    return 0;
  if (!engine_initialized)
    mah_init();

  int parsed_move = parse_move(move);
  if (parsed_move == 0)
    return 0;

  ++search_context.repetition_index;
  search_context.repetition_table[search_context.repetition_index] = board.hash_key;

  if (make_move(parsed_move, all_moves) == 0) {
    --search_context.repetition_index;
    return 0;
  }

  return 1;
}

int mah_best_move_depth(int depth, char* out_move, int out_len) {
  if (!engine_initialized)
    mah_init();

  if (depth <= 0)
    depth = 1;

  reset_time_control();
  int best_move = search_best_move(depth);
  if (best_move == 0)
    return 0;

  return move_to_coords(best_move, out_move, out_len);
}

int mah_best_move_time(int movetime_ms, char* out_move, int out_len) {
  if (!engine_initialized)
    mah_init();

  set_time_limit(movetime_ms);
  int best_move = search_best_move(max_ply);
  if (best_move == 0)
    return 0;

  return move_to_coords(best_move, out_move, out_len);
}

int mah_set_hash_mb(int mb) {
  if (!engine_initialized)
    mah_init();
  if (mb < 4)
    mb = 4;
  init_hash_table(mb);
  return 1;
}

int mah_shutdown(void) {
  if (transposition_table.table != NULL) {
    free(transposition_table.table);
    transposition_table.table = NULL;
    transposition_table.entries = 0;
  }

  reset_board();
  reset_time_control();
  engine_initialized = 0;
  return 1;
}
