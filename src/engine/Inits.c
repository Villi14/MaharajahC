#include "maharajah/engine/Inits.h"
#include "maharajah/engine/Attacks.h"
#include "maharajah/engine/FindMagics.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/engine/MagicNumbers.h"
#include "maharajah/engine/Masks.h"
#include "maharajah/engine/Transposition.h"
#include "maharajah/engine/Zobrist.h"
#include "maharajah/util/Utils.h"

void init_sliders_attacks(int bishop) {
  for (int square = 0; square < 64; ++square) {
    attack_tables.bishop_masks[square] = mask_bishop_attacks(square);
    attack_tables.rook_masks[square] = mask_rook_attacks(square);
    u64 attack_mask = bishop ? attack_tables.bishop_masks[square] : attack_tables.rook_masks[square];
    int relevant_bits_count = count_bits(attack_mask);
    int occupancy_indices = (1 << relevant_bits_count);

    for (int index = 0; index < occupancy_indices; ++index) {
      if (bishop) {
        u64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
        u64 magic_index = (occupancy * bishop_magic_numbers[square]) >> (0x40 - bishop_relevant_bits[square]);
        attack_tables.bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
      } else {
        u64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
        u64 magic_index = (occupancy * rook_magic_numbers[square]) >> (0x40 - rook_relevant_bits[square]);
        attack_tables.rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

static void init_leapers_attacks(void) {
  for (int square = 0; square < 64; ++square) {
    attack_tables.pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    attack_tables.pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    attack_tables.knight_attacks[square] = mask_knight_attacks(square);
    attack_tables.king_attacks[square] = mask_king_attacks(square);
  }
}

void init_all(void) {
  init_leapers_attacks();
  init_sliders_attacks(bishop);
  init_sliders_attacks(rook);
  init_random_keys();
  init_evaluation_masks();
  init_hash_table(0x40);
}
