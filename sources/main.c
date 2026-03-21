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

  int debug = 1;

  if(debug) {
    // parse fen
    parse_fen("r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ");
    print_board();
    //search_position(6);
    printf("move score P x k: %d\n", mvv_lva[P][k]);
    printf("move score P x q: %d\n", mvv_lva[P][q]);
    printf("move score P x r: %d\n", mvv_lva[P][r]);
    printf("move score P x b: %d\n", mvv_lva[P][b]);
    printf("move score P x n: %d\n", mvv_lva[P][n]);
    printf("move score P x p: %d\n", mvv_lva[P][p]);

    printf("move score N x k: %d\n", mvv_lva[N][k]);
    printf("move score N x q: %d\n", mvv_lva[N][q]);
    printf("move score N x r: %d\n", mvv_lva[N][r]);
    printf("move score N x b: %d\n", mvv_lva[N][b]);
    printf("move score N x n: %d\n", mvv_lva[N][n]);
    printf("move score N x p: %d\n", mvv_lva[N][p]);
  } else
    uci_loop();

  return 0;
}
