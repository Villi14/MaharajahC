#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "../../include/engine/Globals.h"
#include "../../include/engine/Moves.h"
#include "../../include/perft/Perft.h"

// Milliseconds relative to the first call. Truncating absolute epoch
// milliseconds to int used to flip sign every ~25 days of wall-clock time,
// silently breaking the absolute stoptime comparisons in communicate();
// process-relative time keeps int comparisons valid for ~24 days of uptime.
int get_time_ms(void) {
#ifdef _MSC_VER
  static ULONGLONG base_ms;
  const ULONGLONG now_ms = GetTickCount64();
  if (base_ms == 0)
    base_ms = now_ms;
  return (int)(now_ms - base_ms);
#else
  static long long base_ms;
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  const long long now_ms =
      (long long)time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
  if (base_ms == 0)
    base_ms = now_ms;
  return (int)(now_ms - base_ms);
#endif
}

void perft_driver(int depth) {
  if (depth == 0) {
    ++search_context.nodes;
    return;
  }

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);

  for (int move_count = 0; move_count < move_list.count; ++move_count) {
    copy_board();

    if (!make_move(move_list.moves[move_count], all_moves))
      continue;

    perft_driver(depth - 1);

    take_back();
  }
}

void perft_test(int depth) {
  printf("\n     Performance test\n\n");

  MoveList move_list = { .count = 0 };
  generate_moves(&move_list);
  long start_time = get_time_ms();

  for (int move_count = 0; move_count < move_list.count; ++move_count) {
    copy_board();

    if (!make_move(move_list.moves[move_count], all_moves))
      continue;

    u64 cumulative_nodes = search_context.nodes;
    perft_driver(depth - 1);
    u64 old_nodes = search_context.nodes - cumulative_nodes;
    take_back();

    printf("     move: %s%s%c  search_context.nodes: %lld\n",
           square_to_coordinates[get_move_source(move_list.moves[move_count])],
           square_to_coordinates[get_move_target(move_list.moves[move_count])],
           get_move_promoted(move_list.moves[move_count]) ? promoted_pieces[get_move_promoted(move_list.moves[move_count])] : ' ',
           old_nodes);
  }

  printf("\n      Depth: %d\n", depth);
  printf("      Nodes: %lld\n", search_context.nodes);
  printf("       Time: %ld\n\n", get_time_ms() - start_time);
}
