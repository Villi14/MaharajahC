#include <stdio.h>
#include <string.h>

#include "../headers/Defines.h"
#include "../headers/Evaluate.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"

// search position for the best move
void search_position(int depth) {
  // find best move within a given position
  int score = negamax(-50000, 50000, depth);
  printf("bestmove ");
  print_move(best_move);
  printf("\n");
}

// negamax alpha beta search
int negamax(int alpha, int beta, int depth) {
  // recurrsion escapre condition
  if(depth == 0)
    // return evaluation
    return evaluate();

  // increment nodes count
  nodes++;

  // best move so far
  int best_sofar;

  // old value of alpha
  int old_alpha = alpha;

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // loop over moves within a movelist
  for(int count = 0; count < move_list->count; count++) {
    // preserve board state
    copy_board();

    // increment ply
    ply++;

    // make sure to make only legal moves
    if(make_move(move_list->moves[count], all_moves) == 0) {
      // decrement ply
      ply--;

      // skip to next move
      continue;
    }

    // score current move
    int score = -negamax(-beta, -alpha, depth - 1);

    // decrement ply
    ply--;

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

  // found better move
  if(old_alpha != alpha)
    // init best move
    best_move = best_sofar;

  // node (move) fails low
  return alpha;
}
