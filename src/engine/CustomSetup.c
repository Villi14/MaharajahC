#include "../../include/engine/CustomSetup.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/board/Fen.h"
#include "../../include/engine/Attacks.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Moves.h"
#include "../../include/engine/Zobrist.h"
#include "../../include/util/Defines.h"

typedef struct {
  int white_piece;
  int black_piece;
  int weight;
} GeneratorPiece;

static const GeneratorPiece generator_pieces[] = {
  {P, p, 1},
  {N, n, 3},
  {B, b, 3},
  {R, r, 5},
  {Q, q, 9},
  {A, a, 6},
  {C, c, 8},
  {M, m, 13},
};

static const int preferred_piece_counts[8] = {8, 2, 2, 2, 1, 1, 1, 1};

static int piece_at_square(int square) {
  for (int piece = P; piece <= k; ++piece) {
    if (get_bit(board.bitboards[piece], square)) {
      return piece;
    }
  }

  return -1;
}

static char piece_to_fen_char(int piece) {
  switch (piece) {
  case P:
    return 'P';
  case N:
    return 'N';
  case B:
    return 'B';
  case R:
    return 'R';
  case Q:
    return 'Q';
  case A:
    return 'A';
  case C:
    return 'C';
  case M:
    return 'M';
  case K:
    return 'K';
  case p:
    return 'p';
  case n:
    return 'n';
  case b:
    return 'b';
  case r:
    return 'r';
  case q:
    return 'q';
  case a:
    return 'a';
  case c:
    return 'c';
  case m:
    return 'm';
  case k:
    return 'k';
  default:
    return '?';
  }
}

static int row_is_full(int row) {
  for (int file = 0; file < 8; ++file) {
    if (piece_at_square(row * 8 + file) < 0) {
      return 0;
    }
  }

  return 1;
}

static int first_open_row_for_player(int player) {
  if (player == white) {
    for (int row = 7; row >= 4; --row) {
      if (!row_is_full(row)) {
        return row;
      }
    }
    return -1;
  }

  for (int row = 0; row <= 3; ++row) {
    if (!row_is_full(row)) {
      return row;
    }
  }
  return -1;
}

static int total_remaining_slots(int player) {
  int remaining = 0;
  const int start_row = player == white ? 7 : 0;
  const int end_row = player == white ? 4 : 3;
  const int step = player == white ? -1 : 1;

  for (int row = start_row;; row += step) {
    for (int file = 0; file < 8; ++file) {
      if (piece_at_square(row * 8 + file) < 0) {
        ++remaining;
      }
    }
    if (row == end_row) {
      break;
    }
  }

  return remaining;
}

static int candidate_score(int piece_index, int file, int height,
                           const int piece_counts[8], int remaining_budget,
                           int remaining_slots) {
  static const int placement_scores[8][4] = {
    {2, 8, 16, 22},
    {12, 16, 12, 7},
    {12, 15, 11, 6},
    {18, 11, 5, 2},
    {17, 9, 3, 1},
    {15, 11, 6, 2},
    {16, 10, 4, 1},
    {14, 8, 2, 1},
  };
  const int remaining_after =
      remaining_budget - generator_pieces[piece_index].weight;
  const int centrality_bonus = 3 - abs(3 - file);
  int score = placement_scores[piece_index][height];

  if (piece_counts[piece_index] >= preferred_piece_counts[piece_index]) {
    score -=
        8 * (piece_counts[piece_index] - preferred_piece_counts[piece_index] + 1);
  }

  if (height == 0 && file != 4) {
    score += 2;
  }

  if (piece_index == 1 || piece_index == 2 || piece_index == 5) {
    score += centrality_bonus;
  }

  if (piece_index == 0 && remaining_slots <= 6) {
    score += 4;
  }

  if (remaining_after == 0) {
    score += 10;
  }

  if (remaining_after > 0 && remaining_after < 3 && piece_index != 0) {
    score -= 5;
  }

  return score > 0 ? score : 1;
}

static void place_piece(int piece, int square) {
  set_bit(board.bitboards[piece], square);
}

static void reset_custom_board(void) {
  reset_board();
  place_piece(K, e1);
  place_piece(k, e8);
}

static void rebuild_board_state(int side_to_move) {
  memset(board.occupancies, 0, sizeof(board.occupancies));

  for (int piece = P; piece <= K; ++piece) {
    board.occupancies[white] |= board.bitboards[piece];
  }

  for (int piece = p; piece <= k; ++piece) {
    board.occupancies[black] |= board.bitboards[piece];
  }

  board.occupancies[both] = board.occupancies[white] | board.occupancies[black];
  board.side = side_to_move;
  board.castle = 0;
  board.enpassant = no_sq;
  board.halfmove_clock = 0;
  board.standard_rules = 0;
  board.hash_key = generate_hash_key();
}

static int count_legal_moves_for_side(int side) {
  MoveList move_list = {.count = 0};
  const int original_side = board.side;
  int legal_moves = 0;

  board.side = side;
  generate_moves(&move_list);

  for (int index = 0; index < move_list.count; ++index) {
    copy_board();
    if (make_move(move_list.moves[index], all_moves)) {
      ++legal_moves;
    }
    take_back();
  }

  board.side = original_side;
  return legal_moves;
}

static int validate_generated_position(int side_to_move) {
  if (side_to_move == white && is_square_attacked(e1, black)) {
    return 0;
  }

  if (side_to_move == black && is_square_attacked(e8, white)) {
    return 0;
  }

  return count_legal_moves_for_side(side_to_move) > 0;
}

static int generate_side(int player) {
  int piece_counts[8] = {0};
  int remaining_budget = 39;

  while (remaining_budget > 0) {
    const int active_row = first_open_row_for_player(player);
    const int row_depth = player == white ? 7 - active_row : active_row;
    const int slots_before = total_remaining_slots(player);
    int total_score = 0;
    int chosen_square = -1;
    int chosen_piece_index = -1;
    int roll = 0;

    if (active_row < 0) {
      return 0;
    }

    for (int file = 0; file < 8; ++file) {
      const int square = active_row * 8 + file;
      if (piece_at_square(square) >= 0) {
        continue;
      }

      for (size_t index = 0;
           index < sizeof(generator_pieces) / sizeof(generator_pieces[0]);
           ++index) {
        const int weight = generator_pieces[index].weight;
        const int remaining_after = remaining_budget - weight;

        if (weight > remaining_budget) {
          continue;
        }

        if (remaining_after > slots_before - 1) {
          continue;
        }

        total_score += candidate_score((int)index, file, row_depth,
                                       piece_counts, remaining_budget,
                                       slots_before);
      }
    }

    if (total_score <= 0) {
      return 0;
    }

    roll = rand() % total_score;

    for (int file = 0; file < 8 && chosen_square < 0; ++file) {
      const int square = active_row * 8 + file;
      if (piece_at_square(square) >= 0) {
        continue;
      }

      for (size_t index = 0;
           index < sizeof(generator_pieces) / sizeof(generator_pieces[0]);
           ++index) {
        const int weight = generator_pieces[index].weight;
        const int remaining_after = remaining_budget - weight;
        int score = 0;

        if (weight > remaining_budget) {
          continue;
        }

        if (remaining_after > slots_before - 1) {
          continue;
        }

        score = candidate_score((int)index, file, row_depth, piece_counts,
                                remaining_budget, slots_before);
        if (roll < score) {
          chosen_square = square;
          chosen_piece_index = (int)index;
          break;
        }
        roll -= score;
      }
    }

    if (chosen_square < 0 || chosen_piece_index < 0) {
      return 0;
    }

    {
      const GeneratorPiece selected = generator_pieces[chosen_piece_index];
      place_piece(player == white ? selected.white_piece : selected.black_piece,
                  chosen_square);
      piece_counts[chosen_piece_index]++;
      remaining_budget -= selected.weight;
    }
  }

  return 1;
}

int generate_reasonable_custom_position(int side_to_move, unsigned int seed) {
  srand(seed);

  if (side_to_move != white && side_to_move != black) {
    side_to_move = rand() % 2;
  }

  for (int attempt = 0; attempt < 5000; ++attempt) {
    reset_custom_board();

    if (!generate_side(white)) {
      continue;
    }

    if (!generate_side(black)) {
      continue;
    }

    rebuild_board_state(side_to_move);

    if (validate_generated_position(side_to_move)) {
      return 1;
    }
  }

  return 0;
}

int custom_position_board_to_fen(char* out, int out_len) {
  size_t offset = 0;
  const size_t capacity = out_len > 0 ? (size_t)out_len : 0;

  if (out == NULL || out_len <= 0) {
    return 0;
  }

  out[0] = '\0';

  for (int rank = 0; rank < 8; ++rank) {
    int empty = 0;

    for (int file = 0; file < 8; ++file) {
      const int square = rank * 8 + file;
      const int piece = piece_at_square(square);

      if (piece < 0) {
        ++empty;
        continue;
      }

      if (empty > 0) {
        if (offset >= capacity) {
          return 0;
        }
        offset += (size_t)snprintf(out + offset, capacity - offset, "%d", empty);
        empty = 0;
      }

      if (offset >= capacity) {
        return 0;
      }
      offset +=
          (size_t)snprintf(out + offset, capacity - offset, "%c", piece_to_fen_char(piece));
    }

    if (empty > 0) {
      if (offset >= capacity) {
        return 0;
      }
      offset += (size_t)snprintf(out + offset, capacity - offset, "%d", empty);
    }

    if (rank < 7) {
      if (offset >= capacity) {
        return 0;
      }
      offset += (size_t)snprintf(out + offset, capacity - offset, "/");
    }
  }

  if (offset >= capacity) {
    return 0;
  }

  snprintf(out + offset, capacity - offset, " %c - - 0 1",
           board.side == white ? 'w' : 'b');
  return 1;
}
