#include <stdio.h>
#include <string.h>

#include "maharajah/engine/Attacks.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/MagicNumbers.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/util/Defines.h"

uint get_random_U32_number() {
  uint number = random_state;
  number ^= number << 0xD;
  number ^= number >> 0x11;
  number ^= number << 0x5;
  random_state = number;
  return number;
}

u64 get_random_u64_number() {
  u64 n1 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n2 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n3 = (u64)(get_random_U32_number()) & 0xFFFF;
  u64 n4 = (u64)(get_random_U32_number()) & 0xFFFF;
  return n1 | (n2 << 0x10) | (n3 << 0x20) | (n4 << 0x30);
}

u64 generate_magic_number() {
  return get_random_u64_number() & get_random_u64_number() & get_random_u64_number();
}

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

u64 find_magic_number(const int square, const int relevant_bits, const int bishop) {
  u64 occupancies_[0x1000];
  u64 attacks[0x1000];
  u64 used_attacks[0x1000];
  u64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
  const int occupancy_indices = 1 << relevant_bits;

  for (int index = 0; index < occupancy_indices; ++index) {
    occupancies_[index] = set_occupancy(index, relevant_bits, attack_mask);
    attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies_[index]) : rook_attacks_on_the_fly(square, occupancies_[index]);
  }

  for (int random_count = 0; random_count < 100000000; ++random_count) {
    u64 magic_number = generate_magic_number();
    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
      continue;
    memset(used_attacks, 0ULL, sizeof(used_attacks));
    int index, fail;
    for (index = 0, fail = 0; !fail && index < occupancy_indices; ++index) {
      int magic_index = (int)((occupancies_[index] * magic_number) >> (0x40 - relevant_bits));
      if (used_attacks[magic_index] == 0ULL)
        used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index])
        fail = 1;
    }

    if (!fail)
      return magic_number;
  }
  printf("  Magic number fails!\n");
  
  return 0ULL;
}

#if 0
void init_magic_numbers() {
  for (int square = 0; square < 0x40; ++square)
    rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
  for (int square = 0; square < 0x40; ++square)
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}
#endif
