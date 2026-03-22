#include "../headers/Fen.h"
#include "../headers/Globals.h"
#include "../headers/Inits.h"
#include "../headers/Moves.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"
#include "../headers/UCI.h"
#include "../headers/Perft.h"

int main() {
  init_all();

  int debug = 1;

  // if debugging
  if (debug) {
    // parse fen
    parse_fen(tricky_position);
    print_board();
    search_position(6);
  } else
    uci_loop();

  //perft_test(5);
  
  return 0;
}
