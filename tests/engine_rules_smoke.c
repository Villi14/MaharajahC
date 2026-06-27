#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/board/Fen.h"
#include "../include/engine/Attacks.h"
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

static int count_legal_moves(void) {
  MoveList move_list = { .count = 0 };
  int legal_moves = 0;

  generate_moves(&move_list);

  for (int index = 0; index < move_list.count; ++index) {
    const Board backup = board;
    if (make_move(move_list.moves[index], all_moves)) {
      ++legal_moves;
      board = backup;
    }
  }

  return legal_moves;
}

int main(void) {
  init_all();

  parse_fen("k7/8/1A6/8/8/8/8/7K b - - 0 1 ");
  if (!is_square_attacked(a8, white))
    return fail("engine_rules_smoke failed: archbishop knight attack was not detected.");

  parse_fen("2k5/8/8/8/2C5/8/8/K7 b - - 0 1 ");
  if (!is_square_attacked(c8, white))
    return fail("engine_rules_smoke failed: chancellor rook attack was not detected.");

  parse_fen("7k/8/8/8/3M4/8/8/K7 b - - 0 1 ");
  if (!is_square_attacked(h8, white))
    return fail("engine_rules_smoke failed: amazon attack was not detected.");

  parse_fen("4r2k/8/8/8/8/8/4A3/4K3 w - - 0 1 ");
  {
    const int illegal_move = parse_move("e2g3");
    if (illegal_move == 0)
      return fail("engine_rules_smoke failed: pinned archbishop move was not parsed.");
    if (make_move(illegal_move, all_moves))
      return fail("engine_rules_smoke failed: pinned archbishop move was treated as legal.");
  }

  parse_fen("7k/P7/8/8/8/8/8/K7 w - - 0 1 ");
  {
    const int archbishop_promotion = parse_move("a7a8a");
    if (archbishop_promotion == 0 || get_move_promoted(archbishop_promotion) != A)
      return fail("engine_rules_smoke failed: archbishop promotion move was not parsed.");
    if (!make_move(archbishop_promotion, all_moves) || !get_bit(board.bitboards[A], a8))
      return fail("engine_rules_smoke failed: archbishop promotion was not applied.");
  }

  parse_fen("7k/P7/8/8/8/8/8/K7 w - - 0 1 ");
  {
    const int chancellor_promotion = parse_move("a7a8c");
    if (chancellor_promotion == 0 || get_move_promoted(chancellor_promotion) != C)
      return fail("engine_rules_smoke failed: chancellor promotion move was not parsed.");
    if (!make_move(chancellor_promotion, all_moves) || !get_bit(board.bitboards[C], a8))
      return fail("engine_rules_smoke failed: chancellor promotion was not applied.");
  }

  parse_fen("k7/1M6/2K5/8/8/8/8/8 b - - 0 1 ");
  if (!is_square_attacked(a8, white))
    return fail("engine_rules_smoke failed: checkmate test position is not in check.");
  if (count_legal_moves() != 0)
    return fail("engine_rules_smoke failed: checkmate test position still has legal moves.");

  parse_fen("k7/2Q5/1K6/8/8/8/8/8 b - - 0 1 ");
  if (is_square_attacked(a8, white))
    return fail("engine_rules_smoke failed: stalemate test position is incorrectly in check.");
  if (count_legal_moves() != 0)
    return fail("engine_rules_smoke failed: stalemate test position still has legal moves.");

  free(transposition_table.table);
  return 0;
}
