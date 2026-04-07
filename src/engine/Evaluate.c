#include "maharajah/engine/Evaluate.h"
#include "maharajah/engine/Attacks.h"

int evaluate(void) {
  int game_phase_score = get_game_phase_score();
  int game_phase = -1;

  if (game_phase_score > eval_tables.opening_phase_score)
    game_phase = opening;
  else if (game_phase_score < eval_tables.endgame_phase_score)
    game_phase = endgame;
  else
    game_phase = middlegame;

  int score = 0, score_opening = 0, score_endgame = 0;
  u64 bitboard;
  int piece, square;
  int double_pawns = 0;

  for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
    bitboard = board.bitboards[bb_piece];
    while (bitboard) {
      piece = bb_piece;
      square = get_ls1b_index(bitboard);
      score_opening += eval_tables.material_score[opening][piece];
      score_endgame += eval_tables.material_score[endgame][piece];

      switch (piece) {
      case P:
        score_opening += eval_tables.positional_score[opening][PAWN][square];
        score_endgame += eval_tables.positional_score[endgame][PAWN][square];
        double_pawns = count_bits(board.bitboards[P] & file_masks[square]);

        if (double_pawns > 1) {
          score_opening += (double_pawns - 1) * eval_tables.double_pawn_penalty_opening;
          score_endgame += (double_pawns - 1) * eval_tables.double_pawn_penalty_endgame;
        }

        if ((board.bitboards[P] & isolated_masks[square]) == 0) {
          score_opening += eval_tables.isolated_pawn_penalty_opening;
          score_endgame += eval_tables.isolated_pawn_penalty_endgame;
        }

        if ((white_passed_masks[square] & board.bitboards[p]) == 0) {
          score_opening += eval_tables.passed_pawn_bonus[get_rank[square]];
          score_endgame += eval_tables.passed_pawn_bonus[get_rank[square]];
        }
        break;
      case N:
        score_opening += eval_tables.positional_score[opening][KNIGHT][square];
        score_endgame += eval_tables.positional_score[endgame][KNIGHT][square];
        break;
      case B:
        score_opening += eval_tables.positional_score[opening][BISHOP][square];
        score_endgame += eval_tables.positional_score[endgame][BISHOP][square];
        score_opening += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        break;
      case R:
        score_opening += eval_tables.positional_score[opening][ROOK][square];
        score_endgame += eval_tables.positional_score[endgame][ROOK][square];

        if ((board.bitboards[P] & file_masks[square]) == 0) {
          score_opening += eval_tables.semi_open_file_score;
          score_endgame += eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening += eval_tables.open_file_score;
          score_endgame += eval_tables.open_file_score;
        }
        break;
      case Q:
        score_opening += eval_tables.positional_score[opening][QUEEN][square];
        score_endgame += eval_tables.positional_score[endgame][QUEEN][square];
        score_opening += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        break;
      case K:
        score_opening += eval_tables.positional_score[opening][KING][square];
        score_endgame += eval_tables.positional_score[endgame][KING][square];

        if ((board.bitboards[P] & file_masks[square]) == 0) {
          score_opening -= eval_tables.semi_open_file_score;
          score_endgame -= eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening -= eval_tables.open_file_score;
          score_endgame -= eval_tables.open_file_score;
        }

        score_opening += count_bits(attack_tables.king_attacks[square] & board.occupancies[white]) * eval_tables.king_shield_bonus;
        score_endgame += count_bits(attack_tables.king_attacks[square] & board.occupancies[white]) * eval_tables.king_shield_bonus;
        break;
      case p:
        score_opening -= eval_tables.positional_score[opening][PAWN][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][PAWN][eval_tables.mirror_score[square]];
        double_pawns = count_bits(board.bitboards[p] & file_masks[square]);

        if (double_pawns > 1) {
          score_opening -= (double_pawns - 1) * eval_tables.double_pawn_penalty_opening;
          score_endgame -= (double_pawns - 1) * eval_tables.double_pawn_penalty_endgame;
        }

        if ((board.bitboards[p] & isolated_masks[square]) == 0) {
          score_opening -= eval_tables.isolated_pawn_penalty_opening;
          score_endgame -= eval_tables.isolated_pawn_penalty_endgame;
        }

        if ((black_passed_masks[square] & board.bitboards[P]) == 0) {
          score_opening -= eval_tables.passed_pawn_bonus[get_rank[square]];
          score_endgame -= eval_tables.passed_pawn_bonus[get_rank[square]];
        }
        break;
      case n:
        score_opening -= eval_tables.positional_score[opening][KNIGHT][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][KNIGHT][eval_tables.mirror_score[square]];
        break;
      case b:
        score_opening -= eval_tables.positional_score[opening][BISHOP][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][BISHOP][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        break;
      case r:
        score_opening -= eval_tables.positional_score[opening][ROOK][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][ROOK][eval_tables.mirror_score[square]];

        if ((board.bitboards[p] & file_masks[square]) == 0) {
          score_opening -= eval_tables.semi_open_file_score;
          score_endgame -= eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening -= eval_tables.open_file_score;
          score_endgame -= eval_tables.open_file_score;
        }
        break;
      case q:
        score_opening -= eval_tables.positional_score[opening][QUEEN][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][QUEEN][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        break;
      case k:
        score_opening -= eval_tables.positional_score[opening][KING][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][KING][eval_tables.mirror_score[square]];

        if ((board.bitboards[p] & file_masks[square]) == 0) {
          score_opening += eval_tables.semi_open_file_score;
          score_endgame += eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening += eval_tables.open_file_score;
          score_endgame += eval_tables.open_file_score;
        }
        
        score_opening -= count_bits(attack_tables.king_attacks[square] & board.occupancies[black]) * eval_tables.king_shield_bonus;
        score_endgame -= count_bits(attack_tables.king_attacks[square] & board.occupancies[black]) * eval_tables.king_shield_bonus;
        break;
      }
      pop_bit(bitboard, square);
    }
  }

  if (game_phase == middlegame)
    score = (score_opening * game_phase_score + score_endgame * (eval_tables.opening_phase_score - game_phase_score)) / eval_tables.opening_phase_score;

  else if (game_phase == opening)
    score = score_opening;

  else if (game_phase == endgame)
    score = score_endgame;

  return (board.side == white) ? score : -score;
}
