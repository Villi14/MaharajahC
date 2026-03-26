#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "Defines.h"
#include "Globals.h"
#include "Masks.h"
#include "Utils.h"

static inline int evaluate() {
  // get game phase score
  int game_phase_score = get_game_phase_score();

  // game phase (opening, middle game, endgame)
  int game_phase = -1;

  // pick up game phase based on game phase score
  if (game_phase_score > eval_tables.opening_phase_score)
    game_phase = opening;
  else if (game_phase_score < eval_tables.endgame_phase_score)
    game_phase = endgame;
  else
    game_phase = middlegame;

  // static evaluation score
  int score = 0, score_opening = 0, score_endgame = 0;

  // current pieces bitboard copy
  u64 bitboard;

  // init piece & square
  int piece, square;

  // penalties
  int double_pawns = 0;

  // loop over piece board.bitboards
  for (int bb_piece = P; bb_piece <= k; bb_piece++) {
    // init piece bitboard copy
    bitboard = board.bitboards[bb_piece];

    // loop over pieces within a bitboard
    while (bitboard) {
      // init piece
      piece = bb_piece;

      // init square
      square = get_ls1b_index(bitboard);

      // get opening/endgame material score
      score_opening += eval_tables.material_score[opening][piece];
      score_endgame += eval_tables.material_score[endgame][piece];

      // score positional piece scores
      switch (piece) {
      // evaluate white pawns
      case P:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][PAWN][square];
        score_endgame += eval_tables.positional_score[endgame][PAWN][square];

        // double pawn penalty
        double_pawns = count_bits(board.bitboards[P] & file_masks[square]);

        // on double pawns (tripple, etc)
        if (double_pawns > 1) {
          score_opening += (double_pawns - 1) * eval_tables.double_pawn_penalty_opening;
          score_endgame += (double_pawns - 1) * eval_tables.double_pawn_penalty_endgame;
        }

        // on isolated pawn
        if ((board.bitboards[P] & isolated_masks[square]) == 0) {
          // give an isolated pawn penalty
          score_opening += eval_tables.isolated_pawn_penalty_opening;
          score_endgame += eval_tables.isolated_pawn_penalty_endgame;
        }
        // on passed pawn
        if ((white_passed_masks[square] & board.bitboards[p]) == 0) {
          // give passed pawn bonus
          score_opening += eval_tables.passed_pawn_bonus[get_rank[square]];
          score_endgame += eval_tables.passed_pawn_bonus[get_rank[square]];
        }

        break;

      // evaluate white knights
      case N:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][KNIGHT][square];
        score_endgame += eval_tables.positional_score[endgame][KNIGHT][square];

        break;

      // evaluate white bishops
      case B:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][BISHOP][square];
        score_endgame += eval_tables.positional_score[endgame][BISHOP][square];

        // mobility
        score_opening += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        break;

      // evaluate white rooks
      case R:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][ROOK][square];
        score_endgame += eval_tables.positional_score[endgame][ROOK][square];

        // semi open file
        if ((board.bitboards[P] & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening += eval_tables.semi_open_file_score;
          score_endgame += eval_tables.semi_open_file_score;
        }

        // semi open file
        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening += eval_tables.open_file_score;
          score_endgame += eval_tables.open_file_score;
        }

        break;

      // evaluate white queens
      case Q:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][QUEEN][square];
        score_endgame += eval_tables.positional_score[endgame][QUEEN][square];

        // mobility
        score_opening += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        break;

      // evaluate white king
      case K:
        // get opening/endgame positional score
        score_opening += eval_tables.positional_score[opening][KING][square];
        score_endgame += eval_tables.positional_score[endgame][KING][square];

        // semi open file
        if ((board.bitboards[P] & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening -= eval_tables.semi_open_file_score;
          score_endgame -= eval_tables.semi_open_file_score;
        }

        // semi open file
        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening -= eval_tables.open_file_score;
          score_endgame -= eval_tables.open_file_score;
        }

        // king safety bonus
        score_opening += count_bits(attack_tables.king_attacks[square] & board.occupancies[white]) * eval_tables.king_shield_bonus;
        score_endgame += count_bits(attack_tables.king_attacks[square] & board.occupancies[white]) * eval_tables.king_shield_bonus;

        break;

      // evaluate black pawns
      case p:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][PAWN][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][PAWN][eval_tables.mirror_score[square]];

        // double pawn penalty
        double_pawns = count_bits(board.bitboards[p] & file_masks[square]);

        // on double pawns (tripple, etc)
        if (double_pawns > 1) {
          score_opening -= (double_pawns - 1) * eval_tables.double_pawn_penalty_opening;
          score_endgame -= (double_pawns - 1) * eval_tables.double_pawn_penalty_endgame;
        }

        // on isolated pawn
        if ((board.bitboards[p] & isolated_masks[square]) == 0) {
          // give an isolated pawn penalty
          score_opening -= eval_tables.isolated_pawn_penalty_opening;
          score_endgame -= eval_tables.isolated_pawn_penalty_endgame;
        }
        // on passed pawn
        if ((black_passed_masks[square] & board.bitboards[P]) == 0) {
          // give passed pawn bonus
          score_opening -= eval_tables.passed_pawn_bonus[get_rank[square]];
          score_endgame -= eval_tables.passed_pawn_bonus[get_rank[square]];
        }

        break;

      // evaluate black knights
      case n:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][KNIGHT][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][KNIGHT][eval_tables.mirror_score[square]];

        break;

      // evaluate black bishops
      case b:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][BISHOP][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][BISHOP][eval_tables.mirror_score[square]];

        // mobility
        score_opening -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        break;

      // evaluate black rooks
      case r:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][ROOK][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][ROOK][eval_tables.mirror_score[square]];

        // semi open file
        if ((board.bitboards[p] & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening -= eval_tables.semi_open_file_score;
          score_endgame -= eval_tables.semi_open_file_score;
        }

        // semi open file
        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file bonus
          score_opening -= eval_tables.open_file_score;
          score_endgame -= eval_tables.open_file_score;
        }

        break;

      // evaluate black queens
      case q:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][QUEEN][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][QUEEN][eval_tables.mirror_score[square]];

        // mobility
        score_opening -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        break;

      // evaluate black king
      case k:
        // get opening/endgame positional score
        score_opening -= eval_tables.positional_score[opening][KING][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][KING][eval_tables.mirror_score[square]];

        // semi open file
        if ((board.bitboards[p] & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening += eval_tables.semi_open_file_score;
          score_endgame += eval_tables.semi_open_file_score;
        }

        // semi open file
        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          // add semi open file penalty
          score_opening += eval_tables.open_file_score;
          score_endgame += eval_tables.open_file_score;
        }

        // king safety bonus
        score_opening -= count_bits(attack_tables.king_attacks[square] & board.occupancies[black]) * eval_tables.king_shield_bonus;
        score_endgame -= count_bits(attack_tables.king_attacks[square] & board.occupancies[black]) * eval_tables.king_shield_bonus;
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
          score_endgame * (eval_tables.opening_phase_score - game_phase_score)
        ) / eval_tables.opening_phase_score

        E.g. the score for pawn on d4 at phase say 5000 would be
        interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
    */

  // interpolate score in the middlegame
  if (game_phase == middlegame)
    score = (score_opening * game_phase_score + score_endgame * (eval_tables.opening_phase_score - game_phase_score)) / eval_tables.opening_phase_score;

  // return pure opening score in opening
  else if (game_phase == opening)
    score = score_opening;

  // return pure endgame score in endgame
  else if (game_phase == endgame)
    score = score_endgame;

  // return final evaluation based on board.side
  return (board.side == white) ? score : -score;
}

#endif // !EVALUATE_H_
