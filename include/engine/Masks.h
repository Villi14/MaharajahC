#ifndef MASKS_H_
#define MASKS_H_

#include "Defines.h"
#include "Globals.h"
#include "Utils.h"

/*         Rank mask            File mask           Isolated mask        Passed pawn mask
        for square a6        for square f2         for square g2          for square c4

    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 1 0 0    8  0 0 0 0 0 1 0 1     8  0 1 1 1 0 0 0 0
    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 1 0 0    7  0 0 0 0 0 1 0 1     7  0 1 1 1 0 0 0 0
    6  1 1 1 1 1 1 1 1    6  0 0 0 0 0 1 0 0    6  0 0 0 0 0 1 0 1     6  0 1 1 1 0 0 0 0
    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 1 0 0    5  0 0 0 0 0 1 0 1     5  0 1 1 1 0 0 0 0
    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 1 0 0    4  0 0 0 0 0 1 0 1     4  0 0 0 0 0 0 0 0
    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 1 0 0    3  0 0 0 0 0 1 0 1     3  0 0 0 0 0 0 0 0
    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 1 0 0    2  0 0 0 0 0 1 0 1     2  0 0 0 0 0 0 0 0
    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 1 0 0    1  0 0 0 0 0 1 0 1     1  0 0 0 0 0 0 0 0

       a b c d e f g h       a b c d e f g h       a b c d e f g h        a b c d e f g h
*/

// file masks [square]
extern U64 file_masks[0x40];

// rank masks [square]
extern U64 rank_masks[0x40];

// isolated pawn masks [square]
extern U64 isolated_masks[0x40];

// white passed pawn masks [square]
extern U64 white_passed_masks[0x40];

// black passed pawn masks [square]
extern U64 black_passed_masks[0x40];

// extract rank from a square [square]
extern const int get_rank[0x40];

// init evaluation masks
void init_evaluation_masks();

// set file or rank mask
U64 set_file_rank_mask(int file_number, int rank_number);

// get game phase score
static inline int get_game_phase_score() {

  /*  The game phase score of the game is derived from the pieces
        (not counting pawns and kings) that are still on the board.
        The full material starting position game phase score is:

        4 * knight material score in the opening +
        4 * bishop material score in the opening +
        4 * rook material score in the opening +
        2 * queen material score in the opening
  */

  // white & black game phase scores
  int white_piece_scores = 0, black_piece_scores = 0;

  // loop over white pieces
  for (int piece = N; piece <= Q; piece++)
    white_piece_scores += count_bits(board.bitboards[piece]) * eval_tables.material_score[opening][piece];

  // loop over white pieces
  for (int piece = n; piece <= q; piece++)
    black_piece_scores += count_bits(board.bitboards[piece]) * -eval_tables.material_score[opening][piece];

  // return game phase score
  return white_piece_scores + black_piece_scores;
}

#endif // !MASKS_H_
