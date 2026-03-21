#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "Defines.h"

void generate_moves(moves* move_list);
int make_move(int move, int move_flag);
int count_bits(U64 bitboard);
int get_ls1b_index(U64 bitboard);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
static inline void add_move(moves* move_list, int move);

#endif // !FUNCTIONS_H_
