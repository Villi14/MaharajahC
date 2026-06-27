#ifndef SEARCH_H_
#define SEARCH_H_

#include "../../include/engine/Globals.h"

int negamax(int alpha, int beta, int depth);
int quiescence(int alpha, int beta);
int search_root(int depth, int emit_info);
void search_position(int depth);
void sort_moves(MoveList* move_list);
int score_move(int move);
void enable_pv_scoring(MoveList* move_list);
int root_move_repeats_position(int move);
int is_repetition(void);
int repetition_count(void);
int is_threefold_repetition(void);
int is_fifty_move_rule(void);
int is_fivefold_repetition(void);
int is_seventy_five_move_rule(void);
int is_insufficient_material(void);

#endif // SEARCH_H_
