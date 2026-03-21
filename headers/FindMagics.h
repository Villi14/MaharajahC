#ifndef FIND_MAGIC_NUMBERS_H_
#define FIND_MAGIC_NUMBERS_H_

#include "Defines.h"

void init_magic_numbers();
unsigned int get_random_U32_number();
U64 get_random_U64_number();
U64 generate_magic_number();
U64 find_magic_number(int square, int relevant_bits, int bishop);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

#endif // !FIND_MAGIC_NUMBERS_H_
