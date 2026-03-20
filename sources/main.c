#include <stdio.h>

#include "../headers/Fen.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"
#include "../headers/Gui.h"
#include "../headers/Inits.h"
#include "../headers/Prints.h"
#include "../headers/Search.h"
#include "../headers/Gui.h"
#include "../headers/Evaluate.h"

int main() {
  init_all();

  int debug = 1;

  if(debug) {
    // parse fen
    parse_fen("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1 ");
    print_board();
    printf("score: %d\n", evaluate());
  }
  else
    uci_loop();

  return 0;
}