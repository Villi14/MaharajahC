#include <string.h>

#include "maharajah/board/Fen.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Zobrist.h"
#include "maharajah/util/Defines.h"

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

  for (int piece = P; piece <= K; ++piece)
    board.occupancies[white] |= board.bitboards[piece];

  for (int piece = p; piece <= k; ++piece)
    board.occupancies[black] |= board.bitboards[piece];

  board.occupancies[both] |= board.occupancies[white];
  board.occupancies[both] |= board.occupancies[black];

  board.hash_key = generate_hash_key();
}

void reset_board(void) {
  memset(board.bitboards, 0ULL, sizeof(board.bitboards));
  memset(board.occupancies, 0ULL, sizeof(board.occupancies));
  board.side = 0;
  board.enpassant = no_sq;
  board.castle = 0;
  search_context.repetition_index = 0;
  memset(search_context.repetition_table, 0ULL, sizeof(search_context.repetition_table));
}
