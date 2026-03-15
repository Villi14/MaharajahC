#include <stdio.h>
#include <string.h>

#include "Functions.h"
#include "Globals.h"
#include "Attacks.h"

// count bits within a bitboard (Brian Kernighan's way)
int count_bits(U64 bitboard) {
  // a bit counter
  int count = 0;

  // consecutively reset least significant 1st bit
  while (bitboard) {
    // increment count
    ++count;

    // reset least significant 1st bit
    bitboard &= bitboard - 1;
  }

  // return bit count
  return count;
}

// get least significant 1st bit index
int get_ls1b_index(U64 bitboard) {
  // make sure bitboard is not 0
  if (bitboard) {
    // count trailing bits before LS1B
    return count_bits((bitboard & -bitboard) - 1);
  }

  // otherwise
  else
    // return illegal index
    return -1;
}

// set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  // occupancy map
  U64 occupancy = 0ULL;

  // loop over the range of bits within attack mask
  for (int count = 0; count < bits_in_mask; ++count) {
    // get LS1B index of attacks mask
    int square = get_ls1b_index(attack_mask);

    // pop LS1B in attack map
    pop_bit(attack_mask, square);

    // make sure occupancy is on board
    if (index & (1 << count))
      // populate occupancy map
      occupancy |= (1ULL << square);
  }

  // return occupancy map
  return occupancy;
}

// add move to the move list
void add_move(moves *move_list, int move) {
  // store move
  move_list->moves[move_list->count] = move;

  // increment move count
  ++move_list->count;
}

// print move (for UCI purposes)
void print_move(int move) {
  printf("%s%s%c\n", square_to_coordinates[get_move_source(move)],
         square_to_coordinates[get_move_target(move)],
         promoted_pieces[get_move_promoted(move)]);
}

// make move on chess board
int make_move(int move, int move_flag) {
  // quiet moves
  if (move_flag == all_moves) {
    // preserve board state
    copy_board();

    // parse move
    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    int capture = get_move_capture(move);
    int double_push = get_move_double(move);
    int enpass = get_move_enpassant(move);
    int castling = get_move_castling(move);

    // move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);

    // handling capture moves
    if (capture) {
      // pick up bitboard piece index ranges depending on side
      int start_piece, end_piece;

      // white to move
      if (side == white) {
        start_piece = p;
        end_piece = k;
      }

      // black to move
      else {
        start_piece = P;
        end_piece = K;
      }

      // loop over bitboards opposite to the current side to move
      for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
        // if there's a piece on the target square
        if (get_bit(bitboards[bb_piece], target_square)) {
          // remove it from corresponding bitboard
          pop_bit(bitboards[bb_piece], target_square);
          break;
        }
      }
    }

    // handle pawn promotions
    if (promoted_piece) {
      // erase the pawn from the target square
      pop_bit(bitboards[(side == white) ? P : p], target_square);

      // set up promoted piece on chess board
      set_bit(bitboards[promoted_piece], target_square);
    }

    // handle enpassant captures
    if (enpass) {
      // erase the pawn depending on side to move
      (side == white) ? pop_bit(bitboards[p], target_square + 8)
                      : pop_bit(bitboards[P], target_square - 8);
    }

    // reset enpassant square
    enpassant = no_sq;

    // handle double pawn push
    if (double_push) {
      // set enpassant square depending on side to move
      (side == white) ? (enpassant = target_square + 8)
                      : (enpassant = target_square - 8);
    }

    // handle castling moves
    if (castling) {
      // switch target square
      switch (target_square) {
      // white castles king side
      case (g1):
        // move H rook
        pop_bit(bitboards[R], h1);
        set_bit(bitboards[R], f1);
        break;

      // white castles queen side
      case (c1):
        // move A rook
        pop_bit(bitboards[R], a1);
        set_bit(bitboards[R], d1);
        break;

      // black castles king side
      case (g8):
        // move H rook
        pop_bit(bitboards[r], h8);
        set_bit(bitboards[r], f8);
        break;

      // black castles queen side
      case (c8):
        // move A rook
        pop_bit(bitboards[r], a8);
        set_bit(bitboards[r], d8);
        break;
      default:
          break;
      }
    }

    // update castling rights
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];

    // reset occupancies
    memset(occupancies, 0ULL, 24);

    // loop over white pieces bitboards
    for (int bb_piece = P; bb_piece <= K; ++bb_piece)
      // update white occupancies
      occupancies[white] |= bitboards[bb_piece];

    // loop over black pieces bitboards
    for (int bb_piece = p; bb_piece <= k; ++bb_piece)
      // update black occupancies
      occupancies[black] |= bitboards[bb_piece];

    // update both sides occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    // change side
    side ^= 1;

    // make sure that king has not been exposed into a check
    if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k])
                                           : get_ls1b_index(bitboards[K]),
                           side)) {
      // take move back
      take_back();

      // return illegal move
      return 0;
    }

    //
    else
      // return legal move
      return 1;

  }

  // capture moves
  else {
    // make sure move is the capture
    if (get_move_capture(move))
      make_move(move, all_moves);

    // otherwise the move is not a capture
    else
      // don't make it
      return 0;
  }
  return 0;
}

// generate all moves
void generate_moves(moves *move_list) {
  // init move count
  move_list->count = 0;

  // define source & target squares
  int source_square, target_square;

  // define current piece's bitboard copy & it's attacks
  U64 attacks;

  // loop over all the bitboards
  for (int piece = P; piece <= k; ++piece) {
    // init piece bitboard copy
    U64 bitboard = bitboards[piece];

    // generate white pawns & white king castling moves
    if (side == white) {
      // pick up white pawn bitboards index
      if (piece == P) {
        // loop over white pawns within white pawn bitboard
        while (bitboard) {
          // init source square
          source_square = get_ls1b_index(bitboard);

          // init target square
          target_square = source_square - 8;

          // generate quiet pawn moves
          if (target_square >= a8 &&
              !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, B, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, N, 0, 0, 0, 0));
            }

            else {
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, 0, 0, 0, 0, 0));

              // two squares ahead pawn move
              if ((source_square >= a2 && source_square <= h2) &&
                  !get_bit(occupancies[both], target_square - 8))
                add_move(move_list,
                         encode_move(source_square, target_square - 8, piece, 0,
                                     0, 1, 0, 0));
            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[black];

          // generate pawn captures
          while (attacks) {
            // init target square
            target_square = get_ls1b_index(attacks);

            // pawn promotion
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, B, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, N, 1, 0, 0, 0));
            }

            else
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, 0, 1, 0, 0, 0));

            // pop ls1b of the pawn attacks
            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq) {
            // lookup pawn attacks and bitwise AND with enpassant square (bit)
            U64 enpassant_attacks =
                pawn_attacks[side][source_square] & (1ULL << enpassant);

            // make sure enpassant capture available
            if (enpassant_attacks) {
              // init enpassant capture target square
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant,
                                              piece, 0, 1, 0, 1, 0));
            }
          }

          // pop ls1b from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
      }

      // castling moves
      if (piece == K) {
        // king side castling is available
        if (castle & wk) {
          // make sure square between king and king's rook are empty
          if (!get_bit(occupancies[both], f1) &&
              !get_bit(occupancies[both], g1)) {
            // make sure king and the f1 squares are not under attacks
            if (!is_square_attacked(e1, black) &&
                !is_square_attacked(f1, black))
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
          }
        }

        // queen side castling is available
        if (castle & wq) {
          // make sure square between king and queen's rook are empty
          if (!get_bit(occupancies[both], d1) &&
              !get_bit(occupancies[both], c1) &&
              !get_bit(occupancies[both], b1)) {
            // make sure king and the d1 squares are not under attacks
            if (!is_square_attacked(e1, black) &&
                !is_square_attacked(d1, black))
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    // generate black pawns & black king castling moves
    else {
      // pick up black pawn bitboards index
      if (piece == p) {
        // loop over white pawns within white pawn bitboard
        while (bitboard) {
          // init source square
          source_square = get_ls1b_index(bitboard);

          // init target square
          target_square = source_square + 8;

          // generate quiet pawn moves
          if (target_square <= h1 &&
              !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, b, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, n, 0, 0, 0, 0));
            }

            else {
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, 0, 0, 0, 0, 0));
              // two squares ahead pawn move
              if ((source_square >= a7 && source_square <= h7) &&
                  !get_bit(occupancies[both], target_square + 8))
                add_move(move_list,
                         encode_move(source_square, target_square + 8, piece, 0,
                                     0, 1, 0, 0));
            }
          }

          // init pawn attacks bitboard
          attacks = pawn_attacks[side][source_square] & occupancies[white];

          // generate pawn captures
          while (attacks) {
            // init target square
            target_square = get_ls1b_index(attacks);

            // pawn promotion
            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, b, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, n, 1, 0, 0, 0));
            }

            else
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square,
                                              piece, 0, 1, 0, 0, 0));

            // pop ls1b of the pawn attacks
            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq) {
            // lookup pawn attacks and bitwise AND with enpassant square (bit)
            U64 enpassant_attacks =
                pawn_attacks[side][source_square] & (1ULL << enpassant);

            // make sure enpassant capture available
            if (enpassant_attacks) {
              // init enpassant capture target square
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant,
                                              piece, 0, 1, 0, 1, 0));
            }
          }

          // pop ls1b from piece bitboard copy
          pop_bit(bitboard, source_square);
        }
      }

      // castling moves
      if (piece == k) {
        // king side castling is available
        if (castle & bk) {
          // make sure square between king and king's rook are empty
          if (!get_bit(occupancies[both], f8) &&
              !get_bit(occupancies[both], g8)) {
            // make sure king and the f8 squares are not under attacks
            if (!is_square_attacked(e8, white) &&
                !is_square_attacked(f8, white))
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
          }
        }

        // queen side castling is available
        if (castle & bq) {
          // make sure square between king and queen's rook are empty
          if (!get_bit(occupancies[both], d8) &&
              !get_bit(occupancies[both], c8) &&
              !get_bit(occupancies[both], b8)) {
            // make sure king and the d8 squares are not under attacks
            if (!is_square_attacked(e8, white) &&
                !is_square_attacked(d8, white))
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    // generate knight moves
    if ((side == white) ? piece == N : piece == n) {
      // loop over source squares of piece bitboard copy
      while (bitboard) {
        // init source square
        source_square = get_ls1b_index(bitboard);

        // init piece attacks in order to get set of target squares
        attacks = knight_attacks[source_square] &
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks) {
          // init target square
          target_square = get_ls1b_index(attacks);

          // quiet move
          if (!get_bit(
                  ((side == white) ? occupancies[black] : occupancies[white]),
                  target_square))
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 0, 0, 0, 0));

          else
            // capture move
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 1, 0, 0, 0));

          // pop ls1b in current attacks set
          pop_bit(attacks, target_square);
        }

        // pop ls1b of the current piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate bishop moves
    if ((side == white) ? piece == B : piece == b) {
      // loop over source squares of piece bitboard copy
      while (bitboard) {
        // init source square
        source_square = get_ls1b_index(bitboard);

        // init piece attacks in order to get set of target squares
        attacks = get_bishop_attacks(source_square, occupancies[both]) &
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks) {
          // init target square
          target_square = get_ls1b_index(attacks);

          // quiet move
          if (!get_bit(
                  ((side == white) ? occupancies[black] : occupancies[white]),
                  target_square))
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 0, 0, 0, 0));

          else
            // capture move
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 1, 0, 0, 0));

          // pop ls1b in current attacks set
          pop_bit(attacks, target_square);
        }

        // pop ls1b of the current piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate rook moves
    if ((side == white) ? piece == R : piece == r) {
      // loop over source squares of piece bitboard copy
      while (bitboard) {
        // init source square
        source_square = get_ls1b_index(bitboard);

        // init piece attacks in order to get set of target squares
        attacks = get_rook_attacks(source_square, occupancies[both]) &
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks) {
          // init target square
          target_square = get_ls1b_index(attacks);

          // quiet move
          if (!get_bit(
                  ((side == white) ? occupancies[black] : occupancies[white]),
                  target_square))
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 0, 0, 0, 0));

          else
            // capture move
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 1, 0, 0, 0));

          // pop ls1b in current attacks set
          pop_bit(attacks, target_square);
        }

        // pop ls1b of the current piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate queen moves
    if ((side == white) ? piece == Q : piece == q) {
      // loop over source squares of piece bitboard copy
      while (bitboard) {
        // init source square
        source_square = get_ls1b_index(bitboard);

        // init piece attacks in order to get set of target squares
        attacks = get_queen_attacks(source_square, occupancies[both]) &
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks) {
          // init target square
          target_square = get_ls1b_index(attacks);

          // quiet move
          if (!get_bit(
                  ((side == white) ? occupancies[black] : occupancies[white]),
                  target_square))
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 0, 0, 0, 0));

          else
            // capture move
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 1, 0, 0, 0));

          // pop ls1b in current attacks set
          pop_bit(attacks, target_square);
        }

        // pop ls1b of the current piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }

    // generate king moves
    if ((side == white) ? piece == K : piece == k) {
      // loop over source squares of piece bitboard copy
      while (bitboard) {
        // init source square
        source_square = get_ls1b_index(bitboard);

        // init piece attacks in order to get set of target squares
        attacks = king_attacks[source_square] &
                  ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        // loop over target squares available from generated attacks
        while (attacks) {
          // init target square
          target_square = get_ls1b_index(attacks);

          // quiet move
          if (!get_bit(
                  ((side == white) ? occupancies[black] : occupancies[white]),
                  target_square))
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 0, 0, 0, 0));

          else
            // capture move
            add_move(move_list, encode_move(source_square, target_square, piece,
                                            0, 1, 0, 0, 0));

          // pop ls1b in current attacks set
          pop_bit(attacks, target_square);
        }

        // pop ls1b of the current piece bitboard copy
        pop_bit(bitboard, source_square);
      }
    }
  }
}
