#include <stdio.h>
#include <stdlib.h>

#include "maharajah/util/Defines.h"
#include "maharajah/board/Fen.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Inits.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/engine/Transposition.h"

typedef struct {
  const char* name;
  const char* fen;
  int depth;
  u64 expected_nodes;
} PerftCase;

int main() {
  const PerftCase cases[] = {
    { "startpos_d2", start_position, 2, 400ULL },
    { "startpos_d3", start_position, 3, 8902ULL },
    { "tricky_d3", tricky_position, 3, 97862ULL },
  };

  init_all();

  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    parse_fen(cases[i].fen);
    search_context.nodes = 0;
    perft_driver(cases[i].depth);

    if (search_context.nodes != cases[i].expected_nodes) {
      fprintf(stderr, "perft_smoke failed (%s): expected %llu nodes, got %llu\n",
              cases[i].name,
              cases[i].expected_nodes,
              (u64)search_context.nodes);
      free(transposition_table.table);
      return 1;
    }
  }

  free(transposition_table.table);
  return 0;
}
