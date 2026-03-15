#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "Defines.h"

void init_all();
void init_magic_numbers();
void print_board();
void print_bitboard(U64 bitboard);
void print_attacked_squares(int side);
void parse_fen(char *fen);
void perft_driver(int depth);
void add_move(moves *move_list, int move);
int make_move(int move, int move_flag);
int is_square_attacked(int square, int side);
int get_time_ms();
int count_bits(U64 bitboard);
int get_ls1b_index(U64 bitboard);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
U64 bishop_attacks_on_the_fly(int square, U64 block);
U64 rook_attacks_on_the_fly(int square, U64 block);
U64 get_bishop_attacks(int square, U64 occupancy);
U64 get_rook_attacks(int square, U64 occupancy);
U64 get_queen_attacks(int square, U64 occupancy);

#endif // !FUNCTIONS_H_
