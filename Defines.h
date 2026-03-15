#ifndef DEFINES_H_
#define DEFINES_H_

// define bitboard data type
#define U64 unsigned long long

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

// set/get/pop bit macros
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

 /*       binary move bits                               hexidecimal constants

    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

// encode move
#define encode_move(source, target, piece, promoted, capture, double_push, enpassant, castling) \
  (source) |          \
  (target << 6) |     \
  (piece << 12) |     \
  (promoted << 16) |  \
  (capture << 20) |   \
  (double_push << 21) |    \
  (enpassant << 22) | \
  (castling << 23)    \

// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) (move & 0x100000)

// extract double pawn push flag
#define get_move_double(move) (move & 0x200000)

// extract enpassant flag
#define get_move_enpassant(move) (move & 0x400000)

// extract castling flag
#define get_move_castling(move) (move & 0x800000)

// move list structure
typedef struct {
  // moves
  int moves[256];
  // move count
  int count;
} moves;

// preserve board state
#define copy_board()                                                    \
  U64 bitboards_copy[12], occupancies_copy[3];                          \
  int side_copy, enpassant_copy, castle_copy;                           \
  memcpy(bitboards_copy, bitboards, 96);                                \
  memcpy(occupancies_copy, occupancies, 24);                            \
  side_copy = side, enpassant_copy = enpassant, castle_copy = castle;   \

// restore board state
#define take_back()                                                     \
  memcpy(bitboards, bitboards_copy, 96);                                \
  memcpy(occupancies, occupancies_copy, 24);                            \
  side = side_copy, enpassant = enpassant_copy, castle = castle_copy;   \

  #endif // DEFINES_H_

