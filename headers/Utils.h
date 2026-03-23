#ifndef UTILS_H_
#define UTILS_H_

#include <assert.h>

#include "Defines.h"
#include "Globals.h"

// count bits within a bitboard

static inline int count_bits(U64 bitboard) {
#if defined(_MSC_VER)
  return (int)__popcnt64(bitboard);
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_popcountll(bitboard);
#else
  // fallback
  // count bits within a bitboard (Brian Kernighan's way)
  int count = 0;
  while (bitboard) {
    bitboard &= bitboard - 1;
    ++count;
  }
  return count;
#endif
}

// get least significant 1st bit index
static inline int get_ls1b_index(U64 bitboard) {
  assert(bitboard);

#if defined(_MSC_VER)
  unsigned long index;
  _BitScanForward64(&index, bitboard);
  return index;
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_ctzll(bitboard);
#else
  // fallback
  // get least significant 1st bit index
  return count_bits((bitboard & -bitboard) - 1);
#endif
}

static inline void reset_time_control() {
  // reset timing
  quit = 0;
  movestogo = 30;
  movetime = -1;
  uci_time = -1;
  inc = 0;
  starttime = 0;
  stoptime = 0;
  timeset = 0;
  stopped = 0;
}

#endif // !UTILS_H_
