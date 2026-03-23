#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdio.h>

#include "Attacks.h"
#include "Defines.h"
#include "Evaluate.h"
#include "Globals.h"
#include "Moves.h"
#include "Prints.h"
#include "Search.h"

static inline int negamax(int alpha, int beta, int depth);
static inline int quiescence(int alpha, int beta);
static inline void search_position(int depth);
static inline void sort_moves(moves* move_list);
static inline int score_move(int move);
static inline void enable_pv_scoring(moves* move_list);

// search position for the best move
static inline void search_position(int depth) {
  // define best score variable
  int score = 0;

  // reset nodes counter
  nodes = 0;

  // reset follow PV flags
  follow_pv = 0;
  score_pv = 0;

  // clear helper data structures for search
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_moves, 0, sizeof(history_moves));
  memset(pv_table, 0, sizeof(pv_table));
  memset(pv_length, 0, sizeof(pv_length));

  // iterative deepening
  for (int current_depth = 1; current_depth <= depth; ++current_depth) {
    // enable follow PV flag
    follow_pv = 1;

    // find best move within a given position
    score = negamax(-50000, 50000, current_depth);

    printf("info score cp %d depth %d nodes %ld pv ", score, current_depth, nodes);

    // loop over the moves within a PV line
    for (int count = 0; count < pv_length[0]; ++count) {
      // print PV move
      print_move(pv_table[0][count]);
      printf(" ");
    }

    // print new line
    printf("\n");
  }

  // best move placeholder
  printf("bestmove ");
  print_move(pv_table[0][0]);
  printf("\n");
}

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth) {
  // define find PV node variable
  int found_pv = 0;

  // init PV length
  pv_length[ply] = ply;

  // recursion escape condition
  if (depth == 0)
    // run quiescence search
    return quiescence(alpha, beta);

  // we are too deep, hence there's an overflow of arrays relying on max ply constant
  if (ply > max_ply - 1)
    // evaluate position
    return evaluate();

  // increment nodes count
  ++nodes;

  // is king in check
  int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) : get_ls1b_index(bitboards[k]), side ^ 1);

  // increase search depth if the king has been exposed into a check
  if (in_check)
    ++depth;

  // legal moves counter
  int legal_moves = 0;

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // if we are now following PV line
  if (follow_pv)
    // enable PV move scoring
    enable_pv_scoring(move_list);

  // sort moves
  sort_moves(move_list);

  // loop over moves within a novelist
  for (int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment ply
    ++ply;

    // make sure to make only legal moves
    if (make_move(move_list->moves[count], all_moves) == 0) {
      // decrement ply
      --ply;

      // skip to next move
      continue;
    }
    // increment legal moves
    ++legal_moves;

    /*
        if (fFoundPv) {
            val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);

            if ((val > alpha) && (val < beta)) // Check for failure.

                val = -AlphaBeta(depth - 1, -beta, -alpha);

        } else

            val = -AlphaBeta(depth - 1, -beta, -alpha);
        */

    // variable to store current move's score (from the static evaluation perspective)
    int score;

    // on PV node hit
    if (found_pv) {
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
      if ((score > alpha) && (score < beta)) // Check for failure.
        /* re-search the move that has failed to be proved to be bad
                   with normal alpha beta score bounds*/
        score = -negamax(-beta, -alpha, depth - 1);
    }

    // for all other types of nodes (moves)  1056080 824253
    else {
      // do normal alpha beta search
      score = -negamax(-beta, -alpha, depth - 1);
    }

    // decrement ply
    --ply;

    // take move back
    take_back();

    // fail-hard beta cutoff
    if (score >= beta) {
      // on quiet moves
      if (get_move_capture(move_list->moves[count]) == 0) {
        // store killer moves
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = move_list->moves[count];
      }
      return beta;
    }

    // found a better move
    if (score > alpha) {
      // on quiet moves
      if (get_move_capture(move_list->moves[count]) == 0)
        // store history moves
        history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;

      // PV node (move)
      alpha = score;

      // enable found PV flag
      found_pv = 1;

      // write PV move
      pv_table[ply][ply] = move_list->moves[count];

      // loop over the next ply
      for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
        // copy move from deeper ply into a current ply's line
        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];

      // adjust PV length
      pv_length[ply] = pv_length[ply + 1];
    }
  }

  // we don't have any legal moves to make in the current postion
  if (legal_moves == 0) {
    // king is in check
    if (in_check)
      // return mating score (assuming closest distance to mating position)
      return -49000 + ply;

    // king is not in check
    else
      // return stalemate score
      return 0;
  }

  // node (move) fails low
  return alpha;
}

// quiescence search
static inline int quiescence(int alpha, int beta) {
  // increment nodes count
  ++nodes;

  // evaluate position
  int evaluation = evaluate();

  // fail-hard beta cutoff
  if (evaluation >= beta) {
    // node (move) fails high
    return beta;
  }

  // found a better move
  if (evaluation > alpha) {
    // PV node (move)
    alpha = evaluation;
  }

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // sort moves
  sort_moves(move_list);

  // loop over moves within a movelist
  for (int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment ply
    ++ply;

    // make sure to make only legal moves
    if (make_move(move_list->moves[count], only_captures) == 0) {
      // decrement ply
      --ply;

      // skip to next move
      continue;
    }

    // score current move
    int score = -quiescence(-beta, -alpha);

    // decrement ply
    --ply;

    // take move back
    take_back();

    // fail-hard beta cutoff
    if (score >= beta) {
      // node (move) fails high
      return beta;
    }

    // found a better move
    if (score > alpha) {
      // PV node (move)
      alpha = score;
    }
  }

  // node (move) fails low
  return alpha;
}

static inline int score_move(int move) {
  // if PV move scoring is allowed
  if (score_pv) {
    // make sure we are dealing with PV move
    if (pv_table[0][ply] == move) {
      // disable score PV flag
      score_pv = 0;

      // give PV move the highest score to search it first
      return 20000;
    }
  }

  // score capture move
  if (get_move_capture(move)) {
    // init target piece
    int target_piece = P;

    // pick up bitboard piece index ranges depending on side
    int start_piece, end_piece;

    // pick up side to move
    if (side == white) {
      start_piece = p;
      end_piece = k;
    } else {
      start_piece = P;
      end_piece = K;
    }

    // loop over bitboards opposite to the current side to move
    for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
      // if there's a piece on the target square
      if (get_bit(bitboards[bb_piece], get_move_target(move))) {
        // remove it from corresponding bitboard
        target_piece = bb_piece;
        break;
      }
    }

    // score move by MVV LVA lookup [source piece][target piece]
    return mvv_lva[get_move_piece(move)][target_piece] + 10000;
  }

  // score quiet move
  else {
    // score 1st killer move
    if (killer_moves[0][ply] == move)
      return 9000;

    // score 2nd killer move
    else if (killer_moves[1][ply] == move)
      return 8000;

    // score history move
    else
      return history_moves[get_move_piece(move)][get_move_target(move)];
  }

  return 0;
}

// sort moves in descending order
static inline void sort_moves(moves* move_list) {
  // move scores
  int move_scores[move_list->count];

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
static inline void enable_pv_scoring(moves* move_list) {
  // disable following PV
  follow_pv = 0;

  // loop over the moves within a move list
  for (int count = 0; count < move_list->count; ++count) {
    // make sure we hit PV move
    if (pv_table[0][ply] == move_list->moves[count]) {
      // enable move scoring
      score_pv = 1;

      // enable following PV
      follow_pv = 1;
    }
  }
}

#endif // !SEARCH_H_
