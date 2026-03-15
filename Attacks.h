#ifndef ATTACKS_H_
#define ATTACKS_H_

#include "Defines.h"

int is_square_attacked(int square, int side_);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 mask_pawn_attacks(int side_, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 bishop_attacks_on_the_fly(int square, U64 block);
U64 rook_attacks_on_the_fly(int square, U64 block);
U64 get_bishop_attacks(int square, U64 occupancy);
U64 get_rook_attacks(int square, U64 occupancy);
U64 get_queen_attacks(int square, U64 occupancy);


#endif // !ATTACKS_H_