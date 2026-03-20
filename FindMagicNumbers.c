#include <stdio.h>
#include <string.h>

#include "Attacks.h"
#include "FindMagicNumbers.h"
#include "Functions.h"
#include "Globals.h"
#include "MagicNumbers.h"

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number() {
  // get current state
  unsigned int number = random_state;

  // XOR shift algorithm
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  // update random number state
  random_state = number;

  // return random number
  return number;
}

// generate 64-bit pseudo legal numbers
U64 get_random_U64_number() {
  // define 4 random numbers

  // init random numbers slicing 16 bits from MS1B side
  U64 n1 = (U64)(get_random_U32_number()) & 0xFFFF;
  U64 n2 = (U64)(get_random_U32_number()) & 0xFFFF;
  U64 n3 = (U64)(get_random_U32_number()) & 0xFFFF;
  U64 n4 = (U64)(get_random_U32_number()) & 0xFFFF;

  // return random number
  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
U64 generate_magic_number() {
  return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

// find appropriate magic number
U64 find_magic_number(int square, int relevant_bits, int bishop) {
  // init occupancies
  U64 occupancies_[0x1000];

  // init attack tables
  U64 attacks[0x1000];

  // init used attacks
  U64 used_attacks[0x1000];

  // init attack mask for a current piece
  U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  // init occupancy indices
  const int occupancy_indices = 1 << relevant_bits;

  // loop over occupancy indices
  for(int index = 0; index < occupancy_indices; ++index) {
    // init occupancies
    occupancies_[index] = set_occupancy(index, relevant_bits, attack_mask);

    // init attacks
    attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies_[index]) : rook_attacks_on_the_fly(square, occupancies_[index]);
  }

  // test magic numbers loop
  for(int random_count = 0; random_count < 100000000; ++random_count) {
    // generate magic number candidate
    U64 magic_number = generate_magic_number();

    // skip inappropriate magic numbers
    if(count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
      continue;

    // init used attacks
    memset(used_attacks, 0ULL, sizeof(used_attacks));

    // init index & fail flag
    int index, fail;

    // test magic index loop
    for(index = 0, fail = 0; !fail && index < occupancy_indices; ++index) {
      // init magic index
      int magic_index = (int)((occupancies_[index] * magic_number) >> (64 - relevant_bits));

      // if magic index works
      if(used_attacks[magic_index] == 0ULL)
        // init used attacks
        used_attacks[magic_index] = attacks[index];

      // otherwise
      else if(used_attacks[magic_index] != attacks[index])
        // magic index doesn't work
        fail = 1;
    }

    // if magic number works
    if(!fail)
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
  for(int square = 0; square < 0x40; ++square)
    // init rook magic numbers
    rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);

  // loop over 64 board squares
  for(int square = 0; square < 0x40; ++square)
    // init bishop magic numbers
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}