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

  parse_fen("4k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1 ");
  {
    const int king_side_castle = parse_move("e1g1");
    const int queen_side_castle = parse_move("e1c1");

    if (king_side_castle == 0 || !get_move_castling(king_side_castle))
      return fail("special_moves_smoke failed: king-side castle was not parsed.");
    if (queen_side_castle == 0 || !get_move_castling(queen_side_castle))
      return fail("special_moves_smoke failed: queen-side castle was not parsed.");
    if (!make_move(king_side_castle, all_moves))
      return fail("special_moves_smoke failed: king-side castle was rejected.");
    if (!get_bit(board.bitboards[K], g1) || !get_bit(board.bitboards[R], f1))
      return fail("special_moves_smoke failed: king-side castle board state is wrong.");
  }

  parse_fen("7k/8/8/3pP3/2A5/8/8/K7 w - d6 0 1 ");
  {
    const int en_passant = parse_move("e5d6");
    if (en_passant == 0 || !get_move_enpassant(en_passant))
      return fail("special_moves_smoke failed: en passant move was not parsed.");
    if (!make_move(en_passant, all_moves))
      return fail("special_moves_smoke failed: en passant move was rejected.");
    if (!get_bit(board.bitboards[P], d6))
      return fail("special_moves_smoke failed: en passant target square is missing the pawn.");
    if (get_bit(board.bitboards[p], d5))
      return fail("special_moves_smoke failed: captured pawn remained on the board after en passant.");
  }

  parse_fen("4k3/8/8/8/8/8/3M4/R3K2R w KQ - 0 1 ");
  {
    const int castle_with_amazon = parse_move("e1g1");
    if (castle_with_amazon != 0)
      return fail("special_moves_smoke failed: castling with amazon on board was allowed.");
  }

  free(transposition_table.table);
  return 0;
}
