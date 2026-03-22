#include "../headers/Fen.h"
#include "../headers/Globals.h"
#include "../headers/Inits.h"
#include "../headers/Moves.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"
#include "../headers/UCI.h"

int main() {
  init_all();

  int debug = 1;

  // if debugging
  if (debug) {
    // parse fen
    parse_fen(tricky_position);
    print_board();
    search_position(5);
  } else
    uci_loop();

  return 0;
}
