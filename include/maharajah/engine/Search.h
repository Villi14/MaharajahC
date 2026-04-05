#ifndef SEARCH_H_
#define SEARCH_H_

#include "maharajah/engine/Globals.h"

int negamax(int alpha, int beta, int depth);
int quiescence(int alpha, int beta);
void search_position(int depth);
void sort_moves(MoveList* move_list);
int score_move(int move);
void enable_pv_scoring(MoveList* move_list);
int is_repetition();

#endif // !SEARCH_H_
