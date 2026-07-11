#include <stdio.h>
#include <stdlib.h>

#include "../include/board/Fen.h"
#include "../include/engine/Evaluate.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Transposition.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

static int evaluate_fen(const char* fen) {
  parse_fen(fen);
  return evaluate();
}

static int expect_lower_eval(const char* worse_fen,
                             const char* better_fen,
                             const char* failure_message) {
  const int worse_eval = evaluate_fen(worse_fen);
  const int better_eval = evaluate_fen(better_fen);

  if (worse_eval >= better_eval)
    return fail(failure_message);

  return 0;
}

/* The classic eval has no colour-specific terms, so a position and its
   colour-flipped mirror must evaluate identically from the mover's side. */
static int expect_mirror_symmetry(const char* white_fen,
                                  const char* black_fen,
                                  const char* failure_message) {
  const int white_eval = evaluate_fen(white_fen);
  const int black_eval = evaluate_fen(black_fen);

  if (white_eval != black_eval)
    return fail(failure_message);

  return 0;
}

int main(void) {
  init_all();

  if (expect_lower_eval(
        "7k/8/8/4p3/3Q4/8/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/3Q4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: hanging queen evaluated no worse than a safe queen."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3R4/8/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/3R4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: hanging rook evaluated no worse than a safe rook."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3M4/8/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/3M4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: hanging amazon evaluated no worse than a safe amazon."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3C4/8/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/3C4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: hanging chancellor evaluated no worse than a safe chancellor."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3A4/8/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/3A4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: hanging archbishop evaluated no worse than a safe archbishop."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/8/8/8/1q6/K1B5 b - - 0 1 ",
        "7k/8/8/8/8/1q6/8/K1B5 b - - 0 1 ",
        "evaluate_safety_smoke failed: poisoned queen evaluated no worse than a safe queen."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3Q4/2B5/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/2BQ4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: attacked-but-defended queen evaluated no worse than a safer queen."
      ))
    return 1;

  if (expect_lower_eval(
        "7k/8/8/4p3/3C4/2B5/8/K7 w - - 0 1 ",
        "7k/8/8/4p3/8/2BC4/8/K7 w - - 0 1 ",
        "evaluate_safety_smoke failed: attacked-but-defended chancellor evaluated no worse than a safer chancellor."
      ))
    return 1;

  /* Passed-pawn regression: the bonus must grow as the pawn advances, for
     Black exactly as for White (the black table lookup goes through
     mirror_score). */
  if (expect_lower_eval(
        "4k3/4p3/8/8/8/8/8/4K3 b - - 0 1 ",
        "4k3/8/8/8/8/8/4p3/4K3 b - - 0 1 ",
        "evaluate_safety_smoke failed: black passed pawn near promotion evaluated no better than an unmoved one."
      ))
    return 1;

  if (expect_mirror_symmetry(
        "4k3/4P3/8/8/8/8/8/4K3 w - - 0 1 ",
        "4k3/8/8/8/8/8/4p3/4K3 b - - 0 1 ",
        "evaluate_safety_smoke failed: mirrored passed-pawn positions evaluated differently."
      ))
    return 1;

  if (expect_mirror_symmetry(
        "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1 ",
        "4k3/4p3/8/8/8/8/8/4K3 b - - 0 1 ",
        "evaluate_safety_smoke failed: mirrored home-rank pawn positions evaluated differently."
      ))
    return 1;

  free(transposition_table.table);
  return 0;
}
