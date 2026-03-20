#include <stdio.h>
#include <string.h>

#include "../headers/Defines.h"
#include "../headers/Evaluate.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"
#include "../headers/Attacks.h"

// search position for the best move
void search_position(int depth) {
  // find best move within a given position
  int score = negamax(-50000, 50000, depth);
  if(best_move) {
    printf("info score cp %d depth %d nodes %ld\n", score, depth, nodes);

    // best move placeholder
    printf("best move ");
    print_move(best_move);
    printf("\n");
  }
}

// negamax alpha beta search
int negamax(int alpha, int beta, int depth) {
  // recursion escape condition
  if(depth == 0)
    // run quiescence search
    return quiescence(alpha, beta);

  // increment nodes count
  ++nodes;

  // is king in check
  int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) : get_ls1b_index(bitboards[k]), side ^ 1);

  // legal moves counter
  int legal_moves = 0;

  // best move so far
  int best_sofar;

  // old value of alpha
  int old_alpha = alpha;

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // loop over moves within a novelist
  for(int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment ply
    ++ply;

    // make sure to make only legal moves
    if(make_move(move_list->moves[count], all_moves) == 0) {
      // decrement ply
      --ply;

      // skip to next move
      continue;
    }
    // increment legal moves
    legal_moves++;

    // score current move
    int score = -negamax(-beta, -alpha, depth - 1);

    // decrement ply
    --ply;

    // take move back
    take_back();

    // fail-hard beta cutoff
    if(score >= beta) {
      // node (move) fails high
      return beta;
    }

    // found a better move
    if(score > alpha) {
      // PV node (move)
      alpha = score;

      // if root move
      if(ply == 0)
        // associate best move with the best score
        best_sofar = move_list->moves[count];
    }
  }

  // we don't have any legal moves to make in the current postion
  if(legal_moves == 0) {
    // king is in check
    if(in_check)
      // return mating score (assuming closest distance to mating position)
      return -49000 + ply;

    // king is not in check
    else
      // return stalemate score
      return 0;
  }

  // found better move
  if(old_alpha != alpha)
    // init best move
    best_move = best_sofar;

  // node (move) fails low
  return alpha;
}

// quiescence search
int quiescence(int alpha, int beta) {
  // increment nodes count
  ++nodes;

  // evaluate position
  int evaluation = evaluate();

  // fail-hard beta cutoff
  if(evaluation >= beta) {
    // node (move) fails high
    return beta;
  }

  // found a better move
  if(evaluation > alpha) {
    // PV node (move)
    alpha = evaluation;
  }

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // loop over moves within a movelist
  for(int count = 0; count < move_list->count; ++count) {
    // preserve board state
    copy_board();

    // increment ply
    ++ply;

    // make sure to make only legal moves
    if(make_move(move_list->moves[count], only_captures) == 0) {
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
    if(score >= beta) {
      // node (move) fails high
      return beta;
    }

    // found a better move
    if(score > alpha) {
      // PV node (move)
      alpha = score;
    }
  }

  // node (move) fails low
  return alpha;
}
