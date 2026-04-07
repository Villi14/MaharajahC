#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include "maharajah/util/Defines.h"

typedef struct {
  u64 piece_keys[12][0x40];
  u64 enpassant_keys[0x40];
  u64 castle_keys[16];
  u64 sidekey;
} ZobristKeys;

extern ZobristKeys zobrist_keys;

void init_random_keys(void);
u64 generate_hash_key(void);

#endif // ZOBRIST_H_
