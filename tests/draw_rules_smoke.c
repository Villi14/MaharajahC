#include <stdio.h>
#include <stdlib.h>

#include "../include/board/Fen.h"
#include "../include/engine/CustomSetup.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Moves.h"
#include "../include/engine/Search.h"
#include "../include/engine/Transposition.h"
#include "../include/parse/Parse.h"

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

static int apply_move_with_history(const char* move_string) {
  const int move = parse_move(move_string);
  if (move == 0)
    return 0;

  ++search_context.repetition_index;
  search_context.repetition_table[search_context.repetition_index] = board.hash_key;

  if (!make_move(move, all_moves)) {
    --search_context.repetition_index;
    return 0;
  }

  return 1;
}

int main(void) {
  init_all();

  parse_fen("7k/8/8/8/8/8/6N1/7K w - - 0 1 ");
  if (is_threefold_repetition())
    return fail("draw_rules_smoke failed: fresh position was treated as threefold repetition.");

  if (!apply_move_with_history("g2f4") ||
      !apply_move_with_history("h8g8") ||
      !apply_move_with_history("f4g2") ||
      !apply_move_with_history("g8h8") ||
      !apply_move_with_history("g2f4") ||
      !apply_move_with_history("h8g8")) {
    return fail("draw_rules_smoke failed: could not build repetition sequence.");
  }

  if (is_threefold_repetition())
    return fail("draw_rules_smoke failed: double occurrence was treated as threefold repetition.");

  const int repeating_root_move = parse_move("f4g2");
  const int fresh_root_move = parse_move("h1g1");
  if (repeating_root_move == 0 || fresh_root_move == 0)
    return fail("draw_rules_smoke failed: could not parse root repetition test moves.");
  if (!root_move_repeats_position(repeating_root_move))
    return fail("draw_rules_smoke failed: root move did not detect repeated position.");
  if (root_move_repeats_position(fresh_root_move))
    return fail("draw_rules_smoke failed: fresh root move was treated as repeated position.");

  if (!apply_move_with_history("f4g2") ||
      !apply_move_with_history("g8h8")) {
    return fail("draw_rules_smoke failed: could not finish threefold repetition sequence.");
  }

  if (!is_threefold_repetition())
    return fail("draw_rules_smoke failed: third occurrence was not detected as repetition.");
  if (repetition_count() != 3)
    return fail("draw_rules_smoke failed: repetition count did not reach three.");

  if (!apply_move_with_history("g2f4") ||
      !apply_move_with_history("h8g8") ||
      !apply_move_with_history("f4g2") ||
      !apply_move_with_history("g8h8") ||
      !apply_move_with_history("g2f4") ||
      !apply_move_with_history("h8g8") ||
      !apply_move_with_history("f4g2") ||
      !apply_move_with_history("g8h8")) {
    return fail("draw_rules_smoke failed: could not finish fivefold repetition sequence.");
  }

  if (!is_fivefold_repetition())
    return fail("draw_rules_smoke failed: fifth occurrence was not detected as repetition.");

  parse_fen("7k/8/8/8/8/8/6N1/7K w - - 99 1 ");
  if (is_fifty_move_rule())
    return fail("draw_rules_smoke failed: fifty-move rule triggered before the 100th halfmove.");
  if (!apply_move_with_history("g2f4"))
    return fail("draw_rules_smoke failed: non-pawn move for fifty-move rule was rejected.");
  if (!is_fifty_move_rule())
    return fail("draw_rules_smoke failed: fifty-move rule did not trigger at 100 halfmoves.");
  if (board.halfmove_clock != 100)
    return fail("draw_rules_smoke failed: halfmove clock did not increment to 100.");

  parse_fen("7k/8/8/8/8/8/6N1/7K w - - 149 1 ");
  if (is_seventy_five_move_rule())
    return fail("draw_rules_smoke failed: seventy-five-move rule triggered before the 150th halfmove.");
  if (!apply_move_with_history("g2f4"))
    return fail("draw_rules_smoke failed: non-pawn move for seventy-five-move rule was rejected.");
  if (!is_seventy_five_move_rule())
    return fail("draw_rules_smoke failed: seventy-five-move rule did not trigger at 150 halfmoves.");

  parse_fen("7k/8/8/8/8/8/4P3/7K w - - 99 1 ");
  if (!apply_move_with_history("e2e3"))
    return fail("draw_rules_smoke failed: pawn move for halfmove reset was rejected.");
  if (board.halfmove_clock != 0)
    return fail("draw_rules_smoke failed: pawn move did not reset halfmove clock.");
  if (is_fifty_move_rule())
    return fail("draw_rules_smoke failed: fifty-move rule stayed active after pawn reset.");

  parse_fen("7k/8/8/8/3M4/8/8/7K w - - 150 1 ");
  if (board.standard_rules)
    return fail("draw_rules_smoke failed: compound-piece position incorrectly kept standard rules enabled.");
  if (is_fifty_move_rule() || is_seventy_five_move_rule())
    return fail("draw_rules_smoke failed: standard halfmove draw rules stayed active for compound-piece position.");

  if (!generate_reasonable_custom_position(-1, 7U))
    return fail("draw_rules_smoke failed: custom position generation failed.");
  if (board.standard_rules)
    return fail("draw_rules_smoke failed: custom position incorrectly kept standard rules enabled.");
  board.halfmove_clock = 150;
  if (is_fifty_move_rule() || is_seventy_five_move_rule())
    return fail("draw_rules_smoke failed: standard halfmove draw rules stayed active for custom position.");

  parse_fen("7k/8/8/8/8/8/8/7K w - - 0 1 ");
  if (!is_insufficient_material())
    return fail("draw_rules_smoke failed: K vs K was not detected as insufficient material.");

  parse_fen("7k/8/8/8/8/8/8/6BK w - - 0 1 ");
  if (!is_insufficient_material())
    return fail("draw_rules_smoke failed: K+B vs K was not detected as insufficient material.");

  parse_fen("7k/8/8/8/8/8/8/6NK w - - 0 1 ");
  if (!is_insufficient_material())
    return fail("draw_rules_smoke failed: K+N vs K was not detected as insufficient material.");

  /* Not FIDE-dead positions: checkmates (helpmates) still exist. */
  parse_fen("7k/8/8/8/8/8/8/5NNK w - - 0 1 ");
  if (is_insufficient_material())
    return fail("draw_rules_smoke failed: K+N+N vs K was treated as insufficient material.");

  /* Bg1 is dark-squared, bg2 is light-squared: opposite colours, not dead. */
  parse_fen("7k/8/8/8/8/8/6b1/6BK w - - 0 1 ");
  if (is_insufficient_material())
    return fail("draw_rules_smoke failed: opposite-coloured K+B vs K+B was treated as insufficient material.");

  /* Bg1 and bf2 are both dark-squared: dead position. */
  parse_fen("7k/8/8/8/8/8/5b2/6BK w - - 0 1 ");
  if (!is_insufficient_material())
    return fail("draw_rules_smoke failed: same-coloured K+B vs K+B was not detected as insufficient material.");

  parse_fen("7k/8/8/8/8/8/6n1/6BK w - - 0 1 ");
  if (is_insufficient_material())
    return fail("draw_rules_smoke failed: K+B vs K+N was treated as insufficient material.");

  parse_fen("7k/8/8/8/8/8/6n1/6NK w - - 0 1 ");
  if (is_insufficient_material())
    return fail("draw_rules_smoke failed: K+N vs K+N was treated as insufficient material.");

  parse_fen("7k/8/8/8/8/8/6q1/6BK w - - 0 1 ");
  if (is_insufficient_material())
    return fail("draw_rules_smoke failed: position with a queen was treated as insufficient material.");

  free(transposition_table.table);
  return 0;
}
