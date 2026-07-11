#include <stdlib.h>

#include "../../include/engine/Evaluate.h"
#include "../../include/engine/Attacks.h"
#include "../../include/engine/EngineConfig.h"
#include "../../include/engine/EvalContext.h"
#include "../../include/engine/NNUE.h"

static int positional_value(int phase, int kind, int square, int is_black) {
  const int index = is_black ? eval_tables.mirror_score[square] : square;
  return eval_tables.positional_score[phase][kind][index];
}

static int piece_material_value(int piece) {
  return abs(eval_tables.material_score[opening][piece]);
}

static int attacker_count_on_square(int square, int side) {
  int attackers = 0;

  if (side == white) {
    attackers += count_bits(attack_tables.pawn_attacks[black][square] &
                            board.bitboards[P]);
    attackers += count_bits(attack_tables.knight_attacks[square] &
                            board.bitboards[N]);
    attackers += count_bits(get_bishop_attacks(square, board.occupancies[both]) &
                            board.bitboards[B]);
    attackers += count_bits(get_archbishop_attacks(square, board.occupancies[both]) &
                            board.bitboards[A]);
    attackers += count_bits(get_rook_attacks(square, board.occupancies[both]) &
                            board.bitboards[R]);
    attackers += count_bits(get_chancellor_attacks(square, board.occupancies[both]) &
                            board.bitboards[C]);
    attackers += count_bits(get_queen_attacks(square, board.occupancies[both]) &
                            board.bitboards[Q]);
    attackers += count_bits(get_amazon_attacks(square, board.occupancies[both]) &
                            board.bitboards[M]);
    attackers += count_bits(attack_tables.king_attacks[square] &
                            board.bitboards[K]);
  } else {
    attackers += count_bits(attack_tables.pawn_attacks[white][square] &
                            board.bitboards[p]);
    attackers += count_bits(attack_tables.knight_attacks[square] &
                            board.bitboards[n]);
    attackers += count_bits(get_bishop_attacks(square, board.occupancies[both]) &
                            board.bitboards[b]);
    attackers += count_bits(get_archbishop_attacks(square, board.occupancies[both]) &
                            board.bitboards[a]);
    attackers += count_bits(get_rook_attacks(square, board.occupancies[both]) &
                            board.bitboards[r]);
    attackers += count_bits(get_chancellor_attacks(square, board.occupancies[both]) &
                            board.bitboards[c]);
    attackers += count_bits(get_queen_attacks(square, board.occupancies[both]) &
                            board.bitboards[q]);
    attackers += count_bits(get_amazon_attacks(square, board.occupancies[both]) &
                            board.bitboards[m]);
    attackers += count_bits(attack_tables.king_attacks[square] &
                            board.bitboards[k]);
  }

  return attackers;
}

static int least_attacker_value_on_square(int square, int side) {
  if (side == white) {
    if (attack_tables.pawn_attacks[black][square] & board.bitboards[P])
      return piece_material_value(P);
    if (attack_tables.knight_attacks[square] & board.bitboards[N])
      return piece_material_value(N);
    if (get_bishop_attacks(square, board.occupancies[both]) & board.bitboards[B])
      return piece_material_value(B);
    if (get_archbishop_attacks(square, board.occupancies[both]) & board.bitboards[A])
      return piece_material_value(A);
    if (get_rook_attacks(square, board.occupancies[both]) & board.bitboards[R])
      return piece_material_value(R);
    if (get_chancellor_attacks(square, board.occupancies[both]) & board.bitboards[C])
      return piece_material_value(C);
    if (get_queen_attacks(square, board.occupancies[both]) & board.bitboards[Q])
      return piece_material_value(Q);
    if (get_amazon_attacks(square, board.occupancies[both]) & board.bitboards[M])
      return piece_material_value(M);
    if (attack_tables.king_attacks[square] & board.bitboards[K])
      return piece_material_value(K);
  } else {
    if (attack_tables.pawn_attacks[white][square] & board.bitboards[p])
      return piece_material_value(p);
    if (attack_tables.knight_attacks[square] & board.bitboards[n])
      return piece_material_value(n);
    if (get_bishop_attacks(square, board.occupancies[both]) & board.bitboards[b])
      return piece_material_value(b);
    if (get_archbishop_attacks(square, board.occupancies[both]) & board.bitboards[a])
      return piece_material_value(a);
    if (get_rook_attacks(square, board.occupancies[both]) & board.bitboards[r])
      return piece_material_value(r);
    if (get_chancellor_attacks(square, board.occupancies[both]) & board.bitboards[c])
      return piece_material_value(c);
    if (get_queen_attacks(square, board.occupancies[both]) & board.bitboards[q])
      return piece_material_value(q);
    if (get_amazon_attacks(square, board.occupancies[both]) & board.bitboards[m])
      return piece_material_value(m);
    if (attack_tables.king_attacks[square] & board.bitboards[k])
      return piece_material_value(k);
  }

  return 0;
}

static int piece_safety_penalty(int piece, int square) {
  if (piece == K || piece == k)
    return 0;

  const int own_side = piece < white_piece_count ? white : black;
  const int enemy_side = own_side ^ 1;
  const int enemy_attackers = attacker_count_on_square(square, enemy_side);

  if (enemy_attackers == 0)
    return 0;

  const int own_defenders = attacker_count_on_square(square, own_side);
  const int defended = own_defenders > 0;
  const int material_value = piece_material_value(piece);
  const int least_attacker_value =
      least_attacker_value_on_square(square, enemy_side);
  int penalty = 0;

  switch (piece) {
  case Q:
  case q:
  case M:
  case m:
    penalty = defended ? material_value / 10 : material_value / 4;
    break;
  case R:
  case r:
  case C:
  case c:
    penalty = defended ? material_value / 12 : material_value / 5;
    break;
  case B:
  case b:
  case N:
  case n:
  case A:
  case a:
    penalty = defended ? material_value / 14 : material_value / 6;
    break;
  case P:
  case p:
    penalty = defended ? material_value / 18 : material_value / 10;
    break;
  default:
    penalty = defended ? material_value / 16 : material_value / 8;
    break;
  }

  if (enemy_attackers > own_defenders) {
    penalty += (material_value * (enemy_attackers - own_defenders)) /
               ((piece == Q || piece == q || piece == M || piece == m) ? 14 : 18);
  }

  if (!defended) {
    penalty += material_value / 12;
  }

  if (least_attacker_value > 0 && least_attacker_value < material_value) {
    penalty += (material_value - least_attacker_value) / 10;

    if (!defended) {
      penalty += (material_value - least_attacker_value) / 12;
    } else if (enemy_attackers >= own_defenders) {
      penalty += (material_value - least_attacker_value) / 16;
    }
  }

  switch (piece) {
  case Q:
  case q:
  case M:
  case m:
    if (penalty > (material_value * 2) / 5)
      penalty = (material_value * 2) / 5;
    break;
  case R:
  case r:
  case C:
  case c:
    if (penalty > material_value / 3)
      penalty = material_value / 3;
    break;
  case B:
  case b:
  case N:
  case n:
  case A:
  case a:
    if (penalty > material_value / 4)
      penalty = material_value / 4;
    break;
  case P:
  case p:
    if (penalty > material_value / 5)
      penalty = material_value / 5;
    break;
  default:
    break;
  }

  return penalty;
}

static int archbishop_positional_value(int phase, int square, int is_black) {
  return positional_value(phase, BISHOP, square, is_black) +
         positional_value(phase, KNIGHT, square, is_black);
}

static int chancellor_positional_value(int phase, int square, int is_black) {
  return positional_value(phase, ROOK, square, is_black) +
         positional_value(phase, KNIGHT, square, is_black);
}

static int amazon_positional_value(int phase, int square, int is_black) {
  return positional_value(phase, QUEEN, square, is_black) +
         positional_value(phase, KNIGHT, square, is_black);
}

int evaluate_classic(void) {
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
  int white_bishops = 0;
  int black_bishops = 0;

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

        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case N:
        score_opening += eval_tables.positional_score[opening][KNIGHT][square];
        score_endgame += eval_tables.positional_score[endgame][KNIGHT][square];
        score_opening += (count_bits(attack_tables.knight_attacks[square] & ~board.occupancies[white]) - 4) *
                         engine_eval_config.knight_mobility_opening;
        score_endgame += (count_bits(attack_tables.knight_attacks[square] & ~board.occupancies[white]) - 4) *
                         engine_eval_config.knight_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case B:
        ++white_bishops;
        score_opening += eval_tables.positional_score[opening][BISHOP][square];
        score_endgame += eval_tables.positional_score[endgame][BISHOP][square];
        score_opening += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame += (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case R:
        score_opening += eval_tables.positional_score[opening][ROOK][square];
        score_endgame += eval_tables.positional_score[endgame][ROOK][square];
        score_opening += (count_bits(get_rook_attacks(square, board.occupancies[both])) - 7) *
                         engine_eval_config.rook_mobility_opening;
        score_endgame += (count_bits(get_rook_attacks(square, board.occupancies[both])) - 7) *
                         engine_eval_config.rook_mobility_endgame;

        if ((board.bitboards[P] & file_masks[square]) == 0) {
          score_opening += eval_tables.semi_open_file_score;
          score_endgame += eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening += eval_tables.open_file_score;
          score_endgame += eval_tables.open_file_score;
        }

        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case Q:
        score_opening += eval_tables.positional_score[opening][QUEEN][square];
        score_endgame += eval_tables.positional_score[endgame][QUEEN][square];
        score_opening += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame += (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
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
      case A:
        score_opening += archbishop_positional_value(opening, square, 0);
        score_endgame += archbishop_positional_value(endgame, square, 0);
        score_opening += (count_bits(get_archbishop_attacks(square, board.occupancies[both])) -
                          (eval_tables.bishop_unit + 4)) * eval_tables.bishop_mobility_opening;
        score_endgame += (count_bits(get_archbishop_attacks(square, board.occupancies[both])) -
                          (eval_tables.bishop_unit + 4)) * eval_tables.bishop_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case C:
        score_opening += chancellor_positional_value(opening, square, 0);
        score_endgame += chancellor_positional_value(endgame, square, 0);
        score_opening += (count_bits(get_chancellor_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit - 1)) * eval_tables.queen_mobility_opening;
        score_endgame += (count_bits(get_chancellor_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit - 1)) * eval_tables.queen_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
        break;
      case M:
        score_opening += amazon_positional_value(opening, square, 0);
        score_endgame += amazon_positional_value(endgame, square, 0);
        score_opening += (count_bits(get_amazon_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit + 4)) * eval_tables.queen_mobility_opening;
        score_endgame += (count_bits(get_amazon_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit + 4)) * eval_tables.queen_mobility_endgame;
        score_opening -= piece_safety_penalty(piece, square);
        score_endgame -= piece_safety_penalty(piece, square);
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
          score_opening -= eval_tables.passed_pawn_bonus[get_rank[eval_tables.mirror_score[square]]];
          score_endgame -= eval_tables.passed_pawn_bonus[get_rank[eval_tables.mirror_score[square]]];
        }

        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case n:
        score_opening -= eval_tables.positional_score[opening][KNIGHT][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][KNIGHT][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(attack_tables.knight_attacks[square] & ~board.occupancies[black]) - 4) *
                         engine_eval_config.knight_mobility_opening;
        score_endgame -= (count_bits(attack_tables.knight_attacks[square] & ~board.occupancies[black]) - 4) *
                         engine_eval_config.knight_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case b:
        ++black_bishops;
        score_opening -= eval_tables.positional_score[opening][BISHOP][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][BISHOP][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_opening;
        score_endgame -= (count_bits(get_bishop_attacks(square, board.occupancies[both])) - eval_tables.bishop_unit) * eval_tables.bishop_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case r:
        score_opening -= eval_tables.positional_score[opening][ROOK][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][ROOK][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(get_rook_attacks(square, board.occupancies[both])) - 7) *
                         engine_eval_config.rook_mobility_opening;
        score_endgame -= (count_bits(get_rook_attacks(square, board.occupancies[both])) - 7) *
                         engine_eval_config.rook_mobility_endgame;

        if ((board.bitboards[p] & file_masks[square]) == 0) {
          score_opening -= eval_tables.semi_open_file_score;
          score_endgame -= eval_tables.semi_open_file_score;
        }

        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
          score_opening -= eval_tables.open_file_score;
          score_endgame -= eval_tables.open_file_score;
        }

        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case q:
        score_opening -= eval_tables.positional_score[opening][QUEEN][eval_tables.mirror_score[square]];
        score_endgame -= eval_tables.positional_score[endgame][QUEEN][eval_tables.mirror_score[square]];
        score_opening -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_opening;
        score_endgame -= (count_bits(get_queen_attacks(square, board.occupancies[both])) - eval_tables.queen_unit) * eval_tables.queen_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
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
      case a:
        score_opening -= archbishop_positional_value(opening, square, 1);
        score_endgame -= archbishop_positional_value(endgame, square, 1);
        score_opening -= (count_bits(get_archbishop_attacks(square, board.occupancies[both])) -
                          (eval_tables.bishop_unit + 4)) * eval_tables.bishop_mobility_opening;
        score_endgame -= (count_bits(get_archbishop_attacks(square, board.occupancies[both])) -
                          (eval_tables.bishop_unit + 4)) * eval_tables.bishop_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case c:
        score_opening -= chancellor_positional_value(opening, square, 1);
        score_endgame -= chancellor_positional_value(endgame, square, 1);
        score_opening -= (count_bits(get_chancellor_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit - 1)) * eval_tables.queen_mobility_opening;
        score_endgame -= (count_bits(get_chancellor_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit - 1)) * eval_tables.queen_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      case m:
        score_opening -= amazon_positional_value(opening, square, 1);
        score_endgame -= amazon_positional_value(endgame, square, 1);
        score_opening -= (count_bits(get_amazon_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit + 4)) * eval_tables.queen_mobility_opening;
        score_endgame -= (count_bits(get_amazon_attacks(square, board.occupancies[both])) -
                          (eval_tables.queen_unit + 4)) * eval_tables.queen_mobility_endgame;
        score_opening += piece_safety_penalty(piece, square);
        score_endgame += piece_safety_penalty(piece, square);
        break;
      }
      pop_bit(bitboard, square);
    }
  }

  if (white_bishops >= 2) {
    score_opening += engine_eval_config.bishop_pair_bonus_opening;
    score_endgame += engine_eval_config.bishop_pair_bonus_endgame;
  }

  if (black_bishops >= 2) {
    score_opening -= engine_eval_config.bishop_pair_bonus_opening;
    score_endgame -= engine_eval_config.bishop_pair_bonus_endgame;
  }

  if (game_phase == middlegame)
    score = (score_opening * game_phase_score + score_endgame * (eval_tables.opening_phase_score - game_phase_score)) / eval_tables.opening_phase_score;

  else if (game_phase == opening)
    score = score_opening;

  else if (game_phase == endgame)
    score = score_endgame;

  score += board.side == white ? engine_eval_config.tempo_bonus : -engine_eval_config.tempo_bonus;

  return (board.side == white) ? score : -score;
}

int evaluate_with_mode(int eval_mode) {
  if (clamp_eval_mode(eval_mode) == eval_mode_nnue && nnue_backend_is_ready()) {
    return evaluate_nnue();
  }

  return evaluate_classic();
}

int evaluate(void) {
  update_eval_context();
  return evaluate_with_mode(engine_eval_config.eval_mode);
}
