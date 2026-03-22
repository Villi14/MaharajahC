#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "../headers/Moves.h"
#include "../headers/Globals.h"
#include "../headers/Perft.h"

int get_time_ms() {
#ifdef _MSC_VER
  return GetTickCount();
#else
  struct timeval time_value;
  gettimeofday(&time_value, nullptr);
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

void perft_test(int depth) {
  printf("\n     Performance test\n\n");

  moves move_list[1];
  generate_moves(move_list);
  long start_time = get_time_ms();

  for (int move_count = 0; move_count < move_list->count; ++move_count) {
    copy_board();

    if (!make_move(move_list->moves[move_count], all_moves))
      continue;

    long cumulative_nodes = nodes;
    perft_driver(depth - 1);
    long old_nodes = nodes - cumulative_nodes;
    take_back();

    printf("     move: %s%s%c  nodes: %ld\n",
           square_to_coordinates[get_move_source(move_list->moves[move_count])],
           square_to_coordinates[get_move_target(move_list->moves[move_count])],
           get_move_promoted(move_list->moves[move_count]) ? promoted_pieces[get_move_promoted(move_list->moves[move_count])] : ' ',
           old_nodes);
  }

  printf("\n      Depth: %d\n", depth);
  printf("      Nodes: %ld\n", nodes);
  printf("       Time: %ld\n\n", get_time_ms() - start_time);
}
