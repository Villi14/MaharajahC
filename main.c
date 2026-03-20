#include <stdio.h>

#include "Fen.h"
#include "Globals.h"
#include "Inits.h"
#include "Perft.h"
#include "Prints.h"

int main(void) {
  init_all();

  parse_fen(tricky_position);
  print_board();

  int start = get_time_ms();

  perft_test(5);
  getchar();

  return 0;
}