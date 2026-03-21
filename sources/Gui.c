#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../headers/Defines.h"
#include "../headers/Fen.h"
#include "../headers/Functions.h"
#include "../headers/Globals.h"

// parse user/GUI move string input (e.g. "e7e8q")
int parse_move(char* move_string) {
  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // parse source square
  int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

  // parse target square
  int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

  // loop over the moves within a move list
  for(int move_count = 0; move_count < move_list->count; ++move_count) {
    // init move
    int move = move_list->moves[move_count];

    // make sure source & target squares are available within the generated move
    if(source_square == get_move_source(move) && target_square == get_move_target(move)) {
      // init promoted piece
      int promoted_piece = get_move_promoted(move);

      // promoted piece is available
      if(promoted_piece) {
        // promoted to queen
        if((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
          // return legal move
          return move;

        // promoted to rook
        else if((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
          // return legal move
          return move;

        // promoted to bishop
        else if((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
          // return legal move
          return move;

        // promoted to knight
        else if((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
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
  if(strncmp(command, "startpos", 8) == 0)
    // init chess board with start position
    parse_fen(start_position);

  // parse UCI "fen" command
  else {
    // make sure "fen" command is available within command string
    current_char = strstr(command, "fen");

    // if no "fen" command is available within command string
    if(current_char == nullptr)
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
  if(current_char != nullptr) {
    // shift pointer to the right where next token begins
    current_char += 6;

    // loop over moves within a move string
    while(*current_char) {
      // parse next move
      int move = parse_move(current_char);

      // if no more moves
      if(move == 0)
        // break out of the loop
        break;

      // make move on the chess board
      make_move(move, all_moves);

      // move current character mointer to the end of current move
      while(*current_char && *current_char != ' ')
        current_char++;

      // go to the next move
      current_char++;
    }

    printf("%s\n", current_char);
  }
}

// parse UCI "go" command
void parse_go(char* command) {
  // init depth
  int depth = -1;

  // init character pointer to the current depth argument
  char* current_depth = strstr(command, "depth");

  // handle fixed depth search
  if(current_depth != nullptr)
    //convert string to integer and assign the result value to depth
    depth = atoi(current_depth + 6);

  // different time controls placeholder
  else
    depth = 6;

  // search position
  // search_position(depth);
  printf("depth: %d\n", depth);
}

/*  GUI -> isready
    Engine -> readyok
    GUI -> ucinewgame
*/
// main UCI loop
void uci_loop() {
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
  printf("id name Maharajah\n");
  printf("id name Villi\n");
  printf("uciok\n");

  // main loop
  while(1) {
    // reset user /GUI input
    memset(input, 0, sizeof(input));

    // make sure output reaches the GUI
    fflush(stdout);

    // get user / GUI input
    if(!fgets(input, 2000, stdin))
      // continue the loop
      continue;

    // make sure input is available
    if(input[0] == '\n')
      // continue the loop
      continue;

    // parse UCI "isready" command
    if(strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
      continue;
    }

    // parse UCI "position" command
    else if(strncmp(input, "position", 8) == 0)
      // call parse position function
      parse_position(input);

    // parse UCI "ucinewgame" command
    else if(strncmp(input, "ucinewgame", 10) == 0)
      // call parse position function
      parse_position("position startpos");

    // parse UCI "go" command
    else if(strncmp(input, "go", 2) == 0)
      // call parse go function
      parse_go(input);

    // parse UCI "quit" command
    else if(strncmp(input, "quit", 4) == 0)
      // quit from the chess engine program execution
      break;

    // parse UCI "uci" command
    else if(strncmp(input, "uci", 3) == 0) {
      // print engine info
      printf("id name Maharajah\n");
      printf("id name Villi\n");
      printf("uciok\n");
    }
  }
}
