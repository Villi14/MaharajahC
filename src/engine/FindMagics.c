#include <stdio.h>
#include <string.h>

#include "Attacks.h"
#include "Defines.h"
#include "Moves.h"
#include "Globals.h"
#include "MagicNumbers.h"

// generate 32-bit pseudo legal numbers
uint get_random_U32_number() {
  // get current state
  uint number = random_state;

  // XOR shift algorithm
  number ^= number << 0xD;
  number ^= number >> 0x11;
  number ^= number << 0x5;

  // update random number state
  random_state = number;

  // return random number
  return number;
}

// generate 64-bit pseudo legal numbers
u64 get_random_u64_number() {
  // define 4 random numbers

  // init random numbers slicing 16 bits from MS1B board.side
  u64 n1 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n2 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n3 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n4 = (u64)(get_random_U32_number()) & 0xFFFF;

  // return random number
  return n1 | (n2 << 0x10) | (n3 << 0x20) | (n4 << 0x30);
}

// generate magic number candidate
u64 generate_magic_number() {
  return get_random_u64_number() & get_random_u64_number() & get_random_u64_number();
}

// set board.occupancies
u64 set_occupancy(int index, int bits_in_mask, u64 attack_mask) {
  u64 occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; ++count) {
    int square = get_ls1b_index(attack_mask);
    pop_bit(attack_mask, square);

    if (index & (1 << count))
      occupancy |= (1ULL << square);
  }

  return occupancy;
}

// find appropriate magic number
u64 find_magic_number(const int square, const int relevant_bits, const int bishop) {
  // init board.occupancies
  u64 occupancies_[0x1000];

  // init attack tables
  u64 attacks[0x1000];

  // init used attacks
  u64 used_attacks[0x1000];

  // init attack mask for a current piece
  u64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  // init occupancy indices
  const int occupancy_indices = 1 << relevant_bits;

  // loop over occupancy indices
  for (int index = 0; index < occupancy_indices; ++index) {
    // init board.occupancies
    occupancies_[index] = set_occupancy(index, relevant_bits, attack_mask);

    // init attacks
    attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies_[index]) : rook_attacks_on_the_fly(square, occupancies_[index]);
  }

  // test magic numbers loop
  for (int random_count = 0; random_count < 100000000; ++random_count) {
    // generate magic number candidate
    u64 magic_number = generate_magic_number();

    // skip inappropriate magic numbers
    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
      continue;

    // init used attacks
    memset(used_attacks, 0ULL, sizeof(used_attacks));

    // init index & fail flag
    int index, fail;

    // test magic index loop
    for (index = 0, fail = 0; !fail && index < occupancy_indices; ++index) {
      // init magic index
      int magic_index = (int)((occupancies_[index] * magic_number) >> (64 - relevant_bits));

      // if magic index works
      if (used_attacks[magic_index] == 0ULL)
        // init used attacks
        used_attacks[magic_index] = attacks[index];

      // otherwise
      else if (used_attacks[magic_index] != attacks[index])
        // magic index doesn't work
        fail = 1;
    }

    // if magic number works
    if (!fail)
      // return it
      return magic_number;
  }

  // if magic number doesn't work
  printf("  Magic number fails!\n");
  return 0ULL;
}

// init magic numbers
void init_magic_numbers() {
  // loop over 64 board squares
  for (int square = 0; square < 0x40; ++square)
    // init rook magic numbers
    rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);

  // loop over 64 board squares
  for (int square = 0; square < 0x40; ++square)
    // init bishop magic numbers
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}
