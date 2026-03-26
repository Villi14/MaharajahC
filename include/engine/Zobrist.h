#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include "Defines.h"

typedef struct {
  u64 piece_keys[12][64];
  u64 enpassant_keys[64];
  u64 castle_keys[16];
  u64 side_key;
} ZobristKeys;

extern ZobristKeys zobrist_keys;

// init random hash keys
void init_random_keys();

// generate "almost" unique position ID aka hash key from scratch
u64 generate_hash_key();

#endif // !ZOBRIST_H_
