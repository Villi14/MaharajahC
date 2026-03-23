#ifndef MOVES_H_
#define MOVES_H_

#include <assert.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "Attacks.h"
#include "Defines.h"
#include "Globals.h"
#include "Moves.h"
#include "Zobrist.h"
#include "Utils.h"

static inline void add_move(moves* move_list, int move) {
  move_list->moves[move_list->count] = move;
  ++move_list->count;
}

// make move on chess board
static inline int make_move(int move, const int move_flag) {
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

    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);

    // hash piece
    hash_key ^= piece_keys[piece][source_square]; // remove piece from source square in hash key
    hash_key ^= piece_keys[piece][target_square]; // set piece to the target square in hash key

    // handling capture moves
    if (capture) {
      // const int opponent_king = (side == white) ? k : K;

      // if (get_bit(bitboards[opponent_king], target_square)) {
      //   take_back();
      //   return 0;
      // }

      int start_piece, end_piece;

      if (side == white) {
        start_piece = p;
        end_piece = k;
      } else {
        start_piece = P;
        end_piece = K;
      }

      for (int bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece) {
        if (get_bit(bitboards[bb_piece], target_square)) {
          pop_bit(bitboards[bb_piece], target_square);

          // remove the piece from hash key
          hash_key ^= piece_keys[bb_piece][target_square];
          break;
        }
      }
    }

    // handle pawn promotions
    if (promoted_piece) {
      // white to move
      if (side == white) {
        // erase the pawn from the target square
        pop_bit(bitboards[P], target_square);

        // remove pawn from hash key
        hash_key ^= piece_keys[P][target_square];
      }

      // black to move
      else {
        // erase the pawn from the target square
        pop_bit(bitboards[p], target_square);

        // remove pawn from hash key
        hash_key ^= piece_keys[p][target_square];
      }

      // set up promoted piece on chess board
      set_bit(bitboards[promoted_piece], target_square);

      // add promoted piece into the hash key
      hash_key ^= piece_keys[promoted_piece][target_square];
    }

    // handle enpassant captures
    if (enpass) {
      // erase the pawn depending on side to move
      (side == white) ? pop_bit(bitboards[p], target_square + 8) : pop_bit(bitboards[P], target_square - 8);

      // white to move
      if (side == white) {
        // remove captured pawn
        pop_bit(bitboards[p], target_square + 8);

        // remove pawn from hash key
        hash_key ^= piece_keys[p][target_square + 8];
      }

      // black to move
      else {
        // remove captured pawn
        pop_bit(bitboards[P], target_square - 8);

        // remove pawn from hash key
        hash_key ^= piece_keys[P][target_square - 8];
      }
    }

    // hash enpassant if available (remove enpassant square from hash key )
    if (enpassant != no_sq)
      hash_key ^= enpassant_keys[enpassant];

    enpassant = no_sq;

    // handle double pawn push
    if (double_push) {
      // white to move
      if (side == white) {
        // set enpassant square
        enpassant = target_square + 8;

        // hash enpassant
        hash_key ^= enpassant_keys[target_square + 8];
      }

      // black to move
      else {
        // set enpassant square
        enpassant = target_square - 8;

        // hash enpassant
        hash_key ^= enpassant_keys[target_square - 8];
      }
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

        // hash rook
        hash_key ^= piece_keys[R][h1]; // remove rook from h1 from hash key
        hash_key ^= piece_keys[R][f1]; // put rook on f1 into a hash key
        break;

      // white castles queen side
      case (c1):
        // move A rook
        pop_bit(bitboards[R], a1);
        set_bit(bitboards[R], d1);

        // hash rook
        hash_key ^= piece_keys[R][a1]; // remove rook from a1 from hash key
        hash_key ^= piece_keys[R][d1]; // put rook on d1 into a hash key
        break;

      // black castles king side
      case (g8):
        // move H rook
        pop_bit(bitboards[r], h8);
        set_bit(bitboards[r], f8);

        // hash rook
        hash_key ^= piece_keys[r][h8]; // remove rook from h8 from hash key
        hash_key ^= piece_keys[r][f8]; // put rook on f8 into a hash key
        break;

      // black castles queen side
      case (c8):
        // move A rook
        pop_bit(bitboards[r], a8);
        set_bit(bitboards[r], d8);

        // hash rook
        hash_key ^= piece_keys[r][a8]; // remove rook from a8 from hash key
        hash_key ^= piece_keys[r][d8]; // put rook on d8 into a hash key
        break;
      }
    }

    // hash castling
    hash_key ^= castle_keys[castle];

    // update castling rights
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];

    // hash castling
    hash_key ^= castle_keys[castle];

    // TODO: why 24?
    memset(occupancies, 0ULL, 24);

    for (int bb_piece = P; bb_piece <= K; ++bb_piece)
      occupancies[white] |= bitboards[bb_piece];

    for (int bb_piece = p; bb_piece <= k; ++bb_piece)
      occupancies[black] |= bitboards[bb_piece];

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];

    side ^= 1;

    // hash side
    hash_key ^= side_key;

    // make sure that king has not been exposed into a check (illegal move)
    if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side)) {
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

// generate all moves
static inline void generate_moves(moves* move_list) {
  move_list->count = 0;

  int source_square, target_square;
  U64 attacks = 0ULL;

  //  int opponent_king_square = get_ls1b_index(bitboards[(side == white) ? k : K]);
  for (int piece = P; piece <= k; ++piece) {
    U64 bitboard = bitboards[piece];

    // generate white pawns & white king castling moves
    if (side == white) {
      if (piece == P) {
        while (bitboard) {
          source_square = get_ls1b_index(bitboard);
          target_square = source_square - 8;

          // generate quiet pawn moves
          if (target_square >= a8 && !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a7 && source_square <= h7) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
            } else {
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

              // two squares ahead pawn move
              if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
            }
          }

          attacks = pawn_attacks[side][source_square] & occupancies[black];

          // generate pawn captures
          while (attacks) {
            target_square = get_ls1b_index(attacks);

            // pawn promotion
            if (source_square >= a7 && source_square <= h7) {
              // if (target_square != opponent_king_square) {
              add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
              // }
            } else
              // one square ahead pawn move
              // if (target_square != opponent_king_square)
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq) {
            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

            if (enpassant_attacks) {
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(bitboard, source_square);
        }
      }

      // castling moves
      if (piece == K) {
        // king side castling is available
        if (castle & wk) {
          // make sure square between king and king's rook are empty
          if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1)) {
            // make sure king and the f1 squares are not under attacks
            if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black) && !is_square_attacked(g1, black))
              add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
          }
        }

        // queen side castling is available
        if (castle & wq) {
          // make sure square between king and queen's rook are empty
          if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1)) {
            // make sure king and the d1 squares are not under attacks
            if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black) && !is_square_attacked(c1, black))
              add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    // generate black pawns & black king castling moves
    else {
      if (piece == p) {
        while (bitboard) {
          source_square = get_ls1b_index(bitboard);
          target_square = source_square + 8;

          // generate quiet pawn moves
          if (target_square <= h1 && !get_bit(occupancies[both], target_square)) {
            // pawn promotion
            if (source_square >= a2 && source_square <= h2) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
            } else {
              // one square ahead pawn move
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
              // two squares ahead pawn move
              if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
            }
          }

          attacks = pawn_attacks[side][source_square] & occupancies[white];

          // generate pawn captures
          while (attacks) {
            target_square = get_ls1b_index(attacks);

            // pawn promotion
            if (source_square >= a2 && source_square <= h2) {
              // if (target_square != opponent_king_square) {
              add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
              add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
              // }
            } else
              // one square ahead pawn move
              // if (target_square != opponent_king_square)
              add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

            pop_bit(attacks, target_square);
          }

          // generate enpassant captures
          if (enpassant != no_sq) {
            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

            if (enpassant_attacks) {
              int target_enpassant = get_ls1b_index(enpassant_attacks);
              add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
            }
          }
          pop_bit(bitboard, source_square);
        }
      }

      // castling moves
      if (piece == k) {
        // king side castling is available
        if (castle & bk) {
          // make sure square between king and king's rook are empty
          if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8)) {
            // make sure king and the f8 squares are not under attacks
            if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white) && !is_square_attacked(g8, white))
              add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
          }
        }

        // queen side castling is available
        if (castle & bq) {
          // make sure square between king and queen's rook are empty
          if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8)) {
            // make sure king and the d8 squares are not under attacks
            if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white) && !is_square_attacked(c8, white))
              add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
          }
        }
      }
    }

    // generate knight moves
    if ((side == white) ? piece == N : piece == n) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);

        attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            // if (target_square != opponent_king_square)
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    // generate bishop moves
    if ((side == white) ? piece == B : piece == b) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);

        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            // if (target_square != opponent_king_square)
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    // generate rook moves
    if ((side == white) ? piece == R : piece == r) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);

        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            // if (target_square != opponent_king_square)
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    // generate queen moves
    if ((side == white) ? piece == Q : piece == q) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);

        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            //if (target_square != opponent_king_square)
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }

    // generate king moves
    if ((side == white) ? piece == K : piece == k) {
      while (bitboard) {
        source_square = get_ls1b_index(bitboard);

        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

        while (attacks) {
          target_square = get_ls1b_index(attacks);

          if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
          else
            // if (target_square != opponent_king_square)
            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

          pop_bit(attacks, target_square);
        }
        pop_bit(bitboard, source_square);
      }
    }
  }
}

#endif // !MOVES_H_
