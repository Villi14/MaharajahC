#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/engine/Globals.h"
#include "../include/engine/Transposition.h"
#include "../include/ffi/maharajah_ffi.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  mah_shutdown();
  return 1;
}

static int expect_best_move(
  const char* fen,
  int depth,
  const char* expected_move,
  const char* failure_message
) {
  char out_move[16] = {0};

  if (!mah_set_position_fen(fen))
    return fail("search_sanity_smoke failed: could not set FEN.");

  if (!mah_best_move_depth(depth, out_move, (int)sizeof(out_move)))
    return fail("search_sanity_smoke failed: engine did not return a best move.");

  if (strcmp(out_move, expected_move) != 0) {
    return fail(failure_message);
  }

  return 0;
}

static int expect_no_best_move(
  const char* fen,
  int depth,
  const char* failure_message
) {
  char out_move[16] = {0};

  if (!mah_set_position_fen(fen))
    return fail("search_sanity_smoke failed: could not set FEN.");

  if (mah_best_move_depth(depth, out_move, (int)sizeof(out_move)))
    return fail(failure_message);

  return 0;
}

static int expect_best_move_not(
  const char* fen,
  int depth,
  const char* forbidden_move,
  const char* failure_message
) {
  char out_move[16] = {0};

  if (!mah_set_position_fen(fen))
    return fail("search_sanity_smoke failed: could not set FEN.");

  if (!mah_best_move_depth(depth, out_move, (int)sizeof(out_move)))
    return fail("search_sanity_smoke failed: engine did not return a best move.");

  if (strcmp(out_move, forbidden_move) == 0)
    return fail(failure_message);

  return 0;
}

static int expect_timed_best_move_not(
  const char* fen,
  int difficulty,
  unsigned int seed,
  int movetime_ms,
  const char* forbidden_move,
  const char* failure_message
) {
  char out_move[16] = {0};

  if (!mah_set_position_fen(fen))
    return fail("search_sanity_smoke failed: could not set FEN.");

  if (!mah_set_difficulty_level(difficulty))
    return fail("search_sanity_smoke failed: could not set difficulty.");

  random_state = seed;

  if (!mah_best_move_time(movetime_ms, out_move, (int)sizeof(out_move)))
    return fail("search_sanity_smoke failed: engine did not return a timed best move.");

  if (strcmp(out_move, forbidden_move) == 0)
    return fail(failure_message);

  if (!mah_set_difficulty_level(5))
    return fail("search_sanity_smoke failed: could not restore difficulty.");

  return 0;
}

int main(void) {
  if (!mah_init()) {
    fprintf(stderr, "search_sanity_smoke failed: engine initialization failed.\n");
    return 1;
  }

  if (expect_no_best_move(
        "k7/1M6/2K5/8/8/8/8/8 b - - 0 1 ",
        1,
        "search_sanity_smoke failed: engine returned a best move in checkmate."
      )) {
    return 1;
  }

  if (expect_best_move(
        "7k/P7/8/8/8/8/8/K7 w - - 0 1 ",
        1,
        "a7a8m",
        "search_sanity_smoke failed: engine did not prefer amazon promotion at depth 1."
      )) {
    return 1;
  }

  if (expect_best_move(
        "k7/5q2/8/4A3/8/8/8/K7 w - - 0 1 ",
        1,
        "e5f7",
        "search_sanity_smoke failed: engine did not find the archbishop queen capture."
      )) {
    return 1;
  }

  if (expect_best_move(
        "7k/8/8/3C4/8/8/3q4/K7 w - - 0 1 ",
        1,
        "d5d2",
        "search_sanity_smoke failed: engine did not find the chancellor queen capture."
      )) {
    return 1;
  }

  if (expect_best_move(
        "k7/8/8/3M4/8/8/8/K6q w - - 0 1 ",
        2,
        "d5h1",
        "search_sanity_smoke failed: engine did not find the amazon queen capture."
      )) {
    return 1;
  }

  if (expect_best_move(
        "6k1/8/8/8/8/8/6q1/6RK w - - 0 1 ",
        1,
        "h1g2",
        "search_sanity_smoke failed: engine did not choose the safe king capture to escape check."
      )) {
    return 1;
  }

  if (expect_best_move(
        "6k1/8/8/8/3M4/8/6q1/6RK w - - 0 1 ",
        2,
        "h1g2",
        "search_sanity_smoke failed: engine did not handle the amazon-check defensive escape."
      )) {
    return 1;
  }

  if (expect_best_move_not(
        "rnb1kbnr/ppp1pppp/8/1q6/8/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 4 ",
        1,
        "b5b2",
        "search_sanity_smoke failed: engine grabbed the poisoned b2 pawn and hung the queen."
      )) {
    return 1;
  }

  if (expect_best_move(
        "rnb1kbnr/ppp1pppp/8/8/8/5N2/PqPP1PPP/RNBQK2R w KQkq - 0 5 ",
        1,
        "c1b2",
        "search_sanity_smoke failed: engine did not punish the trapped queen on b2."
      )) {
    return 1;
  }

  if (expect_best_move(
        "7k/8/8/8/8/8/1m6/K1B5 w - - 0 1 ",
        1,
        "c1b2",
        "search_sanity_smoke failed: engine did not punish the hanging amazon on b2."
      )) {
    return 1;
  }

  if (expect_best_move(
        "7k/8/7c/8/8/8/8/K1B5 w - - 0 1 ",
        1,
        "c1h6",
        "search_sanity_smoke failed: engine did not punish the hanging chancellor on h6."
      )) {
    return 1;
  }

  if (expect_best_move(
        "7k/8/8/8/8/8/1a6/K1B5 w - - 0 1 ",
        1,
        "c1b2",
        "search_sanity_smoke failed: engine did not punish the hanging archbishop on b2."
      )) {
    return 1;
  }

  if (expect_timed_best_move_not(
        "rnbqkb1r/pp3ppp/4N3/2pn4/8/2N5/PPPP1PPP/R1BQKB1R b KQkq - 0 6 ",
        3,
        3U,
        600,
        "b8c6",
        "search_sanity_smoke failed: timed weak-mode move selection hung the queen on d8."
      )) {
    return 1;
  }

  if (expect_no_best_move(
        "k7/2Q5/1K6/8/8/8/8/8 b - - 0 1 ",
        1,
        "search_sanity_smoke failed: engine returned a best move in stalemate."
      )) {
    return 1;
  }

  mah_shutdown();
  free(transposition_table.table);
  return 0;
}
