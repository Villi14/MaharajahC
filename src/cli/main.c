#include <stdlib.h>
#include <stdio.h>

#include "maharajah/engine/Inits.h"
#include "maharajah/uci/UCI.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/board/Prints.h"
#include "maharajah/board/Fen.h"


int main(void) {
  init_all();
    // connect to GUI
    uci_loop();

    // free hash table memory on exit
    free(transposition_table.table);

  return 0;
}
#if 0
int main(void)
{
    // init all
    init_all();

    // parse fen
    parse_fen(tricky_position);
    print_board();

    // perft
    perft_test(5);
    getchar();

    return 0;
}
#endif
