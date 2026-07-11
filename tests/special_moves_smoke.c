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

  // Variant rules: a pawn may advance two squares on its first move from ANY
  // rank (here the 1st) when both squares ahead are free — non-orthodox setups
  // place pawns off the standard home rank. This is the exact position the
  // online server wrongly rejected ("the move is not legal"); compound pieces
  // (a/c/M/C) put parse_fen into variant rules.
  parse_fen("1aak3c/5n2/8/6n1/7M/6C1/8/2P1KP1P w - - 2 8 ");
  {
    const int variant_double = parse_move("f1f3");
    if (variant_double == 0 || !get_move_double(variant_double))
      return fail("special_moves_smoke failed: variant pawn double-step from the 1st rank was not generated.");
    if (!make_move(variant_double, all_moves))
      return fail("special_moves_smoke failed: variant pawn double-step was rejected.");
    if (!get_bit(board.bitboards[P], f3))
      return fail("special_moves_smoke failed: variant double-step did not land the pawn on f3.");
    if (board.enpassant != f2)
      return fail("special_moves_smoke failed: variant double-step set the wrong en-passant square.");
  }

  // Standard rules must stay orthodox: a pawn that is past its home rank (here
  // on the 3rd) may NOT double-step. Guards the variant relaxation from leaking
  // into standard play (and into the engine's standard-rules search).
  parse_fen("4k3/8/8/8/8/4P3/8/4K3 w - - 0 1 ");
  {
    const int illegal_double = parse_move("e3e5");
    if (illegal_double != 0)
      return fail("special_moves_smoke failed: standard pawn double-step from the 3rd rank was allowed.");
  }

  // Standard rules: pawns promote to Q/R/B/N only. The compound promotion
  // targets (archbishop/chancellor/amazon) belong to the variant and must not
  // be generated under standard play — no compound pieces on the board keeps
  // parse_fen in standard rules.
  parse_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1 ");
  {
    if (parse_move("a7a8q") == 0)
      return fail("special_moves_smoke failed: standard queen promotion was rejected.");
    if (parse_move("a7a8m") != 0)
      return fail("special_moves_smoke failed: standard compound (amazon) promotion was allowed.");
  }

  // Variant rules (a compound piece on the board switches parse_fen into
  // variant): promotion to a compound piece is legal.
  parse_fen("4k3/P7/8/8/7M/8/8/4K3 w - - 0 1 ");
  {
    if (parse_move("a7a8m") == 0)
      return fail("special_moves_smoke failed: variant compound (amazon) promotion was rejected.");
  }

  free(transposition_table.table);
  return 0;
}
