#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/board/Fen.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/See.h"
#include "../include/engine/Transposition.h"
#include "../include/parse/Parse.h"
#include "../include/util/Defines.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

int main(void) {
  init_all();

  /* Nxd4xb6: undeveloped B on b6, no favourable recapture (black king too far for cheap recapture on b6 in SEE sequence). */
  parse_fen("8/1k6/1b6/3N3/8/8/8/4K3 w - - 0 1 ");
  {
    const int m = parse_move("d5b6");
    if (m == 0)
      return fail("see_smoke failed: could not parse d5b6.");
    if (!get_move_capture(m))
      return fail("see_smoke failed: d5b6 is not a capture.");
    if (see_evaluate(m) <= 0)
      return fail("see_smoke failed: NxB on b6 should have positive SEE.");
  }

  /* Qxd7: Rxd7 recapture gains material advantage for Black — SEE for the queen capture is negative. */
  parse_fen("3r1k2/3p4/3Q4/8/8/8/8/4K3 w - - 0 1 ");
  {
    const int m = parse_move("d6d7");
    if (m == 0)
      return fail("see_smoke failed: could not parse d6d7.");
    if (see_evaluate(m) >= 0)
      return fail("see_smoke failed: Qxp with R on d8 should have negative SEE.");
  }

  /* Stand-pat regression: Rxa5 wins the queen. After ...bxa5 White is NOT
     forced into the losing Qxa5 Rxa5 continuation — SEE must let White stand
     pat, so the capture stays clearly positive (+Q -R). */
  parse_fen("r6k/8/1p6/q7/R7/Q7/8/7K w - - 0 1 ");
  {
    const int m = parse_move("a4a5");
    if (m == 0)
      return fail("see_smoke failed: could not parse a4a5.");
    if (see_evaluate(m) <= 0)
      return fail("see_smoke failed: RxQ with stand pat should have positive SEE.");
  }

  free(transposition_table.table);
  return 0;
}
