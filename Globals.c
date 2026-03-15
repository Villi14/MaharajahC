#include "Globals.h"
#include "Defines.h"

int side;
int enpassant = no_sq;
int castle;
unsigned int random_state = 0x6B8B4567;
long nodes;

U64 bitboards[0xC];
U64 occupancies[3];
U64 pawn_attacks[2][0x40];
U64 knight_attacks[0x40];
U64 king_attacks[0x40];
U64 bishop_masks[0x40];
U64 rook_masks[0x40];
U64 bishop_attacks[0x40][0x200];
U64 rook_attacks[0x40][0x1000];

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

// ASCII pieces
const char *ascii_pieces[0xC] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};

// Unicode pieces
const char *unicode_pieces[0xC] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};


// convert ASCII character pieces to encoded constants
const char char_pieces[0x80] = {
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