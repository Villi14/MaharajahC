#include "../headers/Inits.h"
#include "../headers/Attacks.h"
#include "../headers/FindMagics.h"
#include "../headers/Globals.h"
#include "../headers/MagicNumbers.h"
#include "../headers/Masks.h"
#include "../headers/Zobrist.h"
#include "../headers/Utils.h"
#include "../headers/Transposition.h"


// init slider piece's attack tables
void init_sliders_attacks(int bishop) {
  // loop over 64 board squares
  for (int square = 0; square < 64; ++square) {
    // init bishop & rook masks
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);

    // init current mask
    U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

    // init relevant occupancy bit count
    int relevant_bits_count = count_bits(attack_mask);

    // init occupancy indices
    int occupancy_indices = (1 << relevant_bits_count);

    // loop over occupancy indices
    for (int index = 0; index < occupancy_indices; ++index) {
      // bishop
      if (bishop) {
        // init current occupancy variation
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        U64 magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

        // init bishop attacks
        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
      }

      // rook
      else {
        // init current occupancy variation
        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

        // init magic index
        U64 magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

        // init rook attacks
        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
      }
    }
  }
}

// init leaper pieces attacks
void init_leapers_attacks() {
  // loop over 64 board squares
  for (int square = 0; square < 64; ++square) {
    // init pawn attacks
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);

    // init knight attacks
    knight_attacks[square] = mask_knight_attacks(square);

    // init king attacks
    king_attacks[square] = mask_king_attacks(square);
  }
}

// init all variables
void init_all() {
  // init leaper pieces attacks
  init_leapers_attacks();

  // init slider pieces attacks
  init_sliders_attacks(bishop);
  init_sliders_attacks(rook);

  // init random keys for hashing purposes
  init_random_keys();

  // init evaluation masks
  init_evaluation_masks();

  // init hash table with default 64 MB
  init_hash_table(64);

  // init magic numbers
  // init_magic_numbers();
}
