#include <stdio.h>
#include <stdlib.h>

#include "maharajah/util/Defines.h"
#include "maharajah/board/Fen.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Inits.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/engine/Transposition.h"

int main() {
  init_all();
  parse_fen(start_position);

  search_context.nodes = 0;
  perft_driver(2);

  if (search_context.nodes != 400) {
    fprintf(stderr, "perft_smoke failed: expected 400 nodes, got %llu\n",
            (u64)search_context.nodes);
    free(transposition_table.table);
    return 1;
  }

  free(transposition_table.table);
  return 0;
}
