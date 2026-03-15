#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Constants.h"

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

extern long nodes;

#endif // !GLOBALS_H_