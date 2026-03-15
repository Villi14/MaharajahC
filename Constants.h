#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "Defines.h"

// board squares
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// sides to move (colors)
 enum { white, black, both };

// bishop and rook
 enum { rook, bishop };

/*
    bin  dec

   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side

   examples

   1111       both sides a castle both directions
   1001       black king => queen side
              white king => king side
*/

 enum { wk = 1, wq = 2, bk = 4, bq = 8 };

// convert squares to coordinates
extern const char *square_to_coordinates[64];

// ASCII pieces
static const char ascii_pieces[12] = "PNBRQKpnbrqk";

// Unicode pieces
static const char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

// convert ASCII character pieces to encoded constants
extern const int char_pieces[128];

// promoted pieces
static const char promoted_pieces[] = {
  [Q] = 'q',
  [R] = 'r',
  [B] = 'b',
  [N] = 'n',
  [q] = 'q',
  [r] = 'r',
  [b] = 'b',
  [n] = 'n'
};

// not A file constant
static const U64 not_a_file = 18374403900871474942ULL;

// not H file constant
static const U64 not_h_file = 9187201950435737471ULL;

// not HG file constant
static const U64 not_hg_file = 4557430888798830399ULL;

// not AB file constant
static const U64 not_ab_file = 18229723555195321596ULL;

// move types
enum { all_moves, only_captures };

// bishop relevant occupancy bit count for every square on board
extern const int bishop_relevant_bits[64];

// rook relevant occupancy bit count for every square on board
extern const int rook_relevant_bits[64];

/*                         castling   move     in      in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13

         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7
*/

// castling rights update constants
extern const int castling_rights[64];

#endif // CONSTANTS_H_