#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/util/Utils.h"
#include "../include/util/Defines.h"
#include "../include/engine/Globals.h"
#include "../include/engine/EngineConfig.h"
#include "../include/engine/Attacks.h"
#include "../include/engine/Moves.h"
#include "../include/engine/Search.h"
#include "../include/engine/CustomSetup.h"
#include "../include/board/Prints.h"
#include "../include/parse/Parse.h"
#include "../include/ffi/maharajah_ffi.h"

static void move_str(int move, char *out) {
  const int from = get_move_source(move);
  const int to = get_move_target(move);
  const int piece = get_move_piece(move);
  const char *names = "PNBRQACMKpnbrqacmk";
  sprintf(out, "%c%s%s", names[piece], square_to_coordinates[from],
          square_to_coordinates[to]);
}

static void dump_kings(const char *tag) {
  const int wk = board.bitboards[K] ? get_ls1b_index(board.bitboards[K]) : -1;
  const int bk = board.bitboards[k] ? get_ls1b_index(board.bitboards[k]) : -1;
  printf("[%s] side=%d  white K sq=%s  black k sq=%s\n", tag, board.side,
         wk >= 0 ? square_to_coordinates[wk] : "NONE",
         bk >= 0 ? square_to_coordinates[bk] : "NONE");
  if (bk >= 0)
    printf("        black king attacked by white? %d\n",
           is_square_attacked(bk, white));
  if (wk >= 0)
    printf("        white king attacked by black? %d\n",
           is_square_attacked(wk, black));
}

int main(int argc, char **argv) {
  const char *fen = argv[1];
  const char *movestr = (argc >= 3 && argv[2][0]) ? argv[2] : NULL;
  const int difficulty = argc >= 4 ? atoi(argv[3]) : 3;
  const int fixed_depth = argc >= 5 ? atoi(argv[4]) : max_ply;

  mah_init();
  if (!mah_set_position_fen(fen)) {
    fprintf(stderr, "failed to set fen\n");
    return 1;
  }
  mah_set_difficulty_level(difficulty);

  printf("=== initial position ===\n");
  print_board();
  dump_kings("initial");

  if (movestr) {
    const int m = parse_move(movestr);
    printf("\nparse_move(\"%s\") = %d\n", movestr, m);
    if (m == 0) { fprintf(stderr, "bad move\n"); return 1; }
    if (!make_move(m, all_moves)) { fprintf(stderr, "move illegal\n"); return 1; }
    printf("=== after %s ===\n", movestr);
    print_board();
    char f[160];
    custom_position_board_to_fen(f, (int)sizeof(f));
    printf("FEN after: %s\n", f);
    dump_kings("after-move");
  }

  reset_time_control();
  time_controls.stdin_polling_enabled = 0;
  search_root(fixed_depth, 0);
  printf("\nroot moves (side-to-move POV, best-first):\n");
  for (int i = 0; i < search_context.root_count && i < 12; ++i) {
    char s[16];
    move_str(search_context.root_moves[i], s);
    printf("  %2d. %-8s score=%d\n", i, s, search_context.root_scores[i]);
  }
  printf("\nPV (principal variation of the search):\n  ");
  for (int i = 0; i < search_context.pv_length[0]; ++i) {
    char s[16];
    move_str(search_context.pv_table[0][i], s);
    printf("%s ", s);
  }
  printf("\n");
  return 0;
}
