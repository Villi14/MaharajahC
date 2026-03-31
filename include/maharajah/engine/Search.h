#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "maharajah/engine/Attacks.h"
#include "maharajah/util/Defines.h"
#include "maharajah/engine/Evaluate.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/uci/Prints.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/uci/UCI.h"
#include "maharajah/util/Utils.h"
#include "maharajah/engine/Zobrist.h"

// mate score

static inline int negamax(int alpha, int beta, int depth);
static inline int quiescence(int alpha, int beta);
static inline void search_position(int depth);
static inline void sort_moves(MoveList* move_list);
static inline int score_move(int move);
static inline void enable_pv_scoring(MoveList* move_list);
static inline int is_repetition();

// search position for the best move
static inline void search_position(int depth) {
  // search start time
  int start = get_time_ms();

  // define best score variable
  int score = 0;

  // reset search_context.nodes counter
  search_context.nodes = 0;

  // reset "time is up" flag
  time_controls.stopped = 0;

  // reset follow PV flags
  search_context.follow_pv = 0;
  search_context.score_pv = 0;

  // clear helper data structures for search
  memset(search_context.killer_moves, 0, sizeof(search_context.killer_moves));
  memset(search_context.history_moves, 0, sizeof(search_context.history_moves));
  memset(search_context.pv_table, 0, sizeof(search_context.pv_table));
  memset(search_context.pv_length, 0, sizeof(search_context.pv_length));

  // define initial alpha beta bounds
  int alpha = -infinity;
  int beta = infinity;

  // iterative deepening
  for (int current_depth = 1; current_depth <= depth; ++current_depth) {
    // if time is up
    if (time_controls.stopped == 1)
      // stop calculating and return best move so far
      break;

    // enable follow PV flag
    search_context.follow_pv = 1;

    // find best move within a given position
    score = negamax(alpha, beta, current_depth);

    // we fell outside the window, so try again with a full-width window (and the same depth)
    if ((score <= alpha) || (score >= beta)) {
      alpha = -infinity;
      beta = infinity;
      continue;
    }

    // set up the window for the next iteration
    alpha = score - 50;
    beta = score + 50;

    // if PV is available
    if (search_context.pv_length[0]) {
      // print search info
      if (score > -mate_value && score < -mate_score)
        printf("info score mate %d depth %d search_context.nodes %llu time %d pv ",
               -(score + mate_value) / 2 - 1,
               current_depth,
               (u64)search_context.nodes,
               get_time_ms() - start);

      else if (score > mate_score && score < mate_value)
        printf("info score mate %d depth %d search_context.nodes %llu time %d pv ",
               (mate_value - score) / 2 + 1,
               current_depth,
               (u64)search_context.nodes,
               get_time_ms() - start);

      else
        printf("info score cp %d depth %d search_context.nodes %llu time %d pv ",
               score,
               current_depth,
               (u64)search_context.nodes,
               get_time_ms() - start);

      // loop over the moves within a PV line
      for (int count = 0; count < search_context.pv_length[0]; ++count) {
        // print PV move
        print_move(search_context.pv_table[0][count]);
        printf(" ");
      }

      // print new line
      printf("\n");
    }
  }

  // print best move
  printf("bestmove ");
  print_move(search_context.pv_table[0][0]);
  printf("\n");
}

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth) {
  // init PV length
  search_context.pv_length[search_context.ply] = search_context.ply;

  // variable to store current move's score (from the static evaluation perspective)
  int score;

  // define hash flag
  int hash_flag = hash_flag_alpha;

  // if position repetition occurs
  if (search_context.ply && is_repetition())
    // return draw score
    return 0;

  // a hack by Pedro Castro to figure out whether the current node is PV node or not
  int pv_node = beta - alpha > 1;

  // read hash entry if we're not in a root search_context.ply and hash entry is available
  // and current node is not a PV node
  if (search_context.ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && pv_node == 0)
    // if the move has already been searched (hence has a value)
    // we just return the score for this move without searching it
    return score;

  // every 2047 search_context.nodes
  if ((search_context.nodes & 2047) == 0)
    // "listen" to the GUI/user input
    communicate();

  // recursion escape condition
  if (depth == 0)
    // run quiescence search
    return quiescence(alpha, beta);

  // we are too deep, hence there's an overflow of arrays relying on max search_context.ply constant
  if (search_context.ply > max_ply - 1)
    // evaluate position
    return evaluate();

  // increment search_context.nodes count
  ++search_context.nodes;

  // is king in check
  int in_check = is_square_attacked((board.side == white) ? get_ls1b_index(board.bitboards[K]) : get_ls1b_index(board.bitboards[k]), board.side ^ 1);

  // increase search depth if the king has been exposed into a check
  if (in_check)
    ++depth;

  // legal moves counter
  int legal_moves = 0;

  // null move pruning
  if (depth >= 3 && in_check == 0 && search_context.ply) {
    // preserve board state
    copy_board();

    // increment search_context.ply
    ++search_context.ply;

    // increment repetition index & store hash key
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    // hash board.enpassant if available
    if (board.enpassant != no_sq)
      board.hash_key ^= zobrist_keys.enpassant_keys[board.enpassant];

    // reset board.enpassant capture square
    board.enpassant = no_sq;

    // switch the board.side, literally giving opponent an extra move to make
    board.side ^= 1;

    // hash the board.side
    board.hash_key ^= zobrist_keys.side_key;

    /* search moves with reduced depth to find beta cutoffs
           depth - 1 - R where R is a reduction limit */
    score = -negamax(-beta, -beta + 1, depth - 1 - 2);

    // decrement search_context.ply
    --search_context.ply;

    // decrement repetition index
    --search_context.repetition_index;

    // restore board state
    take_back();

    // return 0 if time is up
    if (time_controls.stopped == 1)
      return 0;

    // fail-hard beta cutoff
    if (score >= beta)
      // node (position) fails high
      return beta;
  }

  // create move list instance
  MoveList move_list[1];

  // generate moves
  generate_moves(move_list);

  // if we are now following PV line
  if (search_context.follow_pv)
    // enable PV move scoring
    enable_pv_scoring(move_list);

  // sort moves
  sort_moves(move_list);

  // number of moves searched in a move list
  int moves_searched = 0;

  // loop over moves within a movelist
  for (int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment search_context.ply
    ++search_context.ply;

    // increment repetition index & store hash key
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    // make sure to make only legal moves
    if (make_move(move_list->moves[count], all_moves) == 0) {
      // decrement search_context.ply
      --search_context.ply;

      // decrement repetition index
      --search_context.repetition_index;

      // skip to next move
      continue;
    }

    // increment legal moves
    ++legal_moves;

    // full depth search
    if (moves_searched == 0)
      // do normal alpha beta search
      score = -negamax(-beta, -alpha, depth - 1);

    // late move reduction (LMR)
    else {
      // condition to consider LMR
      if (moves_searched >= full_depth_moves && depth >= eval_tables.reduction_limit && in_check == 0 && get_move_capture(move_list->moves[count]) == 0
          && get_move_promoted(move_list->moves[count]) == 0)
        // search current move with reduced depth:
        score = -negamax(-alpha - 1, -alpha, depth - 2);

      // hack to ensure that full-depth search is done
      else
        score = alpha + 1;

      // principle variation search PVS
      if (score > alpha) {
        /* Once you've found a move with a score that is between alpha and beta,
                the rest of the moves are searched with the goal of proving that they are all bad.
                It's possible to do this a bit faster than a search that worries that one
                of the remaining moves might be good. */
        score = -negamax(-alpha - 1, -alpha, depth - 1);

        /* If the algorithm finds out that it was wrong, and that one of the
                subsequent moves was better than the first PV move, it has to search again,
                in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                but generally not often enough to counteract the savings gained from doing the
                "bad move proof" search referred to earlier. */
        if ((score > alpha) && (score < beta))
          /* re-search the move that has failed to be proved to be bad
                    with normal alpha beta score bounds*/
          score = -negamax(-beta, -alpha, depth - 1);
      }
    }

    // decrement search_context.ply
    --search_context.ply;

    // decrement repetition index
    --search_context.repetition_index;

    // take move back
    take_back();

    // return 0 if time is up
    if (time_controls.stopped == 1)
      return 0;

    // increment the counter of moves searched so far
    ++moves_searched;

    // found a better move
    if (score > alpha) {
      // switch hash flag from storing score for fail-low node
      // to the one storing score for PV node
      hash_flag = hash_flag_exact;

      // on quiet moves
      if (get_move_capture(move_list->moves[count]) == 0)
        // store history moves
        search_context.history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;

      // PV node (position)
      alpha = score;

      // write PV move
      search_context.pv_table[search_context.ply][search_context.ply] = move_list->moves[count];

      // loop over the next search_context.ply
      for (int next_ply = search_context.ply + 1; next_ply < search_context.pv_length[search_context.ply + 1]; ++next_ply)
        // copy move from deeper search_context.ply into a current search_context.ply's line
        search_context.pv_table[search_context.ply][next_ply] = search_context.pv_table[search_context.ply + 1][next_ply];

      // adjust PV length
      search_context.pv_length[search_context.ply] = search_context.pv_length[search_context.ply + 1];

      // fail-hard beta cutoff
      if (score >= beta) {
        // store hash entry with the score equal to beta
        write_hash_entry(beta, depth, hash_flag_beta);

        // on quiet moves
        if (get_move_capture(move_list->moves[count]) == 0) {
          // store killer moves
          search_context.killer_moves[1][search_context.ply] = search_context.killer_moves[0][search_context.ply];
          search_context.killer_moves[0][search_context.ply] = move_list->moves[count];
        }

        // node (position) fails high
        return beta;
      }
    }
  }

  // we don't have any legal moves to make in the current position
  if (legal_moves == 0) {
    // king is in check
    if (in_check)
      // return mating score (assuming closest distance to mating position)
      return -mate_value + search_context.ply;

    // king is not in check
    else
      // return stalemate score
      return 0;
  }

  // store hash entry with the score equal to alpha
  write_hash_entry(alpha, depth, hash_flag);

  // node (position) fails low
  return alpha;
}

// quiescence search
static inline int quiescence(int alpha, int beta) {
  // every 2047 search_context.nodes
  if ((search_context.nodes & 2047) == 0)
    // "listen" to the GUI/user input
    communicate();

  // increment search_context.nodes count
  ++search_context.nodes;

  // we are too deep, hence there's an overflow of arrays relying on max search_context.ply constant
  if (search_context.ply > max_ply - 1)
    // evaluate position
    return evaluate();

  // evaluate position
  int evaluation = evaluate();

  // fail-hard beta cutoff
  if (evaluation >= beta) {
    // node (position) fails high
    return beta;
  }

  // found a better move
  if (evaluation > alpha) {
    // PV node (position)
    alpha = evaluation;
  }

  // create move list instance
  MoveList move_list[1];

  // generate moves
  generate_moves(move_list);

  // sort moves
  sort_moves(move_list);

  // loop over moves within a movelist
  for (int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment search_context.ply
    ++search_context.ply;

    // increment repetition index & store hash key
    ++search_context.repetition_index;
    search_context.repetition_table[search_context.repetition_index] = board.hash_key;

    // make sure to make only legal moves
    if (make_move(move_list->moves[count], only_captures) == 0) {
      // decrement search_context.ply
      --search_context.ply;

      // decrement repetition index
      --search_context.repetition_index;

      // skip to next move
      continue;
    }

    // score current move
    int score = -quiescence(-beta, -alpha);

    // decrement search_context.ply
    --search_context.ply;

    // decrement repetition index
    --search_context.repetition_index;

    // take move back
    take_back();

    // return 0 if time is up
    if (time_controls.stopped == 1)
      return 0;

    // found a better move
    if (score > alpha) {
      // PV node (position)
      alpha = score;

      // fail-hard beta cutoff
      if (score >= beta) {
        // node (position) fails high
        return beta;
      }
    }
  }

  // node (position) fails low
  return alpha;
}

/*  =======================
         Move ordering
    =======================

    1. PV move
    2. Captures in MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Unsorted moves
*/

static inline int score_move(int move) {
  // if PV move scoring is allowed
  if (search_context.score_pv) {
    // make sure we are dealing with PV move
    if (search_context.pv_table[0][search_context.ply] == move) {
      // disable score PV flag
      search_context.score_pv = 0;

      // give PV move the highest score to search it first
      return 20000;
    }
  }

  // score capture move
  if (get_move_capture(move)) {
    // init target piece
    int target_piece = P;

    // pick up bitboard piece index ranges depending on board.side
    int start_piece, end_piece;

    // pick up board.side to move
    if (board.side == white) {
      start_piece = p;
      end_piece = k;
    } else {
      start_piece = P;
      end_piece = K;
    }

    // loop over board.bitboards opposite to the current board.side to move
    for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
      // if there's a piece on the target square
      if (get_bit(board.bitboards[bb_piece], get_move_target(move))) {
        // remove it from corresponding bitboard
        target_piece = bb_piece;
        break;
      }
    }

    // score move by MVV LVA lookup [source piece][target piece]
    return eval_tables.mvv_lva[get_move_piece(move)][target_piece] + 10000;
  }

  // score quiet move
  else {
    // score 1st killer move
    if (search_context.killer_moves[0][search_context.ply] == move)
      return 9000;

    // score 2nd killer move
    else if (search_context.killer_moves[1][search_context.ply] == move)
      return 8000;

    // score history move
    else
      return search_context.history_moves[get_move_piece(move)][get_move_target(move)];
  }
}

// sort moves in descending order
static inline void sort_moves(MoveList* move_list) {
  // move scores
  int move_scores[256];

  // score all the moves within a move list
  for (int count = 0; count < move_list->count; ++count)
    // score move
    move_scores[count] = score_move(move_list->moves[count]);

  // loop over current move within a move list
  for (int current_move = 0; current_move < move_list->count; ++current_move) {
    // loop over next move within a move list
    for (int next_move = current_move + 1; next_move < move_list->count; ++next_move) {
      // compare current and next move scores
      if (move_scores[current_move] < move_scores[next_move]) {
        // swap scores
        int temp_score = move_scores[current_move];
        move_scores[current_move] = move_scores[next_move];
        move_scores[next_move] = temp_score;

        // swap moves
        int temp_move = move_list->moves[current_move];
        move_list->moves[current_move] = move_list->moves[next_move];
        move_list->moves[next_move] = temp_move;
      }
    }
  }
}

// enable PV move scoring
static inline void enable_pv_scoring(MoveList* move_list) {
  // disable following PV
  search_context.follow_pv = 0;

  // loop over the moves within a move list
  for (int count = 0; count < move_list->count; ++count) {
    // make sure we hit PV move
    if (search_context.pv_table[0][search_context.ply] == move_list->moves[count]) {
      // enable move scoring
      search_context.score_pv = 1;

      // enable following PV
      search_context.follow_pv = 1;
    }
  }
}

// position repetition detection
static inline int is_repetition() {
  // loop over repetition indices range
  for (int index = 0; index < search_context.repetition_index; ++index)
    // if we found the hash key same with a current
    if (search_context.repetition_table[index] == board.hash_key)
      // we found a repetition
      return 1;

  // if no repetition found
  return 0;
}

#endif // !SEARCH_H_
