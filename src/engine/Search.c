#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maharajah/board/Prints.h"
#include "maharajah/engine/Attacks.h"
#include "maharajah/engine/Evaluate.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/engine/Search.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/engine/Zobrist.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/uci/UCI.h"
#include "maharajah/util/Defines.h"
#include "maharajah/util/Utils.h"

void search_position(int depth) {
  int start = get_time_ms();
  int score = 0;
  search_context.nodes = 0;
  time_controls.stopped = 0;
  search_context.follow_pv = 0;
  search_context.score_pv = 0;

  memset(search_context.killer_moves, 0, sizeof(search_context.killer_moves));
  memset(search_context.history_moves, 0, sizeof(search_context.history_moves));
  memset(search_context.pv_table, 0, sizeof(search_context.pv_table));
  memset(search_context.pv_length, 0, sizeof(search_context.pv_length));

  int alpha = -infinity;
  int beta = infinity;

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

    if (search_context.pv_length[0]) {
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
  }
  printf("bestmove ");
  print_move(search_context.pv_table[0][0]);
  printf("\n");
}

int negamax(int alpha, int beta, int depth) {
  search_context.pv_length[search_context.ply] = search_context.ply;
  int score;
  int hash_flag = hash_flag_alpha;

  if (search_context.ply && is_repetition())
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

  ++search_context.nodes;

  int in_check = is_square_attacked((board.side == white) ? get_ls1b_index(board.bitboards[K]) : get_ls1b_index(board.bitboards[k]), board.side ^ 1);

  if (in_check)
    ++depth;

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
    board.hash_key ^= zobrist_keys.side_key;
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
    copy_board();
    ++search_context.ply;
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    if (make_move(move_list.moves[count], all_moves) == 0) {
      --search_context.ply;
      --search_context.repetition_index;
      continue;
    }

    ++legal_moves;

    if (moves_searched == 0)
      score = -negamax(-beta, -alpha, depth - 1);
    else {
      if (moves_searched >= full_depth_moves && depth >= eval_tables.reduction_limit && in_check == 0 && get_move_capture(move_list.moves[count]) == 0
          && get_move_promoted(move_list.moves[count]) == 0)
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
    ++moves_searched;

    if (score > alpha) {
      hash_flag = hash_flag_exact;

      if (get_move_capture(move_list.moves[count]) == 0)
        search_context.history_moves[get_move_piece(move_list.moves[count])][get_move_target(move_list.moves[count])] += depth;
      alpha = score;
      search_context.pv_table[search_context.ply][search_context.ply] = move_list.moves[count];

      for (int next_ply = search_context.ply + 1; next_ply < search_context.pv_length[search_context.ply + 1]; ++next_ply)
        search_context.pv_table[search_context.ply][next_ply] = search_context.pv_table[search_context.ply + 1][next_ply];
      search_context.pv_length[search_context.ply] = search_context.pv_length[search_context.ply + 1];

      if (score >= beta) {
        write_hash_entry(beta, depth, hash_flag_beta);
        if (get_move_capture(move_list.moves[count]) == 0) {
          search_context.killer_moves[1][search_context.ply] = search_context.killer_moves[0][search_context.ply];
          search_context.killer_moves[0][search_context.ply] = move_list.moves[count];
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
    copy_board();
    ++search_context.ply;
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    if (make_move(move_list.moves[count], only_captures) == 0) {
      --search_context.ply;
      --search_context.repetition_index;
      continue;
    }

    int score = -quiescence(-beta, -alpha);
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

    return eval_tables.mvv_lva[get_move_piece(move)][target_piece] + 10000;
  } else {
    if (search_context.killer_moves[0][search_context.ply] == move)
      return 9000;
    else if (search_context.killer_moves[1][search_context.ply] == move)
      return 8000;
    else
      return search_context.history_moves[get_move_piece(move)][get_move_target(move)];
  }
}

void sort_moves(MoveList* move_list) {
  int move_scores[0x100];

  for (int count = 0; count < move_list->count; ++count)
    move_scores[count] = score_move(move_list->moves[count]);

  for (int current_move = 0; current_move < move_list->count; ++current_move) {
    for (int next_move = current_move + 1; next_move < move_list->count; ++next_move) {
      if (move_scores[current_move] < move_scores[next_move]) {
        int temp_score = move_scores[current_move];
        move_scores[current_move] = move_scores[next_move];
        move_scores[next_move] = temp_score;
        int temp_move = move_list->moves[current_move];
        move_list->moves[current_move] = move_list->moves[next_move];
        move_list->moves[next_move] = temp_move;
      }
    }
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

int is_repetition() {
  for (int index = 0; index < search_context.repetition_index; ++index)
    if (search_context.repetition_table[index] == board.hash_key)
      return 1;

  return 0;
}