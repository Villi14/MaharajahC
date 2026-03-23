#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../headers/Defines.h"
#include "../headers/Fen.h"
#include "../headers/Globals.h"
#include "../headers/Moves.h"
#include "../headers/Perft.h"
#include "../headers/Search.h"
#include "../headers/UCI.h"
#include "../headers/Utils.h"

// parse user/GUI move string input (e.g. "e7e8q")
int parse_move(const char* move_string) {
  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // parse source square
  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

  // parse target square
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

  // loop over the moves within a move list
  for (int move_count = 0; move_count < move_list->count; ++move_count) {
    // init move
    int move = move_list->moves[move_count];

    // make sure source & target squares are available within the generated move
    if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
      // init promoted piece
      int promoted_piece = get_move_promoted(move);

      // promoted piece is available
      if (promoted_piece) {
        // promoted to queen
        if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
          // return legal move
          return move;

        // promoted to rook
        else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
          // return legal move
          return move;

        // promoted to bishop
        else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
          // return legal move
          return move;

        // promoted to knight
        else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
          // return legal move
          return move;

        // continue the loop on possible wrong promotions (e.g. "e7e8f")
        continue;
      }

      // return legal move
      return move;
    }
  }
  // return illegal move
  return 0;
}

/*  Example UCI commands to init position on chess board

    // init start position
    position startpos

    // init start position and make the moves on chess board
    position startpos moves e2e4 e7e5

    // init position from FEN string
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1

    // init position from fen string and make moves on chess board
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
*/
// parse UCI "position" command
void parse_position(char* command) {
  // shift pointer to the right where next token begins
  command += 9;

  // init pointer to the current character in the command string
  char* current_char = command;

  // parse UCI "startpos" command
  if (strncmp(command, "startpos", 8) == 0)
    // init chess board with start position
    parse_fen(start_position);

  // parse UCI "fen" command
  else {
    // make sure "fen" command is available within command string
    current_char = strstr(command, "fen");

    // if no "fen" command is available within command string
    if (current_char == nullptr)
      // init chess board with start position
      parse_fen(start_position);

    // found "fen" substring
    else {
      // shift pointer to the right where next token begins
      current_char += 4;

      // init chess board with position from FEN string
      parse_fen(current_char);
    }
  }

  // parse moves after position
  current_char = strstr(command, "moves");

  // moves available
  if (current_char != nullptr) {
    // shift pointer to the right where next token begins
    current_char += 6;

    // loop over moves within a move string
    while (*current_char) {
      // parse next move
      int move = parse_move(current_char);

      // if no more moves
      if (move == 0)
        // break out of the loop
        break;

      // increment repetition index
      repetition_index++;

      // wtire hash key into a repetition table
      repetition_table[repetition_index] = hash_key;

      // make move on the chess board
      make_move(move, all_moves);

      // move current character mointer to the end of current move
      while (*current_char && *current_char != ' ')
        current_char++;

      // go to the next move
      current_char++;
    }

    printf("%s\n", current_char);
  }
}

// parse UCI "go" command
void parse_go(char* command) {
  // reset time control
  reset_time_control();

  // init parameters
  int depth = -1;

  // init argument
  char* argument = NULL;

  // infinite search
  if ((argument = strstr(command, "infinite"))) {
  }

  // match UCI "binc" command
  if ((argument = strstr(command, "binc")) && side == black)
    // parse black time increment
    inc = atoi(argument + 5);

  // match UCI "winc" command
  if ((argument = strstr(command, "winc")) && side == white)
    // parse white time increment
    inc = atoi(argument + 5);

  // match UCI "wtime" command
  if ((argument = strstr(command, "wtime")) && side == white)
    // parse white time limit
    uci_time = atoi(argument + 6);

  // match UCI "btime" command
  if ((argument = strstr(command, "btime")) && side == black)
    // parse black time limit
    uci_time = atoi(argument + 6);

  // match UCI "movestogo" command
  if ((argument = strstr(command, "movestogo")))
    // parse number of moves to go
    movestogo = atoi(argument + 10);

  // match UCI "movetime" command
  if ((argument = strstr(command, "movetime")))
    // parse amount of time allowed to spend to make a move
    movetime = atoi(argument + 9);

  // match UCI "depth" command
  if ((argument = strstr(command, "depth")))
    // parse search depth
    depth = atoi(argument + 6);

  // if move time is not available
  if (movetime != -1) {
    // set time equal to move time
    uci_time = movetime;

    // set moves to go to 1
    movestogo = 1;
  }

  // init start time
  starttime = get_time_ms();

  // init search depth
  depth = depth;

  // if time control is available
  if (uci_time != -1) {
    // flag we're playing with time control
    timeset = 1;

    // set up timing
    uci_time /= movestogo;

    // disable time buffer when time is almost up
    if (uci_time > 1500)
      uci_time -= 50;

    // init stoptime
    stoptime = starttime + uci_time + inc;

    // treat increment as seconds per move when time is almost up
    if (uci_time < 1500 && inc && depth == 64)
      stoptime = starttime + inc - 50;
  }

  // if depth is not available
  if (depth == -1)
    // set depth to 64 plies (takes ages to complete...)
    depth = 64;

  // print debug info
  printf("time: %d  start: %u  stop: %u  depth: %d  timeset:%d\n", uci_time, starttime, stoptime, depth, timeset);

  // search position
  search_position(depth);
}

/*  GUI -> isready
    Engine -> readyok
    GUI -> ucinewgame
*/
// main UCI loop
void uci_loop() {
  // max hash MB
  int max_hash = 128;

  // default MB value
  int mb = 64;

// reset STDIN & STDOUT buffers
#ifdef _MSC_VER
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
#else
  setbuf(stdin, nullptr);
  setbuf(stdout, nullptr);
#endif

  // define user / GUI input buffer
  char input[2000];

  // print engine info
  printf("id name BBC %s\n", version);
  printf("id author Code Monkey King\n");
  printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
  printf("uciok\n");

  // main loop
  while (true) {
    // reset user /GUI input
    memset(input, 0, sizeof(input));

    // make sure output reaches the GUI
    fflush(stdout);

    // get user / GUI input
    if (!fgets(input, 2000, stdin))
      // continue the loop
      continue;

    // make sure input is available
    if (input[0] == '\n')
      // continue the loop
      continue;

    // parse UCI "isready" command
    if (strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
      continue;
    }

    // parse UCI "position" command
    else if (strncmp(input, "position", 8) == 0) {
      // call parse position function
      parse_position(input);

      // clear hash table
      clear_hash_table();
    }
    // parse UCI "ucinewgame" command
    else if (strncmp(input, "ucinewgame", 10) == 0) {
      // call parse position function
      parse_position("position startpos");

      // clear hash table
      clear_hash_table();
    }
    // parse UCI "go" command
    else if (strncmp(input, "go", 2) == 0)
      // call parse go function
      parse_go(input);

    // parse UCI "quit" command
    else if (strncmp(input, "quit", 4) == 0)
      // quit from the UCI loop (terminate program)
      break;

    // parse UCI "uci" command
    else if (strncmp(input, "uci", 3) == 0) {
      // print engine info
      printf("id name BBC %s\n", version);
      printf("id author Code Monkey King\n");
      printf("uciok\n");
    }

    else if (!strncmp(input, "setoption name Hash value ", 26)) {
      // init MB
      sscanf(input, "%*s %*s %*s %*s %d", &mb);

      // adjust MB if going beyond the aloowed bounds
      if (mb < 4)
        mb = 4;
      if (mb > max_hash)
        mb = max_hash;

      // set hash table size in MB
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
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
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
  FD_SET(fileno(stdin), &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#endif
}

// read GUI/user input
void read_input() {
  // bytes to read holder
  int bytes;

  // GUI/user input
  char input[256] = "", *endc;

  // "listen" to STDIN
  if (input_waiting()) {
    // tell engine to stop calculating
    stopped = 1;

    // loop to read bytes from STDIN
    do {
      // read bytes from STDIN
      bytes = read(fileno(stdin), input, 256);
    }

    // until bytes available
    while (bytes < 0);

    // searches for the first occurrence of '\n'
    endc = strchr(input, '\n');

    // if found new line set value at pointer to 0
    if (endc)
      *endc = 0;

    // if input is available
    if (strlen(input) > 0) {
      // match UCI "quit" command
      if (!strncmp(input, "quit", 4))
        // tell engine to terminate exacution
        quit = 1;

      // // match UCI "stop" command
      else if (!strncmp(input, "stop", 4))
        // tell engine to terminate exacution
        quit = 1;
    }
  }
}

// a bridge function to interact between search and GUI input
void communicate() {
  // if time is up break here
  if (timeset == 1 && get_time_ms() > stoptime) {
    // tell engine to stop calculating
    stopped = 1;
  }

  // read GUI input
  read_input();
}
