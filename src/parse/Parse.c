#include <string.h>

#include "../../include/board/Fen.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Moves.h"
#include "../../include/parse/Parse.h"
#include "../../include/util/Defines.h"

int parse_move(const char* move_string) {
  size_t move_len = 0;

  if (move_string == NULL)
    return 0;
  if (move_string[0] == '\0' || move_string[1] == '\0' || move_string[2] == '\0' || move_string[3] == '\0')
    return 0;

  while (move_string[move_len] != '\0' &&
         move_string[move_len] != ' ' &&
         move_string[move_len] != '\n' &&
         move_string[move_len] != '\r' &&
         move_string[move_len] != '\t') {
    ++move_len;
  }

  if (move_len != 4 && move_len != 5)
    return 0;

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);

  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

  for (int move_count = 0; move_count < move_list.count; ++move_count) {
    int move = move_list.moves[move_count];

    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
      int promoted_piece = get_move_promoted(move);
      if (promoted_piece) {
        if (move_len != 5)
          continue;
        char promo = move_string[4];
        if ((promoted_piece == Q || promoted_piece == q) && promo == 'q')
          return move;
        else if ((promoted_piece == R || promoted_piece == r) && promo == 'r')
          return move;
        else if ((promoted_piece == B || promoted_piece == b) && promo == 'b')
          return move;
        else if ((promoted_piece == N || promoted_piece == n) && promo == 'n')
          return move;
        else if ((promoted_piece == A || promoted_piece == a) && promo == 'a')
          return move;
        else if ((promoted_piece == C || promoted_piece == c) && promo == 'c')
          return move;
        else if ((promoted_piece == M || promoted_piece == m) && promo == 'm')
          return move;
        continue;
      }

      if (move_len != 4)
        continue;
      return move;
    }
  }

  return 0;
}

void parse_position(char* command) {
  if (command == NULL)
    return;

  command += 9;
  char* current_char = command;

  if (strncmp(command, "startpos", 8) == 0)
    parse_fen(start_position);
  else {
    current_char = strstr(command, "fen");
    if (current_char == NULL)
      parse_fen(start_position);
    else {
      current_char += 4;
      parse_fen(current_char);
    }
  }

  current_char = strstr(command, "moves");

  if (current_char != NULL) {
    current_char += 6;
    while (*current_char) {
      int move = parse_move(current_char);
      if (move == 0)
        break;
      ++search_context.repetition_index;
      search_context.repetition_table[search_context.repetition_index] = board.hash_key;
      make_move(move, all_moves);
      while (*current_char && *current_char != ' ')
        ++current_char;
      ++current_char;
    }
  }
}
