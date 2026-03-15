#include <stdio.h>

#include "Attacks.h"
#include "Functions.h"
#include "Globals.h"
#include "Prints.h"

// print bitboard
void print_bitboard(U64 bitboard) {
  // print offset
  printf("\n");

  // loop over board ranks
  for (int rank = 0; rank < 8; ++rank) {
    // loop over board files
    for (int file = 0; file < 8; ++file) {
      // convert file & rank into square index
      int square = rank * 8 + file;

      // print ranks
      if (!file)
        printf("  %d ", 8 - rank);

      // print bit state (either 1 or 0)
      printf(" %d", get_bit(bitboard, square) ? 1 : 0);
    }

    // print new line every rank
    printf("\n");
  }

  // print board files
  printf("\n     a b c d e f g h\n\n");

  // print bitboard as unsigned decimal number
  printf("     Bitboard: %llu\n\n", bitboard);
}

// print board
void print_board() {
  // print offset
  printf("\n");

  // loop over board ranks
  for (int rank = 0; rank < 8; ++rank) {
    // loop over board files
    for (int file = 0; file < 8; ++file) {
      // init square
      int square = rank * 8 + file;

      // print ranks
      if (!file)
        printf("  %d ", 8 - rank);

      // define piece variable
      int piece = -1;

      // loop over all piece bitboards
      for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
        // if there is a piece on current square
        if (get_bit(bitboards[bb_piece], square))
          // get piece code
          piece = bb_piece;
      }

      // print different piece set depending on OS
      printf(" %s", (piece == -1) ? "." : default_pieces[piece]);
    }

    // print new line every rank
    printf("\n");
  }

  // print board files
  printf("\n     a b c d e f g h\n\n");

  // print side to move
  printf("     Side:     %s\n", !side ? "white" : "black");

  // print enpassant square
  printf("     Enpassant:   %s\n",
         (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");

  // print castling rights
  printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
         (castle & wq) ? 'Q' : '-', (castle & bk) ? 'k' : '-',
         (castle & bq) ? 'q' : '-');
}

// print attacked squares
void print_attacked_squares(int side_) {
  printf("\n");

  // loop over board ranks
  for (int rank = 0; rank < 8; ++rank) {
    // loop over board files
    for (int file = 0; file < 8; ++file) {
      // init square
      int square = rank * 8 + file;

      // print ranks
      if (!file)
        printf("  %d ", 8 - rank);

      // check whether current square is attacked or not
      printf(" %d", is_square_attacked(square, side_) ? 1 : 0);
    }

    // print new line every rank
    printf("\n");
  }

  // print files
  printf("\n     a b c d e f g h\n\n");
}

// print move list
void print_move_list(const moves *move_list) {
  // do nothing on empty move list
  if (!move_list->count) {
    printf("\n     No move in the move list!\n");
    return;
  }

  printf("\n     move    piece     capture   double    enpass    castling\n\n");

  // loop over moves within a move list
  for (int move_count = 0; move_count < move_list->count; ++move_count) {
    // init move
    int move = move_list->moves[move_count];
    // print move
    printf("     %s%s%c   %s         %d         %d         %d         %d\n",
           square_to_coordinates[get_move_source(move)],
           square_to_coordinates[get_move_target(move)],
           get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)]
                                   : ' ',
           default_pieces[get_move_piece(move)], get_move_capture(move) ? 1 : 0,
           get_move_double(move) ? 1 : 0, get_move_enpassant(move) ? 1 : 0,
           get_move_castling(move) ? 1 : 0);
  }

  // print total number of moves
  printf("\n\n     Total number of moves: %d\n\n", move_list->count);
}

// print move (for UCI purposes)
void print_move(int move) {
  printf("%s%s%c\n", square_to_coordinates[get_move_source(move)],
         square_to_coordinates[get_move_target(move)],
         promoted_pieces[get_move_promoted(move)]);
}