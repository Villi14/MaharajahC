#include <stdio.h>

#include "../headers/Evaluate.h"
#include "../headers/Fen.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"
#include "../headers/Gui.h"
#include "../headers/Inits.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"

int main() {
  init_all();

  int debug = 0;

  if(debug) {
    // parse fen
    parse_fen(start_position);
    print_board();
    search_position(2);
  } else
    uci_loop();

  return 0;
}