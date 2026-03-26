#include "Defines.h"

// file masks [square]
u64 file_masks[0x40];

// rank masks [square]
u64 rank_masks[0x40];

// isolated pawn masks [square]
u64 isolated_masks[0x40];

// white passed pawn masks [square]
u64 white_passed_masks[0x40];

// black passed pawn masks [square]
u64 black_passed_masks[0x40];

// extract rank from a square [square]
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

// set file or rank mask
u64 set_file_rank_mask(int file_number, int rank_number) {
  // file or rank mask
  u64 mask = 0ULL;

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      if (file_number != -1) {
        // on file match
        if (file == file_number)
          // set bit on mask
          set_bit(mask, square);
      }

      else if (rank_number != -1) {
        // on rank match
        if (rank == rank_number)
          // set bit on mask
          set_bit(mask, square);
      }
    }
  }

  // return mask
  return mask;
}

// init evaluation masks
void init_evaluation_masks() {
  /******** Init file masks ********/

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // init file mask for a current square
      file_masks[square] |= set_file_rank_mask(file, -1);
    }
  }

  /******** Init rank masks ********/

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // init rank mask for a current square
      rank_masks[square] |= set_file_rank_mask(-1, rank);
    }
  }

  /******** Init isolated masks ********/

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // init isolated pawns masks for a current square
      isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
      isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
    }
  }

  /******** White passed masks ********/

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // init white passed pawns mask for a current square
      white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
      white_passed_masks[square] |= set_file_rank_mask(file, -1);
      white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

      // loop over redudant ranks
      for (int i = 0; i < (8 - rank); i++)
        // reset redudant bits
        white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
    }
  }

  /******** Black passed masks ********/

  // loop over ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // init black passed pawns mask for a current square
      black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
      black_passed_masks[square] |= set_file_rank_mask(file, -1);
      black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

      // loop over redudant ranks
      for (int i = 0; i < rank + 1; i++)
        // reset redudant bits
        black_passed_masks[square] &= ~rank_masks[i * 8 + file];
    }
  }
}
