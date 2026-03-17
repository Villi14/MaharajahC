#include <string.h>

#include "Fen.h"
#include "Globals.h"
#include "MagicNumbers.h"

// parse FEN string
void parse_fen(const char *fen) {
  memset(bitboards, 0ULL, sizeof(bitboards));
  memset(occupancies, 0ULL, sizeof(occupancies));
  side = 0;
  enpassant = no_sq;
  castle = 0;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        int piece = char_pieces[*fen];
        set_bit(bitboards[piece], square);
        ++fen;
      }

      if (*fen >= '0' && *fen <= '9') {
        int offset = *fen - '0';
        int piece = -1;

        for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
          if (get_bit(bitboards[bb_piece], square))
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

  (*fen == 'w') ? (side = white) : (side = black);

  fen += 2;

  while (*fen != ' ') {
    switch (*fen) {
    case 'K':
      castle |= wk;
      break;
    case 'Q':
      castle |= wq;
      break;
    case 'k':
      castle |= bk;
      break;
    case 'q':
      castle |= bq;
      break;
    case '-':
    default:
      break;
    }

    ++fen;
  }
  if (*fen != '-') {
    int file = fen[0] - 'a';
    int rank = 8 - (fen[1] - '0');
    enpassant = rank * 8 + file;
  } else
    enpassant = no_sq;

  for (int piece = P; piece <= K; ++piece)
    occupancies[white] |= bitboards[piece];

  for (int piece = p; piece <= k; ++piece)
    occupancies[black] |= bitboards[piece];

  occupancies[both] |= occupancies[white];
  occupancies[both] |= occupancies[black];
}
