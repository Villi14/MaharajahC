#include "../../include/engine/Masks.h"
#include "../../include/util/Defines.h"
#include "../../include/util/Utils.h"

u64 file_masks[0x40];
u64 rank_masks[0x40];
u64 isolated_masks[0x40];
u64 white_passed_masks[0x40];
u64 black_passed_masks[0x40];

// clang-format off
const int get_rank[0x40] = {
  7, 7, 7, 7, 7, 7, 7, 7,
  6, 6, 6, 6, 6, 6, 6, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0
};
// clang-format on

int get_game_phase_score(void) {
  int white_piece_scores = 0, black_piece_scores = 0;

  const int white_phase_pieces[] = { N, B, R, Q, A, C, M };
  const int black_phase_pieces[] = { n, b, r, q, a, c, m };

  for (int index = 0; index < 7; ++index)
    white_piece_scores += count_bits(board.bitboards[white_phase_pieces[index]]) *
                          eval_tables.material_score[opening][white_phase_pieces[index]];

  for (int index = 0; index < 7; ++index)
    black_piece_scores += count_bits(board.bitboards[black_phase_pieces[index]]) *
                          -eval_tables.material_score[opening][black_phase_pieces[index]];

  return white_piece_scores + black_piece_scores;
}

u64 set_file_rank_mask(const int file_number, const int rank_number) {
  u64 mask = 0ULL;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if (file_number != -1) {
        if (file == file_number)
          set_bit(mask, square);
      }

      else if (rank_number != -1) {
        if (rank == rank_number)
          set_bit(mask, square);
      }
    }
  }

  return mask;
}

void init_evaluation_masks(void) {
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      file_masks[square] |= set_file_rank_mask(file, -1);
    }
  }

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      rank_masks[square] |= set_file_rank_mask(-1, rank);
    }
  }

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;
      isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
      isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
    }
  }

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
      white_passed_masks[square] |= set_file_rank_mask(file, -1);
      white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

      for (int i = 0; i < (8 - rank); ++i)
        white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
    }
  }

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
      black_passed_masks[square] |= set_file_rank_mask(file, -1);
      black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

      for (int i = 0; i < rank + 1; ++i)
        black_passed_masks[square] &= ~rank_masks[i * 8 + file];
    }
  }
}
