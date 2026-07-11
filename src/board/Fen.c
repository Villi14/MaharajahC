#include <stdlib.h>
#include <string.h>

#include "../../include/board/Fen.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Zobrist.h"
#include "../../include/util/Defines.h"

void parse_fen(const char* fen) {
  reset_board();
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;

      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        const int piece = char_pieces[(int)*fen];
        set_bit(board.bitboards[piece], square);
        ++fen;
      }

      if (*fen >= '0' && *fen <= '9') {
        const int offset = *fen - '0';
        int piece = -1;

        for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
          if (get_bit(board.bitboards[bb_piece], square))
            piece = bb_piece;
        }

        if (piece == -1)
          --file;

        file += offset;
        ++fen;
      }

      if (*fen == '/')
        ++fen;
    }
  }

  ++fen;

  (*fen == 'w') ? (board.side = white) : (board.side = black);

  fen += 2;

  while (*fen != ' ') {
    switch (*fen) {
    case 'K':
      board.castle |= wk;
      break;
    case 'Q':
      board.castle |= wq;
      break;
    case 'k':
      board.castle |= bk;
      break;
    case 'q':
      board.castle |= bq;
      break;
    case '-':
    default:
      break;
    }

    ++fen;
  }

  ++fen;

  if (*fen != '-') {
    const int file = fen[0] - 'a';
    const int rank = 8 - (fen[1] - '0');
    board.enpassant = rank * 8 + file;
  } else
    board.enpassant = no_sq;

  while (*fen && *fen != ' ')
    ++fen;

  // Field 5: halfmove clock. While here, walk on to the optional field 7
  // (per-side variant rights) which trails the fullmove number. -1 means the
  // field is absent (legacy 6-field FEN) and we fall back to a board-wide
  // derivation below.
  board.halfmove_clock = 0;
  int parsed_white_variant = -1;
  int parsed_black_variant = -1;
  if (*fen == ' ') {
    char* cursor = NULL;
    board.halfmove_clock = (int)strtol(fen + 1, &cursor, 10);
    if (cursor == fen + 1)
      board.halfmove_clock = 0;

    while (*cursor == ' ')
      ++cursor;
    while (*cursor && *cursor != ' ') // skip field 6 (fullmove number)
      ++cursor;
    while (*cursor == ' ')
      ++cursor;

    // Field 7: variant rights. 'V' = White plays variant rules, 'v' = Black,
    // '-' (or empty token) = both classic.
    if (*cursor && *cursor != ' ') {
      parsed_white_variant = 0;
      parsed_black_variant = 0;
      for (; *cursor && *cursor != ' '; ++cursor) {
        if (*cursor == 'V')
          parsed_white_variant = 1;
        else if (*cursor == 'v')
          parsed_black_variant = 1;
      }
    }

    while (*cursor == ' ')
      ++cursor;

    // Field 8 (optional): unmoved-pawn squares, e.g. "h7f8" or "-" for none.
    // Caller-supplied per-piece "has moved" truth — see pawn_unmoved's
    // declaration. Absent entirely (end of string right here) means the
    // caller isn't sending it, so pawn double-step generation falls back to
    // the old rank/side_variant heuristic.
    board.pawn_unmoved = 0ULL;
    board.has_pawn_state = 0;
    if (*cursor && *cursor != ' ') {
      board.has_pawn_state = 1;
      if (*cursor != '-') {
        while (cursor[0] && cursor[1] && cursor[0] != ' ') {
          const int file = cursor[0] - 'a';
          const int rank = 8 - (cursor[1] - '0');
          if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
            set_bit(board.pawn_unmoved, rank * 8 + file);
          cursor += 2;
        }
      }
    }
  }

  for (int piece = P; piece <= K; ++piece)
    board.occupancies[white] |= board.bitboards[piece];

  for (int piece = p; piece <= k; ++piece)
    board.occupancies[black] |= board.bitboards[piece];

  board.occupancies[both] |= board.occupancies[white];
  board.occupancies[both] |= board.occupancies[black];

  board.standard_rules = 1;
  if (board_has_compound_pieces())
    board.standard_rules = 0;

  if (parsed_white_variant >= 0) {
    board.side_variant[white] = parsed_white_variant;
    board.side_variant[black] = parsed_black_variant;
  } else {
    // Legacy FEN: no per-side info. Mirror the board-wide flag so a pure
    // standard board is classic for both sides and a board carrying compound
    // material is variant for both (matches pre-field behavior).
    board.side_variant[white] = board.standard_rules ? 0 : 1;
    board.side_variant[black] = board.standard_rules ? 0 : 1;
  }

  // A variant side never castles, so strip its castling rights regardless of
  // what the FEN's castling field claims. In a mixed game this keeps the
  // classic side's rights intact — the old board-wide "any compound piece on
  // the board kills all castling" rule wrongly disabled the classic side too.
  if (board.side_variant[white])
    board.castle &= ~(wk | wq);
  if (board.side_variant[black])
    board.castle &= ~(bk | bq);

  board.hash_key = generate_hash_key();
}

void reset_board(void) {
  memset(board.bitboards, 0ULL, sizeof(board.bitboards));
  memset(board.occupancies, 0ULL, sizeof(board.occupancies));
  board.side = 0;
  board.enpassant = no_sq;
  board.castle = 0;
  board.halfmove_clock = 0;
  board.standard_rules = 1;
  board.side_variant[white] = 0;
  board.side_variant[black] = 0;
  board.pawn_unmoved = 0ULL;
  board.has_pawn_state = 0;
  search_context.repetition_index = 0;
  memset(search_context.repetition_table, 0ULL, sizeof(search_context.repetition_table));
}
