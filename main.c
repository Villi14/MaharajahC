#include <stdio.h>

#include "Fen.h"
#include "Functions.h"
#include "Globals.h"
#include "Gui.h"
#include "Inits.h"
#include "Prints.h"

int main() {
  init_all();

  // parse "position" command
  parse_position("position startpos moves e2e4 e7e5 g1f3");
  print_board();

  return 0;
}