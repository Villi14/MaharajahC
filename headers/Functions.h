#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <assert.h>
#ifdef _MSC_VER
#  include <intrin.h>
#endif
#include "Defines.h"

void generate_moves(moves* move_list);
int make_move(int move, int move_flag);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

static inline int count_bits(U64 bitboard) {
#if defined(_MSC_VER)
  return __popcnt64(bitboard);
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
  if(bitboard) {
    return count_bits((bitboard & -bitboard) - 1);
  } else
    return -1;
#endif
}

static inline void add_move(moves* move_list, int move) {
  move_list->moves[move_list->count] = move;
  ++move_list->count;
}

#endif // !FUNCTIONS_H_
