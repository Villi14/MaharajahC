#include "../headers/Fen.h"
#include "../headers/Moves.h"
#include "../headers/Globals.h"
#include "../headers/UCI.h"
#include "../headers/Inits.h"
#include "../headers/Prints.h"

int main() {
  init_all();

  int debug = 1;

  if (debug) {
    // parse fen
    parse_fen(tricky_position);
    enpassant = c6;
    print_board();

    // create move list instance
    moves move_list[1];

    // generate moves
    generate_moves(move_list);

    // print move scores
    print_move_scores(move_list);
  } else
    uci_loop();

  return 0;
}
