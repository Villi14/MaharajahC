#include "../headers/Evaluate.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"

 int evaluate() {
  // static evaluation score
  int score = 0;

  // current pieces bitboard copy
  U64 bitboard;

  // init piece & square
  int piece, square;

  // loop over piece bitboards
  for(int bb_piece = P; bb_piece <= k; bb_piece++) {
    // init piece bitboard copy
    bitboard = bitboards[bb_piece];

    // loop over pieces within a bitboard
    while(bitboard) {
      // init piece
      piece = bb_piece;

      // init square
      square = get_ls1b_index(bitboard);

      // score material weights
      score += material_score[piece];

      // pop ls1b
      pop_bit(bitboard, square);
    }
  }

  // return final evaluation based on side
  return (side == white) ? score : -score;
}
