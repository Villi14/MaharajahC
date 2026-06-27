#include <stdio.h>
#include <stdlib.h>

#include "../include/board/Fen.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Moves.h"
#include "../include/engine/Transposition.h"
#include "../include/engine/Zobrist.h"
#include "../include/parse/Parse.h"

static int count_moves_from(const MoveList* move_list, int source_square) {
  int count = 0;
  for (int index = 0; index < move_list->count; ++index) {
    if (get_move_source(move_list->moves[index]) == source_square) {
      ++count;
    }
  }
  return count;
}

static int fail(const char* message) {
  fprintf(stderr, "%s\n", message);
  free(transposition_table.table);
  return 1;
}

static int assert_hash_consistent(const char* message) {
  if (board.hash_key != generate_hash_key())
    return fail(message);

  return 0;
}

int main(void) {
  init_all();

  parse_fen("k7/8/8/8/3A4/8/8/7K w - - 0 1 ");
  if (!get_bit(board.bitboards[A], d4))
    return fail("compound_piece_smoke failed: FEN did not load white archbishop.");
  if (assert_hash_consistent("compound_piece_smoke failed: archbishop FEN hash mismatch."))
    return 1;

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);
  if (count_moves_from(&move_list, d4) != 21)
    return fail("compound_piece_smoke failed: archbishop move count mismatch.");

  parse_fen("k7/8/8/8/3C4/8/8/7K w - - 0 1 ");
  move_list.count = 0;
  generate_moves(&move_list);
  if (count_moves_from(&move_list, d4) != 22)
    return fail("compound_piece_smoke failed: chancellor move count mismatch.");
  if (assert_hash_consistent("compound_piece_smoke failed: chancellor FEN hash mismatch."))
    return 1;

  parse_fen("k7/8/8/8/3M4/8/8/7K w - - 0 1 ");
  move_list.count = 0;
  generate_moves(&move_list);
  if (count_moves_from(&move_list, d4) != 35)
    return fail("compound_piece_smoke failed: amazon move count mismatch.");
  if (assert_hash_consistent("compound_piece_smoke failed: amazon FEN hash mismatch."))
    return 1;

  parse_fen("k7/8/8/8/3A4/8/8/7K w - - 0 1 ");
  {
    const int archbishop_move = parse_move("d4f5");
    if (archbishop_move == 0)
      return fail("compound_piece_smoke failed: archbishop move was not parsed.");
    if (!make_move(archbishop_move, all_moves))
      return fail("compound_piece_smoke failed: archbishop move was not applied.");
    if (assert_hash_consistent("compound_piece_smoke failed: archbishop move hash mismatch."))
      return 1;

    const u64 move_hash = board.hash_key;
    parse_fen("k7/8/8/5A2/8/8/8/7K b - - 0 1 ");
    if (board.hash_key != move_hash)
      return fail("compound_piece_smoke failed: archbishop position hash differs across construction paths.");
  }

  parse_fen("7k/P7/8/8/8/8/8/K7 w - - 0 1 ");
  const int promotion_move = parse_move("a7a8m");
  if (promotion_move == 0 || get_move_promoted(promotion_move) != M)
    return fail("compound_piece_smoke failed: amazon promotion move was not parsed.");

  if (!make_move(promotion_move, all_moves) || !get_bit(board.bitboards[M], a8))
    return fail("compound_piece_smoke failed: amazon promotion move was not applied.");
  if (assert_hash_consistent("compound_piece_smoke failed: amazon promotion hash mismatch."))
    return 1;

  {
    const u64 promotion_hash = board.hash_key;
    parse_fen("M6k/8/8/8/8/8/8/K7 b - - 0 1 ");
    if (board.hash_key != promotion_hash)
      return fail("compound_piece_smoke failed: amazon promotion hash differs across construction paths.");
  }

  free(transposition_table.table);
  return 0;
}
