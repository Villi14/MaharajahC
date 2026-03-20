#include <stdio.h>

#include "Fen.h"
#include "Functions.h"
#include "Globals.h"
#include "Gui.h"
#include "Inits.h"
#include "Prints.h"

int main() {
  init_all();

  parse_fen("r3k2r/p11pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 1 ");
  print_board();

  int move = parse_move("d5c6");
  
  if(move) {
    make_move(move, all_moves);
    print_board();
  }
  else
    printf("illegal move!\n");

  return 0;
}