#include "maharajah/engine/Moves.h"

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

    pop_bit(board.bitboards[piece], source_square);
    set_bit(board.bitboards[piece], target_square);

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
        board.hash_key ^= zobrist_keys.piece_keys[p][target_square + 8];
      } else {
        pop_bit(board.bitboards[P], target_square - 8);
        pop_bit(board.occupancies[white], target_square - 8);
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

    if (is_square_attacked((board.side == white) ? get_ls1b_index(board.bitboards[k]) : get_ls1b_index(board.bitboards[K]), board.side)) {
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
            } else {
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
              if ((source_square >= a2 && source_square <= h2) && !get_bit(board.occupancies[both], target_square - 8))
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
        if (board.castle & wk) {
          if (!get_bit(board.occupancies[both], f1) && !get_bit(board.occupancies[both], g1)) {
            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black) && !is_square_attacked(g1, black))
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
          }
        }

        if (board.castle & wq) {
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
            } else {
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
              if ((source_square >= a7 && source_square <= h7) && !get_bit(board.occupancies[both], target_square + 8))
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
        if (board.castle & bk) {
          if (!get_bit(board.occupancies[both], f8) && !get_bit(board.occupancies[both], g8)) {
            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white) && !is_square_attacked(g8, white))
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
          }
        }

        if (board.castle & bq) {
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