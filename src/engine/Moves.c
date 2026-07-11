#include "../../include/engine/Moves.h"

int make_move(int move, const int move_flag) {
  if (move_flag == all_moves) {
    copy_board();

    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    int capture = get_move_capture(move);
    int double_push = get_move_double(move);
    int enpass = get_move_enpassant(move);
    int castling = get_move_castling(move);
    const int reset_halfmove =
        piece == P || piece == p || capture || enpass;

    if (reset_halfmove)
      board.halfmove_clock = 0;
    else
      ++board.halfmove_clock;

    pop_bit(board.bitboards[piece], source_square);
    set_bit(board.bitboards[piece], target_square);

    // Keep the per-pawn "never moved" state exact across make/unmake (the
    // take_back macro restores it): the mover's source square is spent, and
    // any unmoved pawn sitting on the target square has just been captured.
    // No-op when the caller supplied no pawn state (pawn_unmoved == 0).
    pop_bit(board.pawn_unmoved, source_square);
    pop_bit(board.pawn_unmoved, target_square);

    if (board.side == white) {
      pop_bit(board.occupancies[white], source_square);
      set_bit(board.occupancies[white], target_square);
    } else {
      pop_bit(board.occupancies[black], source_square);
      set_bit(board.occupancies[black], target_square);
    }

    board.hash_key ^= zobrist_keys.piece_keys[piece][source_square];
    board.hash_key ^= zobrist_keys.piece_keys[piece][target_square];

    if (capture) {
      int start_piece, end_piece;
      if (board.side == white) {
        start_piece = p;
        end_piece = k;
      } else {
        start_piece = P;
        end_piece = K;
      }

      for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
        if (get_bit(board.bitboards[bb_piece], target_square)) {
          if (bb_piece == K || bb_piece == k) {
            take_back();
            return 0;
          }

          pop_bit(board.bitboards[bb_piece], target_square);
          if (board.side == white)
            pop_bit(board.occupancies[black], target_square);
          else
            pop_bit(board.occupancies[white], target_square);
          board.hash_key ^= zobrist_keys.piece_keys[bb_piece][target_square];
          break;
        }
      }
    }

    if (promoted_piece) {
      if (board.side == white) {
        pop_bit(board.bitboards[P], target_square);
        board.hash_key ^= zobrist_keys.piece_keys[P][target_square];
      } else {
        pop_bit(board.bitboards[p], target_square);
        board.hash_key ^= zobrist_keys.piece_keys[p][target_square];
      }

      set_bit(board.bitboards[promoted_piece], target_square);
      board.hash_key ^= zobrist_keys.piece_keys[promoted_piece][target_square];
    }

    if (enpass) {
      if (board.side == white) {
        pop_bit(board.bitboards[p], target_square + 8);
        pop_bit(board.occupancies[black], target_square + 8);
        pop_bit(board.pawn_unmoved, target_square + 8);
        board.hash_key ^= zobrist_keys.piece_keys[p][target_square + 8];
      } else {
        pop_bit(board.bitboards[P], target_square - 8);
        pop_bit(board.occupancies[white], target_square - 8);
        pop_bit(board.pawn_unmoved, target_square - 8);
        board.hash_key ^= zobrist_keys.piece_keys[P][target_square - 8];
      }
    }

    if (board.enpassant != no_sq)
      board.hash_key ^= zobrist_keys.enpassant_keys[board.enpassant];
    board.enpassant = no_sq;

    if (double_push) {
      if (board.side == white) {
        board.enpassant = target_square + 8;
        board.hash_key ^= zobrist_keys.enpassant_keys[target_square + 8];
      } else {
        board.enpassant = target_square - 8;
        board.hash_key ^= zobrist_keys.enpassant_keys[target_square - 8];
      }
    }

    if (castling) {
      switch (target_square) {
      case (g1):
        pop_bit(board.bitboards[R], h1);
        set_bit(board.bitboards[R], f1);
        pop_bit(board.occupancies[white], h1);
        set_bit(board.occupancies[white], f1);
        board.hash_key ^= zobrist_keys.piece_keys[R][h1];
        board.hash_key ^= zobrist_keys.piece_keys[R][f1];
        break;
      case (c1):
        pop_bit(board.bitboards[R], a1);
        set_bit(board.bitboards[R], d1);
        pop_bit(board.occupancies[white], a1);
        set_bit(board.occupancies[white], d1);
        board.hash_key ^= zobrist_keys.piece_keys[R][a1];
        board.hash_key ^= zobrist_keys.piece_keys[R][d1];
        break;
      case (g8):
        pop_bit(board.bitboards[r], h8);
        set_bit(board.bitboards[r], f8);
        pop_bit(board.occupancies[black], h8);
        set_bit(board.occupancies[black], f8);
        board.hash_key ^= zobrist_keys.piece_keys[r][h8];
        board.hash_key ^= zobrist_keys.piece_keys[r][f8];
        break;
      case (c8):
        pop_bit(board.bitboards[r], a8);
        set_bit(board.bitboards[r], d8);
        pop_bit(board.occupancies[black], a8);
        set_bit(board.occupancies[black], d8);
        board.hash_key ^= zobrist_keys.piece_keys[r][a8];
        board.hash_key ^= zobrist_keys.piece_keys[r][d8];
        break;
      }
    }

    board.hash_key ^= zobrist_keys.castle_keys[board.castle];
    board.castle &= castling_rights[source_square];
    board.castle &= castling_rights[target_square];
    board.hash_key ^= zobrist_keys.castle_keys[board.castle];

    board.occupancies[both] = board.occupancies[white] | board.occupancies[black];
    board.side ^= 1;
    board.hash_key ^= zobrist_keys.sidekey;

    // Mover's own king must not be left in check. A missing king (kingless
    // test FEN fed through UCI) can't be in check — get_ls1b_index on an
    // empty bitboard is undefined, so guard it.
    const u64 mover_king =
        (board.side == white) ? board.bitboards[k] : board.bitboards[K];
    if (mover_king != 0ULL &&
        is_square_attacked(get_ls1b_index(mover_king), board.side)) {
      take_back();
      return 0;
    } else
      return 1;
  } else {
    if (get_move_capture(move))
      return make_move(move, all_moves);
    else
      return 0;
  }
}

void generate_moves(MoveList* move_list) {
  move_list->count = 0;
  int source_square, target_square;
  u64 attacks = 0ULL;

  for (int piece = P; piece <= k; ++piece) {
    u64 bitboard = board.bitboards[piece];

    if (board.side == white) {
      if (piece == P) {
        while (bitboard) {
          source_square = get_ls1b_index(bitboard);
          target_square = source_square - 8;

          if (target_square >= a8 && !get_bit(board.occupancies[both], target_square)) {
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
              // Compound promotion targets (A/C/M) exist only under variant
              // rules, decided per moving side (side_variant) so a classic
              // side in a mixed game still promotes to Q/R/B/N only.
              if (board.side_variant[board.side]) {
                add_move(move_list, encode_move(source_square, target_square, piece, A, 0, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, C, 0, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, M, 0, 0, 0, 0));
              }
            } else {
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
              // Pawn double-step. In standard rules only from the home rank
              // (2nd). In variant rules a pawn may advance two squares from any
              // rank when both squares ahead are free: the engine is fed only a
              // FEN, which carries no per-pawn "has moved" flag, so the home rank
              // is the unmoved signal under standard play but is meaningless in a
              // custom setup where pawns start on arbitrary ranks. When the
              // caller supplies real per-piece state (has_pawn_state, from the
              // optional 8th FEN field), use it instead of guessing from rank —
              // it's the only way to tell a virgin custom-start pawn from one
              // that already spent its lifetime double-step off rank 2.
              int can_double;
              if (board.has_pawn_state) {
                can_double = get_bit(board.pawn_unmoved, source_square) != 0;
              } else {
                int on_home_rank = (source_square >= a2 && source_square <= h2);
                int variant_double = board.side_variant[board.side] && (target_square - 8) >= a8;
                can_double = on_home_rank || variant_double;
              }
              if (can_double && !get_bit(board.occupancies[both], target_square - 8))
                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
            }
          }

          attacks = attack_tables.pawn_attacks[board.side][source_square] & board.occupancies[black];
          while (attacks) {
            target_square = get_ls1b_index(attacks);
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
              // Compound promotion targets exist only in the variant (see above).
              if (board.side_variant[board.side]) {
                add_move(move_list, encode_move(source_square, target_square, piece, A, 1, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, C, 1, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, M, 1, 0, 0, 0));
              }
            } else
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
            pop_bit(attacks, target_square);
          }

          if (board.enpassant != no_sq) {
            u64 enpassant_attacks = attack_tables.pawn_attacks[board.side][source_square] & (1ULL << board.enpassant);
            if (enpassant_attacks) {
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(bitboard, source_square);
        }
      }

      if (piece == K) {
        if ((board.castle & wk) && !board.side_variant[board.side]) {
          if (!get_bit(board.occupancies[both], f1) && !get_bit(board.occupancies[both], g1)) {
            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black) && !is_square_attacked(g1, black))
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
          }
        }

        if ((board.castle & wq) && !board.side_variant[board.side]) {
          if (!get_bit(board.occupancies[both], d1) && !get_bit(board.occupancies[both], c1) && !get_bit(board.occupancies[both], b1)) {
            if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black) && !is_square_attacked(c1, black))
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    } else {
      if (piece == p) {
        while (bitboard) {
          source_square = get_ls1b_index(bitboard);
          target_square = source_square + 8;

          if (target_square <= h1 && !get_bit(board.occupancies[both], target_square)) {
            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
              // Compound promotion targets exist only in the variant (see above).
              if (board.side_variant[board.side]) {
                add_move(move_list, encode_move(source_square, target_square, piece, a, 0, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, c, 0, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, m, 0, 0, 0, 0));
              }
            } else {
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
              // Pawn double-step — see the white-pawn note above. Variant pawns
              // may advance two from any rank when both squares ahead are free.
              int can_double;
              if (board.has_pawn_state) {
                can_double = get_bit(board.pawn_unmoved, source_square) != 0;
              } else {
                int on_home_rank = (source_square >= a7 && source_square <= h7);
                int variant_double = board.side_variant[board.side] && (target_square + 8) <= h1;
                can_double = on_home_rank || variant_double;
              }
              if (can_double && !get_bit(board.occupancies[both], target_square + 8))
                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
            }
          }

          attacks = attack_tables.pawn_attacks[board.side][source_square] & board.occupancies[white];
          while (attacks) {
            target_square = get_ls1b_index(attacks);

            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
              // Compound promotion targets exist only in the variant (see above).
              if (board.side_variant[board.side]) {
                add_move(move_list, encode_move(source_square, target_square, piece, a, 1, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, c, 1, 0, 0, 0));
                add_move(move_list, encode_move(source_square, target_square, piece, m, 1, 0, 0, 0));
              }
            } else
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
            pop_bit(attacks, target_square);
          }

          if (board.enpassant != no_sq) {
            u64 enpassant_attacks = attack_tables.pawn_attacks[board.side][source_square] & (1ULL << board.enpassant);
            if (enpassant_attacks) {
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(bitboard, source_square);
        }
      }
      
      if (piece == k) {
        if ((board.castle & bk) && !board.side_variant[board.side]) {
          if (!get_bit(board.occupancies[both], f8) && !get_bit(board.occupancies[both], g8)) {
            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white) && !is_square_attacked(g8, white))
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
          }
        }

        if ((board.castle & bq) && !board.side_variant[board.side]) {
          if (!get_bit(board.occupancies[both], d8) && !get_bit(board.occupancies[both], c8) && !get_bit(board.occupancies[both], b8)) {
            if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white) && !is_square_attacked(c8, white))
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    if ((board.side == white) ? piece == N : piece == n) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = attack_tables.knight_attacks[source_square] & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == B : piece == b) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_bishop_attacks(source_square, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == A : piece == a) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_archbishop_attacks(source_square, board.occupancies[both]) &
                  ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == R : piece == r) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_rook_attacks(source_square, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == C : piece == c) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_chancellor_attacks(source_square, board.occupancies[both]) &
                  ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == Q : piece == q) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_queen_attacks(source_square, board.occupancies[both]) & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == M : piece == m) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = get_amazon_attacks(source_square, board.occupancies[both]) &
                  ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    if ((board.side == white) ? piece == K : piece == k) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);
        attacks = attack_tables.king_attacks[source_square] & ((board.side == white) ? ~board.occupancies[white] : ~board.occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);
          if (!get_bit(((board.side == white) ? board.occupancies[black] : board.occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }
  }
}
