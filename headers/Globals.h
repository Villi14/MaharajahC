#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Defines.h"

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// sides to move (colors)
enum { white, black, both };

// bishop and rook
enum { rook, bishop };

/*  bin   dec

   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side

   examples

   1111       both sides a castle both directions
   1001       black king => queen side
              white king => king side
*/
enum { wk = 0x1, wq = 0x2, bk = 0x4, bq = 0x8 };

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

// move types
enum { all_moves, only_captures };

// not A file constant
static const U64 not_a_file = 0xFEFEFEFEFEFEFEFEULL;

// not H file constant
static const U64 not_h_file = 0x7F7F7F7F7F7F7F7FULL;

// not HG file constant
static const U64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL;

// not AB file constant
static const U64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL;

// piece bitboards
extern U64 bitboards[0xC];

// occupancy bitboards
extern U64 occupancies[3];

// side to move
extern int side;

// enpassant square
extern int enpassant;

// castling rights
extern int castle;

// pseudo random number state
extern unsigned int random_state;

// pawn attacks table [side][square]
extern U64 pawn_attacks[2][0x40];

// knight attacks table [square]
extern U64 knight_attacks[0x40];

// king attacks table [square]
extern U64 king_attacks[0x40];

// bishop attack masks
extern U64 bishop_masks[0x40];

// rook attack masks
extern U64 rook_masks[0x40];

// bishop attacks table [square][occupancies]
extern U64 bishop_attacks[0x40][0x200];

// rook attacks table [square][occupancies]
extern U64 rook_attacks[0x40][0x1000];

// leaf nodes (number of positions reached during the test of the move generator at a given depth)
extern long nodes;

// convert squares to coordinates
extern const char* square_to_coordinates[0x40];

// default pieces
extern const char* default_pieces[0xC];

// convert ASCII character pieces to encoded constants
extern const int char_pieces[0x80];

// promoted pieces
extern const char promoted_pieces[0x80];

// bishop relevant occupancy bit count for every square on board
extern const int bishop_relevant_bits[0x40];

// rook relevant occupancy bit count for every square on board
extern const int rook_relevant_bits[0x40];

/*                         castling   move           in
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
extern const int castling_rights[0x40];

/*  ♙ =   100   = ♙
    ♘ =   300   = ♙ * 3
    ♗ =   350   = ♙ * 3 + ♙ * 0.5
    ♖ =   500   = ♙ * 5
    ♕ =   1000  = ♙ * 10
    ♔ =   10000 = ♙ * 100
*/
extern int material_score[0xC];

// pawn positional score
extern const int pawn_score[0x40];

// knight positional score
extern const int knight_score[0x40];

// bishop positional score
extern const int bishop_score[0x40];

// rook positional score
extern const int rook_score[0x40];

// king positional score
extern const int king_score[0x40];

// mirror positional score tables for opposite side
extern const int mirror_score[0x80];

#endif // !GLOBALS_H_