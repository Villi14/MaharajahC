#include <stdlib.h>

#include "../headers/Inits.h"
#include "../headers/UCI.h"
#include "../headers/Transposition.h"

int main() {
  init_all();

    // connect to GUI
    uci_loop();

    // free hash table memory on exit
    free(hash_table);

  
  return 0;
}
