#ifndef MASKS_H_
#define MASKS_H_

#include "Defines.h"
#include "Globals.h"
#include "Utils.h"

// file masks [square]
extern u64 file_masks[0x40];

// rank masks [square]
extern u64 rank_masks[0x40];

// isolated pawn masks [square]
extern u64 isolated_masks[0x40];

// white passed pawn masks [square]
extern u64 white_passed_masks[0x40];

// black passed pawn masks [square]
extern u64 black_passed_masks[0x40];

// extract rank from a square [square]
extern const int get_rank[0x40];

// init evaluation masks
void init_evaluation_masks();

// set file or rank mask
u64 set_file_rank_mask(int file_number, int rank_number);

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
  for (int piece = N; piece <= Q; ++piece)
    white_piece_scores += count_bits(board.bitboards[piece]) * eval_tables.material_score[opening][piece];

  // loop over white pieces
  for (int piece = n; piece <= q; ++piece)
    black_piece_scores += count_bits(board.bitboards[piece]) * -eval_tables.material_score[opening][piece];

  // return game phase score
  return white_piece_scores + black_piece_scores;
}

#endif // !MASKS_H_
