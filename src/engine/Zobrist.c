#include "maharajah/engine/Zobrist.h"
#include "maharajah/engine/FindMagics.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/Moves.h"
#include "maharajah/util/Defines.h"

ZobristKeys zobrist_keys;

void init_random_keys(void) {
  random_state = 1804289383;
  for (int piece = P; piece <= k; ++piece) {
    for (int square = 0; square < 64; ++square)
      zobrist_keys.piece_keys[piece][square] = get_random_u64_number();
  }

  for (int square = 0; square < 64; ++square)
    zobrist_keys.enpassant_keys[square] = get_random_u64_number();

  for (int index = 0; index < 16; ++index)
    zobrist_keys.castle_keys[index] = get_random_u64_number();

  zobrist_keys.sidekey = get_random_u64_number();
}

u64 generate_hash_key(void) {
  u64 final_key = 0ULL;
  u64 bitboard;

  for (int piece = P; piece <= k; ++piece) {
    bitboard = board.bitboards[piece];
    while (bitboard) {
      int square = get_ls1b_index(bitboard);
      final_key ^= zobrist_keys.piece_keys[piece][square];
      pop_bit(bitboard, square);
    }
  }

  if (board.enpassant != no_sq)
    final_key ^= zobrist_keys.enpassant_keys[board.enpassant];

  final_key ^= zobrist_keys.castle_keys[board.castle];

  if (board.side == black)
    final_key ^= zobrist_keys.sidekey;

  return final_key;
}
