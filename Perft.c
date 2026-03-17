#include <string.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Functions.h"
#include "Globals.h"

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
  if (depth == 0) {
    ++nodes;
    return;
  }

  moves move_list[1];
  generate_moves(move_list);

  for (int move_count = 0; move_count < move_list->count; ++move_count) {
    copy_board();

    if (!make_move(move_list->moves[move_count], all_moves))
      continue;

    perft_driver(depth - 1);

    take_back();
  }
}
