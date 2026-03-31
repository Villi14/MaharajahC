#include <stdlib.h>

#include "maharajah/engine/Inits.h"
#include "maharajah/uci/UCI.h"
#include "maharajah/engine/Transposition.h"

int main() {
  init_all();

    // connect to GUI
    uci_loop();

    // free hash table memory on exit
    free(transposition_table.table);

  
  return 0;
}
