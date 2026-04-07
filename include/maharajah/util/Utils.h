#ifndef UTILS_H_
#define UTILS_H_

#include <assert.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "maharajah/util/Defines.h"
#include "maharajah/engine/Globals.h"

static inline int count_bits(u64 bitboard) {
#if defined(_MSC_VER)
  return (int)__popcnt64(bitboard);
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_popcountll(bitboard);
#else
  int count = 0;
  while (bitboard) {
    bitboard &= bitboard - 1;
    ++count;
  }
  return count;
#endif
}

static inline int get_ls1b_index(u64 bitboard) {
  assert(bitboard);

#if defined(_MSC_VER)
  unsigned long index;
  _BitScanForward64(&index, bitboard);
  return index;
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_ctzll(bitboard);
#else
  return count_bits((bitboard & -bitboard) - 1);
#endif
}

static inline void reset_time_control(void) {
  time_controls.quit = 0;
  time_controls.movestogo = 30;
  time_controls.movetime = -1;
  time_controls.uci_time = -1;
  time_controls.inc = 0;
  time_controls.starttime = 0;
  time_controls.stoptime = 0;
  time_controls.timeset = 0;
  time_controls.stopped = 0;
  time_controls.stdin_polling_enabled = 1;
}

#endif // UTILS_H_
