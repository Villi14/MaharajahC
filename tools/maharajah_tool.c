#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/engine/Attacks.h"
#include "../include/engine/CustomSetup.h"
#include "../include/engine/Globals.h"
#include "../include/engine/Inits.h"
#include "../include/engine/Moves.h"
#include "../include/engine/Search.h"
#include "../include/engine/Transposition.h"
#include "../include/ffi/maharajah_ffi.h"
#include "../include/perft/Perft.h"
#include "../include/util/Defines.h"

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

static void board_to_fen(char* out, size_t out_len) {
  custom_position_board_to_fen(out, (int)out_len);
}

static void json_write_string(FILE* out, const char* value) {
  fputc('"', out);
  while (*value != '\0') {
    if (*value == '\\' || *value == '"') {
      fputc('\\', out);
    }
    fputc(*value, out);
    ++value;
  }
  fputc('"', out);
}

static void json_write_game(FILE* out, int first_game, int game_number,
                            const char* start_fen, const char* result,
                            const char* end_fen, char (*moves)[8],
                            int move_count) {
  if (!first_game) {
    fprintf(out, ",\n");
  }

  fprintf(out, "  {\n    \"game\": %d,\n    \"start_fen\": ", game_number);
  json_write_string(out, start_fen);
  fprintf(out, ",\n    \"result\": ");
  json_write_string(out, result);
  fprintf(out, ",\n    \"end_fen\": ");
  json_write_string(out, end_fen);
  fprintf(out, ",\n    \"moves\": [");

  for (int index = 0; index < move_count; ++index) {
    if (index > 0) {
      fprintf(out, ", ");
    }
    json_write_string(out, moves[index]);
  }

  fprintf(out, "]\n  }");
}

static int run_generate_command(int count, unsigned int seed) {
  char fen[128] = {0};

  srand(seed);

  if (!mah_init()) {
    fprintf(stderr, "maharajah_tool: failed to initialize engine.\n");
    return 1;
  }

  for (int index = 0; index < count; ++index) {
    const int side_to_move = rand() % 2;
    if (!mah_generate_custom_position_fen(side_to_move, seed + (unsigned int)index,
                                          fen, (int)sizeof(fen))) {
      fprintf(stderr,
              "maharajah_tool: failed to generate a valid custom position.\n");
      mah_shutdown();
      return 1;
    }

    printf("%s\n", fen);
  }

  mah_shutdown();
  return 0;
}

static int current_result(char* out_result, size_t out_len) {
  const int side_to_move = board.side;
  const int white_king_missing = board.bitboards[K] == 0ULL;
  const int black_king_missing = board.bitboards[k] == 0ULL;
  const int white_king_square = white_king_missing ? no_sq : get_ls1b_index(board.bitboards[K]);
  const int black_king_square = black_king_missing ? no_sq : get_ls1b_index(board.bitboards[k]);

  if (white_king_missing || black_king_missing) {
    snprintf(out_result, out_len, "%s",
             white_king_missing ? "black_checkmate" : "white_checkmate");
    return 1;
  }

  const int in_check = side_to_move == white
                           ? is_square_attacked(white_king_square, black)
                           : is_square_attacked(black_king_square, white);
  const int legal_moves = count_legal_moves_for_side(side_to_move);

  if (legal_moves > 0) {
    snprintf(out_result, out_len, "ongoing");
    return 0;
  }

  if (in_check) {
    snprintf(out_result, out_len, "%s",
             side_to_move == white ? "black_checkmate" : "white_checkmate");
    return 1;
  }

  snprintf(out_result, out_len, "stalemate");
  return 1;
}

static int run_selfplay_command(int games, int depth, int max_plies,
                                unsigned int seed, const char* json_path) {
  char move[16] = {0};
  char result[32] = {0};
  FILE* log_file = NULL;
  int first_logged_game = 1;

  srand(seed);

  if (!mah_init()) {
    fprintf(stderr, "maharajah_tool: failed to initialize engine.\n");
    return 1;
  }

  if (json_path != NULL) {
    log_file = fopen(json_path, "w");
    if (log_file == NULL) {
      fprintf(stderr, "maharajah_tool: failed to open JSON log file.\n");
      mah_shutdown();
      return 1;
    }
    fprintf(log_file, "[\n");
  }

  for (int game = 0; game < games; ++game) {
    const int side_to_move = rand() % 2;
    char (*move_history)[8] = calloc((size_t)max_plies, sizeof(*move_history));
    int move_count = 0;
    char start_fen[128] = {0};
    char end_fen[128] = {0};

    if (move_history == NULL) {
      fprintf(stderr, "maharajah_tool: failed to allocate move history.\n");
      if (log_file != NULL) {
        fprintf(log_file, "\n]\n");
        fclose(log_file);
      }
      mah_shutdown();
      return 1;
    }

    if (!mah_generate_custom_position_fen(side_to_move, seed + (unsigned int)game,
                                          start_fen, (int)sizeof(start_fen))) {
      fprintf(stderr,
              "maharajah_tool: failed to generate a valid custom position.\n");
      free(move_history);
      if (log_file != NULL) {
        fprintf(log_file, "\n]\n");
        fclose(log_file);
      }
      mah_shutdown();
      return 1;
    }

    if (!mah_set_position_fen(start_fen)) {
      fprintf(stderr, "maharajah_tool: failed to load generated FEN.\n");
      free(move_history);
      if (log_file != NULL) {
        fprintf(log_file, "\n]\n");
        fclose(log_file);
      }
      mah_shutdown();
      return 1;
    }

    printf("game %d start %s\n", game + 1, start_fen);

    for (int ply = 0; ply < max_plies; ++ply) {
      memset(move, 0, sizeof(move));
      if (!mah_best_move_depth(depth, move, (int)sizeof(move))) {
        current_result(result, sizeof(result));
        printf("game %d result %s at ply %d\n", game + 1, result, ply);
        break;
      }

      printf("game %d ply %d %s\n", game + 1, ply + 1, move);
      snprintf(move_history[move_count++], 8, "%s", move);
      if (!mah_apply_move(move)) {
        fprintf(stderr, "maharajah_tool: engine produced an invalid move.\n");
        free(move_history);
        if (log_file != NULL) {
          fprintf(log_file, "\n]\n");
          fclose(log_file);
        }
        mah_shutdown();
        return 1;
      }

      if (current_result(result, sizeof(result))) {
        board_to_fen(end_fen, sizeof(end_fen));
        printf("game %d result %s fen %s\n", game + 1, result, end_fen);
        break;
      }

      if (ply == max_plies - 1) {
        board_to_fen(end_fen, sizeof(end_fen));
        snprintf(result, sizeof(result), "move_limit");
        printf("game %d result move_limit fen %s\n", game + 1, end_fen);
      }
    }

    if (end_fen[0] == '\0') {
      board_to_fen(end_fen, sizeof(end_fen));
      current_result(result, sizeof(result));
    }

    if (log_file != NULL) {
      json_write_game(log_file, first_logged_game, game + 1, start_fen, result,
                      end_fen, move_history, move_count);
      first_logged_game = 0;
    }

    free(move_history);
  }

  if (log_file != NULL) {
    fprintf(log_file, "\n]\n");
    fclose(log_file);
  }

  mah_shutdown();
  return 0;
}

typedef struct {
  const char* name;
  const char* fen;
  int depth;
} BenchPosition;

static int run_bench_command(void) {
  static const BenchPosition positions[] = {
    { "startpos", start_position, 3 },
    { "archbishop_capture", "k7/5q2/8/4A3/8/8/8/K7 w - - 0 1 ", 2 },
    { "chancellor_capture", "7k/8/8/3C4/8/8/3q4/K7 w - - 0 1 ", 2 },
    { "amazon_capture", "k7/8/8/3M4/8/8/8/K6q w - - 0 1 ", 2 },
    { "defensive_escape", "6k1/8/8/8/3M4/8/6q1/6RK w - - 0 1 ", 2 }
  };

  unsigned long long total_nodes = 0ULL;
  int total_time_ms = 0;
  char move[16] = {0};

  if (!mah_init()) {
    fprintf(stderr, "maharajah_tool: failed to initialize engine.\n");
    return 1;
  }

  if (!mah_set_difficulty_level(5)) {
    fprintf(stderr, "maharajah_tool: failed to set benchmark difficulty.\n");
    mah_shutdown();
    return 1;
  }

  printf("bench suite positions=%zu difficulty=5 mode=classic\n",
         sizeof(positions) / sizeof(positions[0]));

  for (size_t index = 0; index < sizeof(positions) / sizeof(positions[0]); ++index) {
    const BenchPosition* position = &positions[index];
    const int start_ms = get_time_ms();
    unsigned long long nodes = 0ULL;
    int elapsed_ms = 0;

    if (!mah_set_position_fen(position->fen)) {
      fprintf(stderr, "maharajah_tool: bench could not set FEN for %s.\n",
              position->name);
      mah_shutdown();
      return 1;
    }

    memset(move, 0, sizeof(move));
    if (!mah_best_move_depth(position->depth, move, (int)sizeof(move))) {
      fprintf(stderr, "maharajah_tool: bench produced no move for %s.\n",
              position->name);
      mah_shutdown();
      return 1;
    }

    elapsed_ms = get_time_ms() - start_ms;
    nodes = search_context.nodes;
    total_nodes += nodes;
    total_time_ms += elapsed_ms;

    printf("bench case=%s depth=%d move=%s nodes=%llu time_ms=%d",
           position->name, position->depth, move, nodes, elapsed_ms);
    if (elapsed_ms > 0) {
      printf(" nps=%llu", (nodes * 1000ULL) / (unsigned long long)elapsed_ms);
    }
    printf("\n");
  }

  printf("bench summary positions=%zu total_nodes=%llu total_time_ms=%d",
         sizeof(positions) / sizeof(positions[0]), total_nodes, total_time_ms);
  if (total_time_ms > 0) {
    printf(" avg_nps=%llu", (total_nodes * 1000ULL) / (unsigned long long)total_time_ms);
  }
  printf("\n");

  mah_shutdown();
  return 0;
}

static void print_usage(const char* program) {
  fprintf(stderr,
          "Usage:\n"
          "  %s generate [count] [seed]\n"
          "  %s selfplay [games] [depth] [max_plies] [seed] [json_path]\n"
          "  %s bench\n",
          program, program, program);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  if (strcmp(argv[1], "generate") == 0) {
    const int count = argc >= 3 ? atoi(argv[2]) : 1;
    const unsigned int seed = argc >= 4
                                  ? (unsigned int)strtoul(argv[3], NULL, 10)
                                  : 1U;
    return run_generate_command(count > 0 ? count : 1, seed);
  }

  if (strcmp(argv[1], "selfplay") == 0) {
    const int games = argc >= 3 ? atoi(argv[2]) : 1;
    const int depth = argc >= 4 ? atoi(argv[3]) : 1;
    const int max_plies = argc >= 5 ? atoi(argv[4]) : 100;
    const unsigned int seed = argc >= 6
                                  ? (unsigned int)strtoul(argv[5], NULL, 10)
                                  : 1U;
    const char* json_path = argc >= 7 ? argv[6] : NULL;

    return run_selfplay_command(games > 0 ? games : 1, depth > 0 ? depth : 1,
                                max_plies > 0 ? max_plies : 100, seed,
                                json_path);
  }

  if (strcmp(argv[1], "bench") == 0) {
    return run_bench_command();
  }

  print_usage(argv[0]);
  return 1;
}
