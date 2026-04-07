#ifndef MOVES_H_
#define MOVES_H_

#include <assert.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "maharajah/engine/Attacks.h"
#include "maharajah/util/Defines.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Zobrist.h"
#include "maharajah/util/Utils.h"

static inline void add_move(MoveList* move_list, int move) {
  move_list->moves[move_list->count] = move;
  ++move_list->count;
}

int make_move(int move, const int move_flag);

void generate_moves(MoveList* move_list);

#endif // MOVES_H_
