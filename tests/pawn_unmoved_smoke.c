#include <stdio.h>
#include <stdlib.h>

#include "../include/board/Fen.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Moves.h"
#include "../include/engine/Transposition.h"
#include "../include/parse/Parse.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

int main(void) {
  init_all();

  // Real position from the LAN automation hang (match 8, ply 87): a black
  // pawn that started off-rank (custom hiddenVariant army) has already
  // walked from h7 to h6. Without field 8 the engine can't tell that apart
  // from a still-virgin custom pawn and offers h6-h4 as "legal" — which the
  // authoritative Dart client (real per-piece move history) correctly
  // rejects, causing the resubmit-storm hang this smoke test guards against.
  parse_fen("8/5p2/7p/1K6/4a3/5k2/8/8 b - - 25 44 Vv -");
  {
    if (!board.has_pawn_state)
      return fail("pawn_unmoved_smoke failed: field 8 '-' should set has_pawn_state.");
    const int illegal_double = parse_move("h6h4");
    if (illegal_double != 0 && get_move_double(illegal_double))
      return fail("pawn_unmoved_smoke failed: h6-h4 should be illegal once field 8 says no pawn is unmoved.");
  }

  // Same position, but field 8 truthfully says the h6 pawn is still virgin
  // (e.g. a custom army whose pawn legitimately starts on h6) - the
  // double-step must be offered.
  parse_fen("8/5p2/7p/1K6/4a3/5k2/8/8 b - - 25 44 Vv h6");
  {
    const int legal_double = parse_move("h6h4");
    if (legal_double == 0 || !get_move_double(legal_double))
      return fail("pawn_unmoved_smoke failed: h6-h4 should be legal when field 8 marks h6 unmoved.");
    if (!make_move(legal_double, all_moves))
      return fail("pawn_unmoved_smoke failed: legal double-step move was rejected by make_move.");
    if (!get_bit(board.bitboards[p], h4))
      return fail("pawn_unmoved_smoke failed: pawn did not land on h4 after the double-step.");

    // make_move maintains pawn_unmoved: the double-step spent the pawn's
    // lifetime right, so h6 must no longer be marked unmoved...
    if (get_bit(board.pawn_unmoved, h6))
      return fail("pawn_unmoved_smoke failed: h6 stayed marked unmoved after the pawn moved.");

    // ...and after any white reply the h4 pawn must not double-step again.
    const int white_reply = parse_move("b5b4");
    if (white_reply == 0 || !make_move(white_reply, all_moves))
      return fail("pawn_unmoved_smoke failed: white reply b5b4 was rejected.");
    const int second_double = parse_move("h4h2");
    if (second_double != 0 && get_move_double(second_double))
      return fail("pawn_unmoved_smoke failed: h4-h2 second double-step was offered after the pawn already moved.");
  }

  // Legacy 7-field FEN (no field 8 at all): falls back to the old rank/
  // side_variant heuristic untouched, so existing callers keep working.
  parse_fen("8/5p2/7p/1K6/4a3/5k2/8/8 b - - 25 44 Vv");
  {
    if (board.has_pawn_state)
      return fail("pawn_unmoved_smoke failed: omitted field 8 must not set has_pawn_state.");
    const int legacy_double = parse_move("h6h4");
    if (legacy_double == 0 || !get_move_double(legacy_double))
      return fail("pawn_unmoved_smoke failed: legacy heuristic should still allow h6-h4 for a variant side.");
  }

  free(transposition_table.table);
  printf("pawn_unmoved_smoke: OK\n");
  return 0;
}
