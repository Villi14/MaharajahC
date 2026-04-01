#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <io.h>
#include <stdbool.h>
#include <windows.h>
#define STDIN_FILENO 0
#elif defined(__GNUC__) || defined(__clang__)
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "maharajah/board/Fen.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/engine/Search.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/perft/Perft.h"
#include "maharajah/uci/UCI.h"
#include "maharajah/util/Defines.h"
#include "maharajah/util/Utils.h"

int parse_move(const char* move_string) {
  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);

  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

  for (int move_count = 0; move_count < move_list.count; ++move_count) {
    int move = move_list.moves[move_count];

    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
      int promoted_piece = get_move_promoted(move);
      if (promoted_piece) {
        if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
          return move;
        else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
          return move;
        else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
          return move;
        else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
          return move;
        continue;
      }

      return move;
    }
  }

  for (int move_count = 0; move_count < move_list.count; ++move_count) {
    int move = move_list.moves[move_count];

    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
      int promoted_piece = get_move_promoted(move);

      if (promoted_piece) {
        if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
          return move;
        else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
          return move;
        else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
          return move;
        else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
          return move;
        continue;
      }

      return move;
    }
  }

  return 0;
}

void parse_position(char* command) {
  command += 9;
  char* current_char = command;

  if (strncmp(command, "startpos", 8) == 0)
    parse_fen(start_position);
  else {
    current_char = strstr(command, "fen");
    if (current_char == nullptr)
      parse_fen(start_position);
    else {
      current_char += 4;
      parse_fen(current_char);
    }
  }

  current_char = strstr(command, "moves");

  if (current_char != nullptr) {
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

void parse_go(char* command) {
  reset_time_control();
  int depth = -1;
  char* argument = nullptr;
  if ((argument = strstr(command, "infinite"))) {
  }
  if ((argument = strstr(command, "binc")) && board.side == black)
    time_controls.inc = atoi(argument + 5);

  if ((argument = strstr(command, "winc")) && board.side == white)
    time_controls.inc = atoi(argument + 5);

  if ((argument = strstr(command, "wtime")) && board.side == white)
    time_controls.uci_time = atoi(argument + 6);

  if ((argument = strstr(command, "btime")) && board.side == black)
    time_controls.uci_time = atoi(argument + 6);

  if ((argument = strstr(command, "movestogo")))
    time_controls.movestogo = atoi(argument + 10);

  if ((argument = strstr(command, "movetime")))
    time_controls.movetime = atoi(argument + 9);

  if ((argument = strstr(command, "depth")))
    depth = atoi(argument + 6);

  if (time_controls.movetime != -1) {
    time_controls.uci_time = time_controls.movetime;
    time_controls.movestogo = 1;
  }

  time_controls.starttime = get_time_ms();

  if (time_controls.uci_time != -1) {
    time_controls.timeset = 1;
    time_controls.uci_time /= time_controls.movestogo;
    if (time_controls.uci_time > 1500)
      time_controls.uci_time -= 50;
    time_controls.stoptime = time_controls.starttime + time_controls.uci_time + time_controls.inc;
    if (time_controls.uci_time < 1500 && time_controls.inc && depth == 64)
      time_controls.stoptime = time_controls.starttime + time_controls.inc - 50;
  }

  if (depth == -1)
    depth = 0x40;

  printf("time: %d  start: %u  stop: %u  depth: %d  timeset:%d\n",
         time_controls.uci_time,
         time_controls.starttime,
         time_controls.stoptime,
         depth,
         time_controls.timeset);

  search_position(depth);
}

void uci_loop() {
  int max_hash = 0x80;
  int mb = 0x40;
  setbuf(stdin, nullptr);
  setbuf(stdout, nullptr);

  char input[2000];
  printf("id name Maharajah %s\n", version);
  printf("id author Villi\n");
  printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
  printf("uciok\n");

  while (true) {
    memset(input, 0, sizeof(input));
    fflush(stdout);

    if (!fgets(input, 2000, stdin))
      continue;

    if (input[0] == '\n')
      continue;

    if (strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
      continue;
    } else if (strncmp(input, "position", 8) == 0) {
      parse_position(input);
      clear_hash_table();
    } else if (strncmp(input, "ucinewgame", 10) == 0) {
      parse_position("position startpos");
      clear_hash_table();
    } else if (strncmp(input, "go", 2) == 0)
      parse_go(input);
    else if (strncmp(input, "quit", 4) == 0)
      break;
    else if (strncmp(input, "uci", 3) == 0) {
      printf("id name Maharajah %s\n", version);
      printf("id author Villi\n");
      printf("uciok\n");
    } else if (!strncmp(input, "setoption name Hash value ", 26)) {
      sscanf(input, "%*s %*s %*s %*s %d", &mb);
      if (mb < 4)
        mb = 4;
      if (mb > max_hash)
        mb = max_hash;
      printf("    Set hash table size to %dMB\n", mb);

      init_hash_table(mb);
    }
  }
}

int input_waiting() {
#ifdef _MSC_VER
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init) {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe) {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe) {
    if (!PeekNamedPipe(inh, nullptr, 0, nullptr, &dw, nullptr))
      return 1;
    return dw;
  }

  else {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
  }
#elif defined(__GNUC__) || defined(__clang__)
  fd_set readfds;
  struct timeval tv;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(STDIN_FILENO, &readfds));
#endif
}

void read_input() {
  int bytes;
  char input[0x100] = "", *endc;

  if (input_waiting()) {
    time_controls.stopped = 1;

    do {
      bytes = read(STDIN_FILENO, input, 0x100);
    } while (bytes < 0);
    endc = strchr(input, '\n');

    if (endc)
      *endc = 0;
    if (strlen(input) > 0) {
      if (!strncmp(input, "quit", 4))
        time_controls.quit = 1;
      else if (!strncmp(input, "stop", 4))
        time_controls.stopped = 1;
    }
  }
}

void communicate() {
  if (time_controls.timeset == 1 && get_time_ms() > time_controls.stoptime) {
    time_controls.stopped = 1;
  }
  read_input();
}
