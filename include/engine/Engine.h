#ifndef ENGINE_H_
#define ENGINE_H_

#include "Globals.h"
#include "Transposition.h"
#include "Zobrist.h"

typedef struct {
  Board* board;
  SearchContext* search_context;
  TimeControls* time_controls;
  TranspositionTable* transposition_table;
  AttackTables* attack_tables;
  const EvalTables* eval_tables;
  ZobristKeys* zobrist_keys;
  uint* random_state;
} Engine;

 static Engine engine = {
  .board = &board,
  .search_context = &search_context,
  .time_controls = &time_controls,
  .transposition_table = &transposition_table,
  .attack_tables = &attack_tables,
  .eval_tables = &eval_tables,
  .zobrist_keys = &zobrist_keys,
  .random_state = &random_state
};

#endif // ENGINE_H_
