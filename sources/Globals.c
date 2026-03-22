#include "../headers/Globals.h"
#include "../headers/Defines.h"

int side;
int enpassant = no_sq;
int castle;
unsigned int random_state = 0x6B8B4567;
long nodes;
int ply;
int best_move;

U64 bitboards[0xC];
U64 occupancies[3];
U64 pawn_attacks[2][0x40];
U64 knight_attacks[0x40];
U64 king_attacks[0x40];
U64 bishop_masks[0x40];
U64 rook_masks[0x40];
U64 bishop_attacks[0x40][0x200];
U64 rook_attacks[0x40][0x1000];
int killer_moves[2][0x40];
int history_moves[12][0x40];

// convert squares to coordinates
const char *square_to_coordinates[0x40] = {
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

#ifdef _MSC_VER
const char *default_pieces[0xC] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};
#else
const char *default_pieces[0xC] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};
#endif

// convert ASCII character pieces to encoded constants
const int char_pieces[0x80] = {
  ['P'] = P,
  ['N'] = N,
  ['B'] = B,
  ['R'] = R,
  ['Q'] = Q,
  ['K'] = K,
  ['p'] = p,
  ['n'] = n,
  ['b'] = b,
  ['r'] = r,
  ['q'] = q,
  ['k'] = k
};

const char promoted_pieces[0x80] = {
  [Q] = 'q',
  [R] = 'r',
  [B] = 'b',
  [N] = 'n',
  [q] = 'q',
  [r] = 'r',
  [b] = 'b',
  [n] = 'n'
};

// bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[0x40] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[0x40] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

// castling rights update constants
const int castling_rights[0x40] = {
   7, 15, 15, 15,  3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

int material_score[0xC] = {
    100,      // white pawn score
    300,      // white knight score
    350,      // white bishop score
    500,      // white rook score
   1000,      // white queen score
  10000,      // white king score
   -100,      // black pawn score
   -300,      // black knight score
   -350,      // black bishop score
   -500,      // black rook score
  -1000,      // black queen score
 -10000,      // black king score
};

// pawn positional score
const int pawn_score[0x40] = {
  90,  90,  90,  90,  90,  90,  90,  90,
  30,  30,  30,  40,  40,  30,  30,  30,
  20,  20,  20,  30,  30,  30,  20,  20,
  10,  10,  10,  20,  20,  10,  10,  10,
   5,   5,  10,  20,  20,   5,   5,   5,
   0,   0,   0,   5,   5,   0,   0,   0,
   0,   0,   0, -10, -10,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[0x40] = {
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,  10,  10,   0,   0,  -5,
  -5,   5,  20,  20,  20,  20,   5,  -5,
  -5,  10,  20,  30,  30,  20,  10,  -5,
  -5,  10,  20,  30,  30,  20,  10,  -5,
  -5,   5,  20,  10,  10,  20,   5,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[0x40] = {
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,  10,  10,   0,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,  10,   0,   0,   0,   0,  10,   0,
  0,  30,   0,   0,   0,   0,  30,   0,
  0,   0, -10,   0,   0, -10,   0,   0
};

// rook positional score
const int rook_score[0x40] = {
  50,  50,  50,  50,  50,  50,  50,  50,
  50,  50,  50,  50,  50,  50,  50,  50,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,  10,  20,  20,  10,   0,   0,
  0,   0,   0,  20,  20,   0,   0,   0
};

// king positional score
const int king_score[0x40] = {
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   5,   5,   5,   5,   0,   0,
  0,   5,   5,  10,  10,   5,   5,   0,
  0,   5,  10,  20,  20,  10,   5,   0,
  0,   5,  10,  20,  20,  10,   5,   0,
  0,   0,   5,  10,  10,   5,   0,   0,
  0,   5,   5,  -5,  -5,   0,   5,   0,
  0,   0,   5,   0, -15,   0,  10,   0
};

// mirror positional score tables for opposite side
const int mirror_score[0x80] = {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};

// most valuable victim & less valuable attacker

/*

    (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600

*/

// MVV LVA [attacker][victim]
const int mvv_lva[0xC][0xC] = {
  {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
  {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
  {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
  {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
  {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
  {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600},

  {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
  {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
  {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
  {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
  {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
  {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}
};
