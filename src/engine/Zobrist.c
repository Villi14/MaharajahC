#include "Zobrist.h"
#include "Globals.h"
#include "Defines.h"
#include "FindMagics.h"
#include "Moves.h"

ZobristKeys zobrist_keys;

// init random hash keys
void init_random_keys() {
  // update pseudo random number state
  random_state = 1804289383;

  // loop over piece codes
  for (int piece = P; piece <= k; ++piece) {
    // loop over board squares
    for (int square = 0; square < 64; ++square)
      // init random piece keys
      zobrist_keys.piece_keys[piece][square] = get_random_U64_number();
  }

  // loop over board squares
  for (int square = 0; square < 64; ++square)
    // init random board.enpassant keys
    zobrist_keys.enpassant_keys[square] = get_random_U64_number();

  // loop over castling keys
  for (int index = 0; index < 16; ++index)
    // init castling keys
    zobrist_keys.castle_keys[index] = get_random_U64_number();

  // init random board.side key
  zobrist_keys.side_key = get_random_U64_number();
}

// generate "almost" unique position ID aka hash key from scratch
U64 generate_hash_key() {
  // final hash key
  U64 final_key = 0ULL;

  // temp piece bitboard copy
  U64 bitboard;

  // loop over piece board.bitboards
  for (int piece = P; piece <= k; ++piece) {
    // init piece bitboard copy
    bitboard = board.bitboards[piece];

    // loop over the pieces within a bitboard
    while (bitboard) {
      // init square occupied by the piece
      int square = get_ls1b_index(bitboard);

      // hash piece
      final_key ^= zobrist_keys.piece_keys[piece][square];

      // pop LS1B
      pop_bit(bitboard, square);
    }
  }

  // if board.enpassant square is on board
  if (board.enpassant != no_sq)
    // hash board.enpassant
    final_key ^= zobrist_keys.enpassant_keys[board.enpassant];

  // hash castling rights
  final_key ^= zobrist_keys.castle_keys[board.castle];

  // hash the board.side only if black is to move
  if (board.side == black)
    final_key ^= zobrist_keys.side_key;

  // return generated hash key
  return final_key;
}
