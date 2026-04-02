#ifndef PRINTS_H_
#define PRINTS_H_

#include "maharajah/engine/Globals.h"

void print_board();
void print_bitboard(u64 bitboard);
void print_attacked_squares(int side);
void print_move(int move);
void print_move_scores(MoveList* move_list);

#endif // !PRINTS_H_
