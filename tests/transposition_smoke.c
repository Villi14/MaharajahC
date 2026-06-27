#include <stdio.h>
#include <stdlib.h>

#include "../include/board/Fen.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Transposition.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

static int expect_exact_roundtrip(
  const char* fen,
  int score,
  int depth,
  const char* read_failure,
  const char* restore_failure
) {
  parse_fen(fen);
  search_context.ply = 0;
  write_hash_entry(score, depth, hash_flag_exact);

  if (read_hash_entry(-infinity, infinity, depth) != score)
    return fail(read_failure);

  parse_fen(fen);
  search_context.ply = 0;
  if (read_hash_entry(-infinity, infinity, depth) != score)
    return fail(restore_failure);

  return 0;
}

int main(void) {
  init_all();

  if (expect_exact_roundtrip(
        "k7/8/8/8/3A4/8/8/7K w - - 0 1 ",
        123,
        4,
        "transposition_smoke failed: could not read archbishop position from transposition table.",
        "transposition_smoke failed: archbishop position hash did not restore transposition entry."
      )) {
    return 1;
  }

  if (expect_exact_roundtrip(
        "k7/8/8/8/3C4/8/8/7K w - - 0 1 ",
        -77,
        5,
        "transposition_smoke failed: could not read chancellor position from transposition table.",
        "transposition_smoke failed: chancellor position hash did not restore transposition entry."
      )) {
    return 1;
  }

  if (expect_exact_roundtrip(
        "k7/8/8/8/3M4/8/8/7K w - - 0 1 ",
        314,
        6,
        "transposition_smoke failed: could not read amazon position from transposition table.",
        "transposition_smoke failed: amazon position hash did not restore transposition entry."
      )) {
    return 1;
  }

  clear_hash_table();
  parse_fen("k7/8/8/8/3M4/8/8/7K w - - 0 1 ");
  search_context.ply = 0;
  if (read_hash_entry(-infinity, infinity, 6) != no_hash_entry)
    return fail("transposition_smoke failed: clear_hash_table did not remove amazon entry.");

  free(transposition_table.table);
  return 0;
}
