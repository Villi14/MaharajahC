#include "maharajah/engine/Attacks.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/util/Defines.h"

u64 mask_pawn_attacks(int side_, int square) {
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if (!side_) {
    if ((bitboard >> 7) & not_a_file)
      attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & not_h_file)
      attacks |= (bitboard >> 9);
  } else {
    if ((bitboard << 7) & not_h_file)
      attacks |= (bitboard << 7);
    if ((bitboard << 9) & not_a_file)
      attacks |= (bitboard << 9);
  }

  return attacks;
}

u64 mask_knight_attacks(int square) {
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if ((bitboard >> 17) & not_h_file)
    attacks |= (bitboard >> 17);
  if ((bitboard >> 15) & not_a_file)
    attacks |= (bitboard >> 15);
  if ((bitboard >> 10) & not_hg_file)
    attacks |= (bitboard >> 10);
  if ((bitboard >> 6) & not_ab_file)
    attacks |= (bitboard >> 6);
  if ((bitboard << 17) & not_a_file)
    attacks |= (bitboard << 17);
  if ((bitboard << 15) & not_h_file)
    attacks |= (bitboard << 15);
  if ((bitboard << 10) & not_ab_file)
    attacks |= (bitboard << 10);
  if ((bitboard << 6) & not_hg_file)
    attacks |= (bitboard << 6);

  return attacks;
}

u64 mask_king_attacks(int square) {
  u64 attacks = 0ULL;
  u64 bitboard = 0ULL;

  set_bit(bitboard, square);

  if (bitboard >> 8)
    attacks |= (bitboard >> 8);
  if ((bitboard >> 9) & not_h_file)
    attacks |= (bitboard >> 9);
  if ((bitboard >> 7) & not_a_file)
    attacks |= (bitboard >> 7);
  if ((bitboard >> 1) & not_h_file)
    attacks |= (bitboard >> 1);
  if (bitboard << 8)
    attacks |= (bitboard << 8);
  if ((bitboard << 9) & not_a_file)
    attacks |= (bitboard << 9);
  if ((bitboard << 7) & not_h_file)
    attacks |= (bitboard << 7);
  if ((bitboard << 1) & not_a_file)
    attacks |= (bitboard << 1);

  return attacks;
}

u64 mask_bishop_attacks(int square) {
  u64 attacks = 0ULL;
  int r, f;
  int tr = square / 8;
  int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; ++r, ++f)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; --r, ++f)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; ++r, --f)
    attacks |= (1ULL << (r * 8 + f));
  for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; --r, --f)
    attacks |= (1ULL << (r * 8 + f));

  return attacks;
}

u64 mask_rook_attacks(int square) {
  u64 attacks = 0ULL;
  int r, f;
  const int tr = square / 8;
  const int tf = square % 8;

  for (r = tr + 1; r <= 6; ++r)
    attacks |= (1ULL << (r * 8 + tf));
  for (r = tr - 1; r >= 1; --r)
    attacks |= (1ULL << (r * 8 + tf));
  for (f = tf + 1; f <= 6; ++f)
    attacks |= (1ULL << (tr * 8 + f));
  for (f = tf - 1; f >= 1; --f)
    attacks |= (1ULL << (tr * 8 + f));

  return attacks;
}

u64 bishop_attacks_on_the_fly(int square, u64 block) {
  u64 attacks = 0ULL;
  int r, f;
  const int tr = square / 8;
  const int tf = square % 8;

  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    attacks |= (1ULL << (r * 8 + f));
    if ((1ULL << (r * 8 + f)) & block)
      break;
  }

  return attacks;
}

u64 rook_attacks_on_the_fly(int square, u64 block) {
  u64 attacks = 0ULL;
  int r, f;
  const int tr = square / 8;
  const int tf = square % 8;

  for (r = tr + 1; r <= 7; ++r) {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }

  for (r = tr - 1; r >= 0; --r) {
    attacks |= (1ULL << (r * 8 + tf));
    if ((1ULL << (r * 8 + tf)) & block)
      break;
  }

  for (f = tf + 1; f <= 7; ++f) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }

  for (f = tf - 1; f >= 0; --f) {
    attacks |= (1ULL << (tr * 8 + f));
    if ((1ULL << (tr * 8 + f)) & block)
      break;
  }

  return attacks;
}

int is_square_attacked(int square, int side_) {
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
