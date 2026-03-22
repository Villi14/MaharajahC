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

    // create move list instance
    //moves move_list[1];

    // generate moves
    //generate_moves(move_list);

    // print move scores
    //print_move_scores(move_list);

    // sort move
    //sort_moves(move_list);

    // print move scores
    //print_move_scores(move_list);
  } else
    uci_loop();

  return 0;
}
