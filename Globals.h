#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Constants.h"

// piece bitboards
extern U64 bitboards[12];

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
extern U64 pawn_attacks[2][64];

// knight attacks table [square]
extern U64 knight_attacks[64];

// king attacks table [square]
extern U64 king_attacks[64];

// bishop attack masks
extern U64 bishop_masks[64];

// rook attack masks
extern U64 rook_masks[64];

// bishop attacks table [square][occupancies]
extern U64 bishop_attacks[64][512];

// rook attacks rable [square][occupancies]
extern U64 rook_attacks[64][4096];

extern long nodes;

#endif // !GLOBALS_H_