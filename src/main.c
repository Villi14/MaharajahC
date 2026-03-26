#include <stdlib.h>

#include "Inits.h"
#include "UCI.h"
#include "Transposition.h"

int main() {
  init_all();

    // connect to GUI
    uci_loop();

    // free hash table memory on exit
    free(transposition_table.table);

  
  return 0;
}
