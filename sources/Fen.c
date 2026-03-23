#include <string.h>

#include "../headers/Fen.h"
#include "../headers/Defines.h"
#include "../headers/Globals.h"
#include "../headers/Zobrist.h"

// parse FEN string
void parse_fen(const char* fen) {
  // prepare for new game
  reset_board();

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        const int piece = char_pieces[(int)*fen];
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

  ++fen;

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

  // init hash key
  hash_key = generate_hash_key();
}

// reset board variables
void reset_board() {
  // reset board position (bitboards)
  memset(bitboards, 0ULL, sizeof(bitboards));

  // reset occupancies (bitboards)
  memset(occupancies, 0ULL, sizeof(occupancies));

  // reset game state variables
  side = 0;
  enpassant = no_sq;
  castle = 0;

  // reset repetition index
  repetition_index = 0;

  // reset repetition table
  memset(repetition_table, 0ULL, sizeof(repetition_table));
}
