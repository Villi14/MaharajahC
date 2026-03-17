#include <stdio.h>

#include "Fen.h"
#include "Globals.h"
#include "Inits.h"
#include "Perft.h"
#include "Prints.h"

int main(void) {
  init_all();

  parse_fen(start_position);
  print_board();

  int start = get_time_ms();

  perft_driver(6);

  printf("time taken to execute: %d ms\n", get_time_ms() - start);
  printf("nodes: %ld\n", nodes);
}