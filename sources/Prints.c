#include "../headers/Prints.h"
#include "../headers/Attacks.h"
#include "../headers/Globals.h"
#include "../headers/Search.h"

void print_bitboard(U64 bitboard) {
  printf("\n");

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if (!file)
        printf("  %d ", 8 - rank);

      printf(" %d", get_bit(bitboard, square) ? 1 : 0);
    }
    printf("\n");
  }

  printf("\n     a b c d e f g h\n\n");
  printf("     Bitboard: %llu\n\n", bitboard);
}

void print_board() {
  printf("\n");

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if (!file)
        printf("  %d ", 8 - rank);

      int piece = -1;

      for (int bb_piece = P; bb_piece <= k; ++bb_piece) {
        if (get_bit(bitboards[bb_piece], square))
          piece = bb_piece;
      }

      printf(" %s", (piece == -1) ? "." : default_pieces[piece]);
    }
    printf("\n");
  }

  printf("\n     a b c d e f g h\n\n");
  printf("     Side:     %s\n", !side ? "white" : "black");
  printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
  printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-', (castle & wq) ? 'Q' : '-', (castle & bk) ? 'k' : '-', (castle & bq) ? 'q' : '-');

  // print hash key
  printf("     Hash key:  %llx\n\n", hash_key);
}

void print_attacked_squares(int side_) {
  printf("\n");

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if (!file)
        printf("  %d ", 8 - rank);

      printf(" %d", is_square_attacked(square, side_) ? 1 : 0);
    }
    printf("\n");
  }
  printf("\n     a b c d e f g h\n\n");
}

void print_move_list(const moves* move_list) {
  if (!move_list->count) {
    printf("\n     No move in the move list!\n");
    return;
  }

  printf("\n     move    piece     capture   double    enpass    castling\n\n");

  for (int move_count = 0; move_count < move_list->count; ++move_count) {
    int move = move_list->moves[move_count];
    printf("     %s%s%c   %s         %d         %d         %d         %d\n",
           square_to_coordinates[get_move_source(move)],
           square_to_coordinates[get_move_target(move)],
           get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
           default_pieces[get_move_piece(move)],
           get_move_capture(move) ? 1 : 0,
           get_move_double(move) ? 1 : 0,
           get_move_enpassant(move) ? 1 : 0,
           get_move_castling(move) ? 1 : 0);
  }

  printf("\n\n     Total number of moves: %d\n\n", move_list->count);
}

void print_move(int move) {
  if (get_move_promoted(move))
    printf("%s%s%c", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)]);
  else
    printf("%s%s", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)]);
}

// print move scores
void print_move_scores(moves* move_list) {
  printf("     Move scores:\n\n");

  // loop over moves within a move list
  for (int count = 0; count < move_list->count; ++count) {
    printf("     move: ");
    print_move(move_list->moves[count]);
    printf(" score: %d\n", score_move(move_list->moves[count]));
  }
}
