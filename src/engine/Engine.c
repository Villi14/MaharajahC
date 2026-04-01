#include "maharajah/engine/Engine.h"

Engine engine = { .board = &board,
                  .search_context = &search_context,
                  .time_controls = &time_controls,
                  .transposition_table = &transposition_table,
                  .attack_tables = &attack_tables,
                  .eval_tables = &eval_tables,
                  .zobrist_keys = &zobrist_keys,
                  .random_state = &random_state };
