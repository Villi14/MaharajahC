#include <stdio.h>
#include <stdlib.h>

#include "Defines.h"
#include "Fen.h"
#include "Globals.h"
#include "Inits.h"
#include "Perft.h"
#include "Transposition.h"

int main() {
  init_all();
  parse_fen(start_position);

  search_context.nodes = 0;
  perft_driver(2);

  if (search_context.nodes != 400) {
    fprintf(stderr, "perft_smoke failed: expected 400 nodes, got %llu\n",
            (unsigned long long)search_context.nodes);
    free(transposition_table.table);
    return 1;
  }

  free(transposition_table.table);
  return 0;
}
