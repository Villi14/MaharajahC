#ifndef FIND_MAGIC_NUMBERS_H_
#define FIND_MAGIC_NUMBERS_H_

#include "maharajah/util/Defines.h"

void init_magic_numbers(void);
unsigned int get_random_U32_number(void);
u64 get_random_u64_number(void);
u64 generate_magic_number(void);
u64 find_magic_number(int square, int relevant_bits, int bishop);
u64 set_occupancy(int index, int bits_in_mask, u64 attack_mask);

#endif // FIND_MAGIC_NUMBERS_H_
