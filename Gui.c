#include "Gui.h"
#include "Functions.h"
#include "Globals.h"

// parse user/GUI move string input (e.g. "e7e8q")
int parse_move(char* move_string) {
  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // parse source square
  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

  // parse target square
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

  // loop over the moves within a move list
  for(int move_count = 0; move_count < move_list->count; move_count++) {
    // init move
    int move = move_list->moves[move_count];

    // make sure source & target squares are available within the generated move
    if(source_square == get_move_source(move) && target_square == get_move_target(move)) {
      // init promoted piece
      int promoted_piece = get_move_promoted(move);

      // promoted piece is available
      if(promoted_piece) {
        // promoted to queen
        if((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
          // return legal move
          return move;

        // promoted to rook
        else if((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
          // return legal move
          return move;

        // promoted to bishop
        else if((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
          // return legal move
          return move;

        // promoted to knight
        else if((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
          // return legal move
          return move;

        // continue the loop on possible wrong promotions (e.g. "e7e8f")
        continue;
      }

      // return legal move
      return move;
    }
  }
  // return illegal move
  return 0;
}
