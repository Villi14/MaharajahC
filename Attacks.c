#include "Attacks.h"
#include "Functions.h"
#include "Globals.h"
#include "MagicNumbers.h"

// generate pawn attacks
U64 mask_pawn_attacks(int side_, int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if(!side_) {
    if((bitboard >> 7) & not_a_file)
      attacks |= (bitboard >> 7);
    if((bitboard >> 9) & not_h_file)
      attacks |= (bitboard >> 9);
  } else {
    if((bitboard << 7) & not_h_file)
      attacks |= (bitboard << 7);
    if((bitboard << 9) & not_a_file)
      attacks |= (bitboard << 9);
  }

  return attacks;
}

// generate knight attacks
U64 mask_knight_attacks(int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if((bitboard >> 17) & not_h_file)
    attacks |= (bitboard >> 17);
  if((bitboard >> 15) & not_a_file)
    attacks |= (bitboard >> 15);
  if((bitboard >> 10) & not_hg_file)
    attacks |= (bitboard >> 10);
  if((bitboard >> 6) & not_ab_file)
    attacks |= (bitboard >> 6);
  if((bitboard << 17) & not_a_file)
    attacks |= (bitboard << 17);
  if((bitboard << 15) & not_h_file)
    attacks |= (bitboard << 15);
  if((bitboard << 10) & not_ab_file)
    attacks |= (bitboard << 10);
  if((bitboard << 6) & not_hg_file)
    attacks |= (bitboard << 6);

  return attacks;
}

// generate king attacks
U64 mask_king_attacks(int square) {
  U64 attacks = 0ULL;
  U64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if(bitboard >> 8)
    attacks |= (bitboard >> 8);
  if((bitboard >> 9) & not_h_file)
    attacks |= (bitboard >> 9);
  if((bitboard >> 7) & not_a_file)
    attacks |= (bitboard >> 7);
  if((bitboard >> 1) & not_h_file)
    attacks |= (bitboard >> 1);
  if(bitboard << 8)
    attacks |= (bitboard << 8);
  if((bitboard << 9) & not_a_file)
    attacks |= (bitboard << 9);
  if((bitboard << 7) & not_h_file)
    attacks |= (bitboard << 7);
  if((bitboard << 1) & not_a_file)
    attacks |= (bitboard << 1);

  return attacks;
}

// mask bishop attacks
U64 mask_bishop_attacks(int square) {
  U64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for(r = tr + 1, f = tf + 1; r <= 6 && f <= 6; ++r, ++f)
    attacks |= (1ULL << (r * 8 + f));
  for(r = tr - 1, f = tf + 1; r >= 1 && f <= 6; --r, ++f)
    attacks |= (1ULL << (r * 8 + f));
  for(r = tr + 1, f = tf - 1; r <= 6 && f >= 1; ++r, --f)
    attacks |= (1ULL << (r * 8 + f));
  for(r = tr - 1, f = tf - 1; r >= 1 && f >= 1; --r, --f)
    attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

// mask rook attacks
U64 mask_rook_attacks(int square) {
  U64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for(r = tr + 1; r <= 6; ++r)
    attacks |= (1ULL << (r * 8 + tf));
  for(r = tr - 1; r >= 1; --r)
    attacks |= (1ULL << (r * 8 + tf));
  for(f = tf + 1; f <= 6; ++f)
    attacks |= (1ULL << (tr * 8 + f));
  for(f = tf - 1; f >= 1; --f)
    attacks |= (1ULL << (tr * 8 + f));

  return attacks;
}

// generate bishop attacks on the fly
U64 bishop_attacks_on_the_fly(int square, U64 block) {
  U64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for(r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    attacks |= (1ULL << (r * 8 + f));
    if((1ULL << (r * 8 + f)) & block)
      break;
  }

  for(r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    attacks |= (1ULL << (r * 8 + f));
    if((1ULL << (r * 8 + f)) & block)
      break;
  }

  for(r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    attacks |= (1ULL << (r * 8 + f));
    if((1ULL << (r * 8 + f)) & block)
      break;
  }

  for(r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    attacks |= (1ULL << (r * 8 + f));
    if((1ULL << (r * 8 + f)) & block)
      break;
  }

  return attacks;
}

// generate rook attacks on the fly
U64 rook_attacks_on_the_fly(int square, U64 block) {
  U64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for(r = tr + 1; r <= 7; ++r) {
    attacks |= (1ULL << (r * 8 + tf));
    if((1ULL << (r * 8 + tf)) & block)
      break;
  }

  for(r = tr - 1; r >= 0; --r) {
    attacks |= (1ULL << (r * 8 + tf));
    if((1ULL << (r * 8 + tf)) & block)
      break;
  }

  for(f = tf + 1; f <= 7; ++f) {
    attacks |= (1ULL << (tr * 8 + f));
    if((1ULL << (tr * 8 + f)) & block)
      break;
  }

  for(f = tf - 1; f >= 0; --f) {
    attacks |= (1ULL << (tr * 8 + f));
    if((1ULL << (tr * 8 + f)) & block)
      break;
  }

  return attacks;
}

// get bishop attacks
U64 get_bishop_attacks(int square, U64 occupancy) {
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_bits[square];

  return bishop_attacks[square][occupancy];
}

// get rook attacks
U64 get_rook_attacks(int square, U64 occupancy) {
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_bits[square];

  return rook_attacks[square][occupancy];
}

// get queen attacks
U64 get_queen_attacks(int square, U64 occupancy) {
  U64 queen_attacks = 0ULL;
  U64 bishop_occupancy = occupancy;
  U64 rook_occupancy = occupancy;

  bishop_occupancy &= bishop_masks[square];
  bishop_occupancy *= bishop_magic_numbers[square];
  bishop_occupancy >>= 64 - bishop_relevant_bits[square];

  queen_attacks = bishop_attacks[square][bishop_occupancy];

  rook_occupancy &= rook_masks[square];
  rook_occupancy *= rook_magic_numbers[square];
  rook_occupancy >>= 64 - rook_relevant_bits[square];

  queen_attacks |= rook_attacks[square][rook_occupancy];

  return queen_attacks;
}

// is square current given attacked by the current given side
int is_square_attacked(int square, int side_) {
  if((side_ == white) && (pawn_attacks[black][square] & bitboards[P]))
    return 1;

  if((side_ == black) && (pawn_attacks[white][square] & bitboards[p]))
    return 1;

  if(knight_attacks[square] & ((side_ == white) ? bitboards[N] : bitboards[n]))
    return 1;

  if(get_bishop_attacks(square, occupancies[both]) & ((side_ == white) ? bitboards[B] : bitboards[b]))
    return 1;

  if(get_rook_attacks(square, occupancies[both]) & ((side_ == white) ? bitboards[R] : bitboards[r]))
    return 1;

  if(get_queen_attacks(square, occupancies[both]) & ((side_ == white) ? bitboards[Q] : bitboards[q]))
    return 1;

  if(king_attacks[square] & ((side_ == white) ? bitboards[K] : bitboards[k]))
    return 1;

  return 0;
}
