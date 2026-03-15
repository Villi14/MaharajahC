#include <string.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Functions.h"
#include "Globals.h"
#include "Prints.h"
#include "Constants.h"

int get_time_ms() {
#ifdef _MSC_VER
  return GetTickCount();
#else
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}

// perft driver
void perft_driver(int depth) {
  // recursion escape condition
  if (depth == 0) {
    // increment nodes count (count reached positions)
    nodes++;
    return;
  }

  // create move list instance
  moves move_list[1];

  // generate moves
  generate_moves(move_list);

  // loop over generated moves
  for (int move_count = 0; move_count < move_list->count; move_count++) {
    // preserve board state
    copy_board();

    // make move
    if (!make_move(move_list->moves[move_count], all_moves))
      // skip to the next move
      continue;

    // call perft driver recursively
    perft_driver(depth - 1);

    // take back
    take_back();
  }
}