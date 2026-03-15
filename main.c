#include <stdio.h>

#include "Functions.h"
#include "Globals.h"

int main(void) {
  // init all
  init_all();

  // parse fen
  parse_fen(start_position);
  print_board();

  // start tracking time
  int start = get_time_ms();

  // perft
  perft_driver(6);

  // time taken to execute program
  printf("time taken to execute: %d ms\n", get_time_ms() - start);
  printf("nodes: %ld\n", nodes);

  return 0;
}