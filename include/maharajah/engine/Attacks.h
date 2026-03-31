#ifndef ATTACKS_H_
#define ATTACKS_H_

#include "maharajah/util/Defines.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/MagicNumbers.h"

u64 mask_bishop_attacks(int square);
u64 mask_rook_attacks(int square);
u64 mask_pawn_attacks(int side_, int square);
u64 mask_knight_attacks(int square);
u64 mask_king_attacks(int square);
u64 bishop_attacks_on_the_fly(int square, u64 block);
u64 rook_attacks_on_the_fly(int square, u64 block);

static inline u64 get_bishop_attacks(int square, u64 occupancy) {
  occupancy &= attack_tables.bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 0x40 - bishop_relevant_bits[square];

  return attack_tables.bishop_attacks[square][occupancy];
}

static inline u64 get_rook_attacks(int square, u64 occupancy) {
  occupancy &= attack_tables.rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 0x40 - rook_relevant_bits[square];

  return attack_tables.rook_attacks[square][occupancy];
}

static inline u64 get_queen_attacks(const int square, u64 occupancy) {
  u64 queen_attacks = 0ULL;
  u64 bishop_occupancy = occupancy;
  u64 rook_occupancy = occupancy;

  bishop_occupancy &= attack_tables.bishop_masks[square];
  bishop_occupancy *= bishop_magic_numbers[square];
  bishop_occupancy >>= 0x40 - bishop_relevant_bits[square];

  queen_attacks = attack_tables.bishop_attacks[square][bishop_occupancy];

  rook_occupancy &= attack_tables.rook_masks[square];
  rook_occupancy *= rook_magic_numbers[square];
  rook_occupancy >>= 0x40 - rook_relevant_bits[square];

  queen_attacks |= attack_tables.rook_attacks[square][rook_occupancy];

  return queen_attacks;
}

// is square current given attacked by the current given board.side
static inline int is_square_attacked(int square, int side_) {
  if ((side_ == white) && (attack_tables.pawn_attacks[black][square] & board.bitboards[P]))
    return 1;

  if ((side_ == black) && (attack_tables.pawn_attacks[white][square] & board.bitboards[p]))
    return 1;

  if (attack_tables.knight_attacks[square] & ((side_ == white) ? board.bitboards[N] : board.bitboards[n]))
    return 1;

  if (get_bishop_attacks(square, board.occupancies[both]) & ((side_ == white) ? board.bitboards[B] : board.bitboards[b]))
    return 1;

  if (get_rook_attacks(square, board.occupancies[both]) & ((side_ == white) ? board.bitboards[R] : board.bitboards[r]))
    return 1;

  if (get_queen_attacks(square, board.occupancies[both]) & ((side_ == white) ? board.bitboards[Q] : board.bitboards[q]))
    return 1;

  if (attack_tables.king_attacks[square] & ((side_ == white) ? board.bitboards[K] : board.bitboards[k]))
    return 1;

  return 0;
}

#endif // !ATTACKS_H_
