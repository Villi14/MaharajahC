#ifndef ENGINE_H_
#define ENGINE_H_

#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/engine/Zobrist.h"

typedef struct {
  Board* board;
  SearchContext* search_context;
  TimeControls* time_controls;
  TranspositionTable* transposition_table;
  AttackTables* attack_tables;
  const EvalTables* eval_tables;
  ZobristKeys* zobrist_keys;
  unsigned int* random_state;
} Engine;

extern Engine engine;

#endif // ENGINE_H_
