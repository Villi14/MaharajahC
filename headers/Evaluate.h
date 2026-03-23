#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "../headers/Defines.h"
#include "../headers/Evaluate.h"
#include "../headers/Globals.h"
#include "../headers/Masks.h"
#include "../headers/Utils.h"

static inline int evaluate() {
  // get game phase score
  int game_phase_score = get_game_phase_score();

  // game phase (opening, middle game, endgame)
  int game_phase = -1;

  // pick up game phase based on game phase score
  if (game_phase_score > opening_phase_score)
    game_phase = opening;
  else if (game_phase_score < endgame_phase_score)
    game_phase = endgame;
  else
    game_phase = middlegame;

  // static evaluation score
  int score = 0, score_opening = 0, score_endgame = 0;

  // current pieces bitboard copy
  U64 bitboard;

  // init piece & square
  int piece, square;

  // penalties
  int double_pawns = 0;

  // loop over piece bitboards
  for (int bb_piece = P; bb_piece <= k; bb_piece++) {
    // init piece bitboard copy
    bitboard = bitboards[bb_piece];

    // loop over pieces within a bitboard
    while (bitboard) {
      // init piece
      piece = bb_piece;

      // init square
      square = get_ls1b_index(bitboard);

      // get opening/endgame material score
      score_opening += material_score[opening][piece];
      score_endgame += material_score[endgame][piece];

      // score positional piece scores
      switch (piece) {
      // evaluate white pawns
      case P:
        // get opening/endgame positional score
        score_opening += positional_score[opening][PAWN][square];
        score_endgame += positional_score[endgame][PAWN][square];

        // double pawn penalty
        double_pawns = count_bits(bitboards[P] & file_masks[square]);

        // on double pawns (tripple, etc)
        if (double_pawns > 1) {
          score_opening += (double_pawns - 1) * double_pawn_penalty_opening;
          score_endgame += (double_pawns - 1) * double_pawn_penalty_endgame;
        }

        // on isolated pawn
        if ((bitboards[P] & isolated_masks[square]) == 0) {
          // give an isolated pawn penalty
          score_opening += isolated_pawn_penalty_opening;
          score_endgame += isolated_pawn_penalty_endgame;
        }
        // on passed pawn
        if ((white_passed_masks[square] & bitboards[p]) == 0) {
          // give passed pawn bonus
          score_opening += passed_pawn_bonus[get_rank[square]];
          score_endgame += passed_pawn_bonus[get_rank[square]];
        }

        break;

      // evaluate white knights
      case N:
        // get opening/endgame positional score
        score_opening += positional_score[opening][KNIGHT][square];
        score_endgame += positional_score[endgame][KNIGHT][square];

        break;

      // evaluate white bishops
      case B:
        // get opening/endgame positional score
        score_opening += positional_score[opening][BISHOP][square];
        score_endgame += positional_score[endgame][BISHOP][square];

        // mobility
        score_opening += (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_opening;
        score_endgame += (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_endgame;
        break;

      // evaluate white rooks
      case R:
        // get opening/endgame positional score
        score_opening += positional_score[opening][ROOK][square];
        score_endgame += positional_score[endgame][ROOK][square];

        // semi open file
        if ((bitboards[P] & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening += semi_open_file_score;
          score_endgame += semi_open_file_score;
        }

        // semi open file
        if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening += open_file_score;
          score_endgame += open_file_score;
        }

        break;

      // evaluate white queens
      case Q:
        // get opening/endgame positional score
        score_opening += positional_score[opening][QUEEN][square];
        score_endgame += positional_score[endgame][QUEEN][square];

        // mobility
        score_opening += (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_opening;
        score_endgame += (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_endgame;
        break;

      // evaluate white king
      case K:
        // get opening/endgame positional score
        score_opening += positional_score[opening][KING][square];
        score_endgame += positional_score[endgame][KING][square];

        // semi open file
        if ((bitboards[P] & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening -= semi_open_file_score;
          score_endgame -= semi_open_file_score;
        }

        // semi open file
        if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening -= open_file_score;
          score_endgame -= open_file_score;
        }

        // king safety bonus
        score_opening += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
        score_endgame += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;

        break;

      // evaluate black pawns
      case p:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][PAWN][mirror_score[square]];
        score_endgame -= positional_score[endgame][PAWN][mirror_score[square]];

        // double pawn penalty
        double_pawns = count_bits(bitboards[p] & file_masks[square]);

        // on double pawns (tripple, etc)
        if (double_pawns > 1) {
          score_opening -= (double_pawns - 1) * double_pawn_penalty_opening;
          score_endgame -= (double_pawns - 1) * double_pawn_penalty_endgame;
        }

        // on isolated pawn
        if ((bitboards[p] & isolated_masks[square]) == 0) {
          // give an isolated pawn penalty
          score_opening -= isolated_pawn_penalty_opening;
          score_endgame -= isolated_pawn_penalty_endgame;
        }
        // on passed pawn
        if ((black_passed_masks[square] & bitboards[P]) == 0) {
          // give passed pawn bonus
          score_opening -= passed_pawn_bonus[get_rank[square]];
          score_endgame -= passed_pawn_bonus[get_rank[square]];
        }

        break;

      // evaluate black knights
      case n:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][KNIGHT][mirror_score[square]];
        score_endgame -= positional_score[endgame][KNIGHT][mirror_score[square]];

        break;

      // evaluate black bishops
      case b:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][BISHOP][mirror_score[square]];
        score_endgame -= positional_score[endgame][BISHOP][mirror_score[square]];

        // mobility
        score_opening -= (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_opening;
        score_endgame -= (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_endgame;
        break;

      // evaluate black rooks
      case r:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][ROOK][mirror_score[square]];
        score_endgame -= positional_score[endgame][ROOK][mirror_score[square]];

        // semi open file
        if ((bitboards[p] & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening -= semi_open_file_score;
          score_endgame -= semi_open_file_score;
        }

        // semi open file
        if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening -= open_file_score;
          score_endgame -= open_file_score;
        }

        break;

      // evaluate black queens
      case q:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][QUEEN][mirror_score[square]];
        score_endgame -= positional_score[endgame][QUEEN][mirror_score[square]];

        // mobility
        score_opening -= (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_opening;
        score_endgame -= (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_endgame;
        break;

      // evaluate black king
      case k:
        // get opening/endgame positional score
        score_opening -= positional_score[opening][KING][mirror_score[square]];
        score_endgame -= positional_score[endgame][KING][mirror_score[square]];

        // semi open file
        if ((bitboards[p] & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening += semi_open_file_score;
          score_endgame += semi_open_file_score;
        }

        // semi open file
        if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening += open_file_score;
          score_endgame += open_file_score;
        }

        // king safety bonus
        score_opening -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
        score_endgame -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
        break;
      }

      // pop ls1b
      pop_bit(bitboard, square);
    }
  }

  /*
        Now in order to calculate interpolated score
        for a given game phase we use this formula
        (same for material and positional scores):

        (
          score_opening * game_phase_score +
          score_endgame * (opening_phase_score - game_phase_score)
        ) / opening_phase_score

        E.g. the score for pawn on d4 at phase say 5000 would be
        interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
    */

  // interpolate score in the middlegame
  if (game_phase == middlegame)
    score = (score_opening * game_phase_score + score_endgame * (opening_phase_score - game_phase_score)) / opening_phase_score;

  // return pure opening score in opening
  else if (game_phase == opening)
    score = score_opening;

  // return pure endgame score in endgame
  else if (game_phase == endgame)
    score = score_endgame;

  // return final evaluation based on side
  return (side == white) ? score : -score;
}

#endif // !EVALUATE_H_
