#ifndef ZOBRIST_H_
#define ZOBRIST_H_

#include "Defines.h"

typedef struct {
  U64 piece_keys[12][64];
  U64 enpassant_keys[64];
  U64 castle_keys[16];
  U64 side_key;
} ZobristKeys;

extern ZobristKeys zobrist_keys;

// init random hash keys
void init_random_keys();

// generate "almost" unique position ID aka hash key from scratch
U64 generate_hash_key();

#endif // !ZOBRIST_H_
