#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/board/Prints.h"
#include "../../include/engine/Attacks.h"
#include "../../include/engine/EngineConfig.h"
#include "../../include/engine/Evaluate.h"
#include "../../include/engine/FindMagics.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Moves.h"
#include "../../include/engine/Search.h"
#include "../../include/engine/See.h"
#include "../../include/engine/Transposition.h"
#include "../../include/engine/Zobrist.h"
#include "../../include/perft/Perft.h"
#include "../../include/uci/UCI.h"
#include "../../include/util/Defines.h"
#include "../../include/util/Utils.h"

static int count_standard_material(int piece) {
  return count_bits(board.bitboards[piece]);
}

static void reset_search_state(void) {
  search_context.nodes = 0;
  time_controls.stopped = 0;
  search_context.follow_pv = 0;
  search_context.score_pv = 0;
  search_context.ply = 0;
  search_context.root_count = 0;

  memset(search_context.killer_moves, 0, sizeof(search_context.killer_moves));
  memset(search_context.history_moves, 0, sizeof(search_context.history_moves));
  memset(search_context.pv_table, 0, sizeof(search_context.pv_table));
  memset(search_context.pv_length, 0, sizeof(search_context.pv_length));
  memset(search_context.root_moves, 0, sizeof(search_context.root_moves));
  memset(search_context.root_scores, 0, sizeof(search_context.root_scores));
}

static int effective_depth_for_skill(int depth) {
  const int cap = engine_search_config.max_depth_cap;

  if (depth <= 0)
    return 1;
  if (cap == max_ply || depth < cap)
    return depth;
  return cap;
}

/* Same terminal-draw rule as the prior duplicated condition in negamax / quiescence. */
static int should_return_draw_score(void) {
  if (search_context.ply == 0)
    return 0;
  return is_threefold_repetition() || is_fivefold_repetition() || is_fifty_move_rule() ||
         is_seventy_five_move_rule() || is_insufficient_material();
}

static void sort_root_moves(void) {
  for (int i = 1; i < search_context.root_count; ++i) {
    const int s = search_context.root_scores[i];
    const int m = search_context.root_moves[i];
    int j = i;
    while (j > 0 && search_context.root_scores[j - 1] < s) {
      search_context.root_scores[j] = search_context.root_scores[j - 1];
      search_context.root_moves[j] = search_context.root_moves[j - 1];
      --j;
    }
    search_context.root_scores[j] = s;
    search_context.root_moves[j] = m;
  }
}

static int verify_root_candidate_score(int move) {
  copy_board();

  const int saved_ply = search_context.ply;
  const int saved_repetition_index = search_context.repetition_index;
  const int saved_follow_pv = search_context.follow_pv;
  const int saved_score_pv = search_context.score_pv;
  int score = -infinity;

  ++search_context.ply;
  ++search_context.repetition_index;
  search_context.repetition_table[search_context.repetition_index] =
      board.hash_key;

  if (make_move(move, all_moves) != 0) {
    search_context.follow_pv = 0;
    search_context.score_pv = 0;
    score = -negamax(-infinity, infinity, 1);
  }

  search_context.ply = saved_ply;
  search_context.repetition_index = saved_repetition_index;
  search_context.follow_pv = saved_follow_pv;
  search_context.score_pv = saved_score_pv;
  take_back();

  return score;
}

int root_move_repeats_position(int move) {
  copy_board();

  const int saved_ply = search_context.ply;
  const int saved_repetition_index = search_context.repetition_index;
  int repeats = 0;

  ++search_context.ply;
  ++search_context.repetition_index;
  search_context.repetition_table[search_context.repetition_index] =
      board.hash_key;

  if (make_move(move, all_moves) != 0)
    repeats = is_repetition();

  search_context.ply = saved_ply;
  search_context.repetition_index = saved_repetition_index;
  take_back();

  return repeats;
}

static int select_non_repeating_root_move(int best_move) {
  if (!root_move_repeats_position(best_move))
    return best_move;

  const int best_verified_score = verify_root_candidate_score(best_move);
  const int anti_repeat_margin = 150;
  int fallback_move = 0;
  int fallback_verified_score = -infinity;

  for (int index = 0; index < search_context.root_count; ++index) {
    const int candidate_move = search_context.root_moves[index];
    if (candidate_move == best_move || root_move_repeats_position(candidate_move))
      continue;

    const int verified_score = verify_root_candidate_score(candidate_move);
    if (verified_score > fallback_verified_score) {
      fallback_verified_score = verified_score;
      fallback_move = candidate_move;
    }
  }

  if (fallback_move != 0 &&
      fallback_verified_score >= best_verified_score - anti_repeat_margin) {
    return fallback_move;
  }

  return best_move;
}

static int select_skill_move(void) {
  if (search_context.root_count == 0)
    return 0;

  sort_root_moves();

  const int best_move = select_non_repeating_root_move(search_context.root_moves[0]);
  const int skill_level = engine_search_config.skill_level;

  if (skill_level >= 10)
    return best_move;

  if (search_context.repetition_index >= engine_search_config.opening_variety_plies)
    return best_move;

  const int score_margin = engine_search_config.weak_move_margin_cp;
  int candidate_limit = engine_search_config.weak_move_candidates;
  if (candidate_limit > search_context.root_count)
    candidate_limit = search_context.root_count;

  int eligible_count = 1;
  const int best_score = search_context.root_scores[0];
  while (eligible_count < candidate_limit &&
         search_context.root_scores[eligible_count] >= best_score - score_margin) {
    ++eligible_count;
  }

  if (eligible_count <= 1)
    return best_move;

  int candidate_moves[max_moves];
  int candidate_count = 1;
  candidate_moves[0] = best_move;

  const int best_verified_score = verify_root_candidate_score(best_move);
  const int tactical_margin = 120;

  for (int index = 1; index < eligible_count; ++index) {
    const int candidate_move = search_context.root_moves[index];
    const int verified_score = verify_root_candidate_score(candidate_move);

    if (verified_score >= best_verified_score - tactical_margin)
      candidate_moves[candidate_count++] = candidate_move;
  }

  if (candidate_count <= 1)
    return best_move;

  const unsigned int roll = get_random_U32_number();
  const int best_move_bias = skill_level + 1;
  const int choice_pool = candidate_count + best_move_bias - 1;
  const int weighted_index = (int)(roll % choice_pool);
  const int selected_index =
      weighted_index < candidate_count ? weighted_index : 0;
  return candidate_moves[selected_index];
}


int search_root(int depth, int emit_info) {
  int start = get_time_ms();
  int score = 0;
  int best_move = 0;
  reset_search_state();

  int alpha = -infinity;
  int beta = infinity;
  const int effective_depth = effective_depth_for_skill(depth);

  for (int current_depth = 1; current_depth <= effective_depth; ++current_depth) {
    if (time_controls.stopped == 1)
      break;
    search_context.follow_pv = 1;
    score = negamax(alpha, beta, current_depth);

    if (time_controls.stopped == 1)
      break;

    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;
      continue;
    }

    alpha = score - 50;
    beta = score + 50;

    if (emit_info && search_context.pv_length[0]) {
      if (score > -mate_value && score < -mate_score)
        printf("info score mate %d depth %d nodes %llu time %d pv ",
               -(score + mate_value) / 2 - 1,
               current_depth,
               (u64)search_context.nodes,
               get_time_ms() - start);
      else if (score > mate_score && score < mate_value)
        printf("info score mate %d depth %d nodes %llu time %d pv ",
               (mate_value - score) / 2 + 1,
               current_depth,
               (u64)search_context.nodes,
               get_time_ms() - start);
      else
        printf("info score cp %d depth %d nodes %llu time %d pv ", score, current_depth, (u64)search_context.nodes, get_time_ms() - start);

      for (int count = 0; count < search_context.pv_length[0]; ++count) {
        print_move(search_context.pv_table[0][count]);
        printf(" ");
      }
      printf("\n");
    }

    if (search_context.root_count > 0)
      best_move = select_skill_move();
  }

  return best_move;
}

void search_position(int depth) {
  const int best_move = search_root(depth, 1);
  printf("bestmove ");
  if (best_move == 0)
    printf("(none)");
  else
    print_move(best_move);
  printf("\n");
}

int negamax(int alpha, int beta, int depth) {
  search_context.pv_length[search_context.ply] = search_context.ply;
  if (search_context.ply == 0)
    search_context.root_count = 0;

  int score;
  int hash_flag = hash_flag_alpha;
  const u64 own_king = (board.side == white) ? board.bitboards[K] : board.bitboards[k];

  if (should_return_draw_score())
    return 0;

  int pv_node = beta - alpha > 1;
  if (search_context.ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && pv_node == 0)
    return score;

  if ((search_context.nodes & 2047) == 0)
    communicate();

  if (depth == 0)
    return quiescence(alpha, beta);

  if (search_context.ply > max_ply - 1)
    return evaluate();

  if (own_king == 0ULL)
    return -mate_value + search_context.ply;

  ++search_context.nodes;

  int in_check = is_square_attacked(get_ls1b_index(own_king), board.side ^ 1);
  int static_eval = 0;
  int has_static_eval = 0;

  if (in_check)
    ++depth;
  else {
    static_eval = evaluate();
    has_static_eval = 1;
  }

  if (depth <= 2 && search_context.ply && in_check == 0 && pv_node == 0) {
    const int reverse_futility_margin =
        engine_search_config.reverse_futility_margin_per_depth * depth;
    if (static_eval - reverse_futility_margin >= beta)
      return beta;
  }

  int legal_moves = 0;

  if (depth >= 3 && in_check == 0 && search_context.ply) {
    copy_board();
    ++search_context.ply;
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    if (board.enpassant != no_sq)
      board.hash_key ^= zobrist_keys.enpassant_keys[board.enpassant];

    board.enpassant = no_sq;
    board.side ^= 1;
    board.hash_key ^= zobrist_keys.sidekey;
    score = -negamax(-beta, -beta + 1, depth - 1 - 2);
    --search_context.ply;
    --search_context.repetition_index;

    take_back();

    if (time_controls.stopped == 1)
      return 0;
    if (score >= beta)
      return beta;
  }

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);

  if (search_context.follow_pv)
    enable_pv_scoring(&move_list);

  sort_moves(&move_list);

  int moves_searched = 0;
  for (int count = 0; count < move_list.count; ++count) {
    const int move = move_list.moves[count];
    const int is_quiet_move = get_move_capture(move) == 0 && get_move_promoted(move) == 0;

    if (depth <= 2 && search_context.ply && in_check == 0 && pv_node == 0 &&
        has_static_eval && is_quiet_move &&
        static_eval + (engine_search_config.futility_margin_per_depth * depth) <= alpha) {
      continue;
    }

    if (depth <= 2 && search_context.ply && in_check == 0 && pv_node == 0 &&
        is_quiet_move &&
        moves_searched >= engine_search_config.late_move_pruning_base +
                              engine_search_config.late_move_pruning_scale * depth) {
      continue;
    }

    copy_board();
    ++search_context.ply;
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    if (make_move(move, all_moves) == 0) {
      --search_context.ply;
      --search_context.repetition_index;
      continue;
    }

    ++legal_moves;

    if (moves_searched == 0)
      score = -negamax(-beta, -alpha, depth - 1);
    else {
      if (moves_searched >= full_depth_moves && depth >= eval_tables.reduction_limit &&
          in_check == 0 && is_quiet_move)
        score = -negamax(-alpha - 1, -alpha, depth - 2);
      else
        score = alpha + 1;
      if (score > alpha) {
        score = -negamax(-alpha - 1, -alpha, depth - 1);

        if ((score > alpha) && (score < beta))
          score = -negamax(-beta, -alpha, depth - 1);
      }
    }

    --search_context.ply;
    --search_context.repetition_index;

    take_back();

    if (time_controls.stopped == 1)
      return 0;

    if (search_context.ply == 0 && search_context.root_count < max_moves) {
      search_context.root_moves[search_context.root_count] = move;
      search_context.root_scores[search_context.root_count] = score;
      ++search_context.root_count;
    }

    ++moves_searched;

    if (score > alpha) {
      hash_flag = hash_flag_exact;

      if (get_move_capture(move) == 0) {
        search_context.history_moves[get_move_piece(move)][get_move_target(move)] +=
            engine_search_config.history_bonus_scale * depth * depth;
      }
      alpha = score;
      search_context.pv_table[search_context.ply][search_context.ply] = move;

      for (int next_ply = search_context.ply + 1; next_ply < search_context.pv_length[search_context.ply + 1]; ++next_ply)
        search_context.pv_table[search_context.ply][next_ply] = search_context.pv_table[search_context.ply + 1][next_ply];
      search_context.pv_length[search_context.ply] = search_context.pv_length[search_context.ply + 1];

      if (score >= beta) {
        write_hash_entry(beta, depth, hash_flag_beta);
        if (get_move_capture(move) == 0) {
          search_context.killer_moves[1][search_context.ply] = search_context.killer_moves[0][search_context.ply];
          search_context.killer_moves[0][search_context.ply] = move;
        }

        return beta;
      }
    }
  }

  if (legal_moves == 0) {
    if (in_check)
      return -mate_value + search_context.ply;
    else
      return 0;
  }

  write_hash_entry(alpha, depth, hash_flag);

  return alpha;
}

int quiescence(int alpha, int beta) {
  if (should_return_draw_score())
    return 0;

  if ((search_context.nodes & 2047) == 0)
    communicate();

  ++search_context.nodes;

  if (search_context.ply > max_ply - 1)
    return evaluate();

  int evaluation = evaluate();

  if (evaluation >= beta)
    return beta;
  if (evaluation > alpha)
    alpha = evaluation;

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);
  sort_moves(&move_list);

  for (int count = 0; count < move_list.count; ++count) {
    const int move = move_list.moves[count];
    if (!get_move_capture(move))
      continue;
    const int se = see_evaluate(move);
    const int see_m = engine_search_config.quiescence_see_prune_margin;
    if (se < -see_m && evaluation + se < alpha)
      continue;
    copy_board();
    ++search_context.ply;
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    if (make_move(move, only_captures) == 0) {
      --search_context.ply;
      --search_context.repetition_index;
      continue;
    }

    const int score = -quiescence(-beta, -alpha);
    --search_context.ply;
    --search_context.repetition_index;
    take_back();

    if (time_controls.stopped == 1)
      return 0;

    if (score > alpha) {
      alpha = score;
      if (score >= beta)
        return beta;
    }
  }

  return alpha;
}

int score_move(int move) {
  int promotion_bonus = 0;

  if (get_move_promoted(move)) {
    promotion_bonus = 2000 + abs(eval_tables.material_score[opening][get_move_promoted(move)]);
  }

  if (search_context.score_pv) {
    if (search_context.pv_table[0][search_context.ply] == move) {
      search_context.score_pv = 0;
      return 20000;
    }
  }
  if (get_move_capture(move)) {
    int target_piece = P;
    int start_piece, end_piece;

    if (board.side == white) {
      start_piece = p;
      end_piece = k;
    } else {
      start_piece = P;
      end_piece = K;
    }

    for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
      if (get_bit(board.bitboards[bb_piece], get_move_target(move))) {
        target_piece = bb_piece;
        break;
      }
    }

    return eval_tables.mvv_lva[get_move_piece(move)][target_piece] + 10000 + promotion_bonus;
  } else {
    if (promotion_bonus > 0)
      return 9500 + promotion_bonus;
    if (search_context.killer_moves[0][search_context.ply] == move)
      return 9000;
    else if (search_context.killer_moves[1][search_context.ply] == move)
      return 8000;
    else
      return search_context.history_moves[get_move_piece(move)][get_move_target(move)];
  }
}

void sort_moves(MoveList* move_list) {
  assert(move_list->count <= max_moves);
  int move_scores[max_moves];

  for (int i = 0; i < move_list->count; ++i)
    move_scores[i] = score_move(move_list->moves[i]);

  for (int i = 1; i < move_list->count; ++i) {
    const int s = move_scores[i];
    const int m = move_list->moves[i];
    int j = i;
    while (j > 0 && move_scores[j - 1] < s) {
      move_scores[j] = move_scores[j - 1];
      move_list->moves[j] = move_list->moves[j - 1];
      --j;
    }
    move_scores[j] = s;
    move_list->moves[j] = m;
  }
}

void enable_pv_scoring(MoveList* move_list) {
  search_context.follow_pv = 0;

  for (int count = 0; count < move_list->count; ++count) {
    if (search_context.pv_table[0][search_context.ply] == move_list->moves[count]) {
      search_context.score_pv = 1;
      search_context.follow_pv = 1;
    }
  }
}

int is_repetition(void) {
  return repetition_count() > 1;
}

int repetition_count(void) {
  int count = 1;

  for (int index = 0; index < search_context.repetition_index; ++index)
    if (search_context.repetition_table[index] == board.hash_key)
      ++count;

  return count;
}

int is_threefold_repetition(void) {
  return repetition_count() >= 3;
}

int is_fifty_move_rule(void) {
  return board.standard_rules && board.halfmove_clock >= 100;
}

int is_fivefold_repetition(void) {
  return repetition_count() >= 5;
}

int is_seventy_five_move_rule(void) {
  return board.standard_rules && board.halfmove_clock >= 150;
}

int is_insufficient_material(void) {
  if (!board.standard_rules)
    return 0;

  if (count_standard_material(P) || count_standard_material(p) ||
      count_standard_material(R) || count_standard_material(r) ||
      count_standard_material(Q) || count_standard_material(q))
    return 0;

  if (count_standard_material(A) || count_standard_material(a) ||
      count_standard_material(C) || count_standard_material(c) ||
      count_standard_material(M) || count_standard_material(m))
    return 0;

  const int white_knights = count_standard_material(N);
  const int black_knights = count_standard_material(n);
  const int white_bishops = count_standard_material(B);
  const int black_bishops = count_standard_material(b);
  const int white_minors = white_knights + white_bishops;
  const int black_minors = black_knights + black_bishops;

  if (white_minors == 0 && black_minors == 0)
    return 1;

  if (white_minors == 1 && black_minors == 0)
    return 1;

  if (white_minors == 0 && black_minors == 1)
    return 1;

  if (white_knights == 2 && white_bishops == 0 && black_minors == 0)
    return 1;

  if (black_knights == 2 && black_bishops == 0 && white_minors == 0)
    return 1;

  if (white_minors == 1 && black_minors == 1)
    return 1;

  if (white_knights == 0 && black_knights == 0 &&
      white_bishops == 1 && black_bishops == 1) {
    const int white_square = get_ls1b_index(board.bitboards[B]);
    const int black_square = get_ls1b_index(board.bitboards[b]);
    return ((white_square ^ black_square) & 1) == 0;
  }

  return 0;
}
