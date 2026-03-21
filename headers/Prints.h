#ifndef PRINTS_H_
#define PRINTS_H_

#include "Defines.h"

void print_board();
void print_bitboard(U64 bitboard);
void print_attacked_squares(int side_);
void print_move(int move);
void print_move_scores(moves* move_list);

#endif // !PRINTS_H_
