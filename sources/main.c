#include <stdio.h>

#include "../headers/Fen.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"
#include "../headers/Gui.h"
#include "../headers/Inits.h"
#include "../headers/Prints.h"

int main() {
  init_all();

  // parse "position" command
  parse_position("position startpos moves e2e4 e7e5 g1f3");
  print_board();

  return 0;
}