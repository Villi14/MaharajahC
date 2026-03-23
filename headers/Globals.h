#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Defines.h"

// encode pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// sides to move (colors)
enum { white, black, both };

// bishop and rook
enum { rook, bishop };

/*  bin   dec

   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side

   examples

   1111       both sides a castle both directions
   1001       black king => queen side
              white king => king side
*/
enum { wk = 0x1, wq = 0x2, bk = 0x4, bq = 0x8 };

// game phases
enum { opening, endgame, middlegame };

// piece types
enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

// clang-format off

// board squares
enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

// clang-format on

// move types
enum { all_moves, only_captures };

// not A file constant
static const U64 not_a_file = 0xFEFEFEFEFEFEFEFEULL;

// not H file constant
static const U64 not_h_file = 0x7F7F7F7F7F7F7F7FULL;

// not HG file constant
static const U64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL;

// not AB file constant
static const U64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL;

// piece bitboards
extern U64 bitboards[0xC];

// occupancy bitboards
extern U64 occupancies[3];

// side to move
extern int side;

// enpassant square
extern int enpassant;

// castling rights
extern int castle;

// pseudo random number state
extern unsigned int random_state;

// half move counter
extern int ply;

// follow PV & score PV move
extern int follow_pv;

extern int score_pv;

// "almost" unique position identifier aka hash key or position key
extern U64 hash_key;

// repetition index
extern int repetition_index;

// best move
extern int best_move;

// leaf nodes (number of positions reached during the test of the move generator at a given depth)
extern U64 nodes;

//time controls variables

// exit from engine flag
extern int quit;

// UCI "movestogo" command moves counter
extern int movestogo;

// UCI "movetime" command time counter
extern int movetime;

// UCI "time" command holder (ms)
extern int uci_time;

// UCI "inc" command's time increment holder
extern int inc;

// UCI "starttime" command time holder
extern int starttime;

// UCI "stoptime" command time holder
extern int stoptime;

// variable to flag time control availability
extern int timeset;

// variable to flag when the time is up
extern int stopped;

// full depth moves counter
extern const int full_depth_moves;

// double pawns penalty
extern const int double_pawn_penalty_opening;
extern const int double_pawn_penalty_endgame;

// isolated pawn penalty
extern const int isolated_pawn_penalty_opening;
extern const int isolated_pawn_penalty_endgame;

// passed pawn bonus
extern const int passed_pawn_bonus[8] ;

// semi open file score
extern const int semi_open_file_score;

// open file score
extern const int open_file_score;

// mobility units (values from engine Fruit reloaded)
extern const int bishop_unit;
extern  const int queen_unit;

// mobility bonuses (values from engine Fruit reloaded)
extern  const int bishop_mobility_opening;
extern  const int bishop_mobility_endgame;
extern  const int queen_mobility_opening;
extern  const int queen_mobility_endgame;

// depth limit to consider reduction
extern  const int reduction_limit;

// number hash table entries
extern int hash_entries;

// king's shield bonus
extern const int king_shield_bonus;

// game phase scores
extern const int opening_phase_score;
extern const int endgame_phase_score;

// pawn attacks table [side][square]
extern U64 pawn_attacks[2][0x40];

// knight attacks table [square]
extern U64 knight_attacks[0x40];

// king attacks table [square]
extern U64 king_attacks[0x40];

// bishop attack masks
extern U64 bishop_masks[0x40];

// rook attack masks
extern U64 rook_masks[0x40];

// bishop attacks table [square][occupancies]
extern U64 bishop_attacks[0x40][0x200];

// rook attacks table [square][occupancies]
extern U64 rook_attacks[0x40][0x1000];

// killer moves [id][ply]
extern int killer_moves[2][max_ply];

// history moves [piece][square]
extern int history_moves[12][0x40];

// convert squares to coordinates
extern const char* square_to_coordinates[0x40];

// default pieces
extern const char* default_pieces[0xC];

// convert ASCII character pieces to encoded constants
extern const int char_pieces[0x80];

// promoted pieces
extern const char promoted_pieces[0x80];

// bishop relevant occupancy bit count for every square on board
extern const int bishop_relevant_bits[0x40];

// rook relevant occupancy bit count for every square on board
extern const int rook_relevant_bits[0x40];

/*                         castling   move           in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13

         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7
*/

// castling rights update constants
extern const int castling_rights[0x40];

/*  ♙ =   100   = ♙
    ♘ =   300   = ♙ * 3
    ♗ =   350   = ♙ * 3 + ♙ * 0.5
    ♖ =   500   = ♙ * 5
    ♕ =   1000  = ♙ * 10
    ♔ =   10000 = ♙ * 100
*/
extern int material_score[2][0xC];

// pawn positional score
extern const int pawn_score[0x40];

// knight positional score
extern const int knight_score[0x40];

// bishop positional score
extern const int bishop_score[0x40];

// rook positional score
extern const int rook_score[0x40];

// king positional score
extern const int king_score[0x40];

// mirror positional score tables for opposite side
extern const int mirror_score[0x80];

// most valuable victim & less valuable attacker

/*  (Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
        Pawn   105    205    305    405    505    605
      Knight   104    204    304    404    504    604
      Bishop   103    203    303    403    503    603
        Rook   102    202    302    402    502    602
       Queen   101    201    301    401    501    601
        King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]

extern const int mvv_lva[0xC][0xC];

/*
      ================================
            Triangular PV table
      --------------------------------
        PV line: e2e4 e7e5 g1f3 b8c6
      ================================

           0    1    2    3    4    5

      0    m1   m2   m3   m4   m5   m6

      1    0    m2   m3   m4   m5   m6

      2    0    0    m3   m4   m5   m6

      3    0    0    0    m4   m5   m6

      4    0    0    0    0    m5   m6

      5    0    0    0    0    0    m6
*/

// PV length
extern int pv_length[max_ply];

// PV table
extern int pv_table[max_ply][max_ply];

// positions repetition table
extern U64 repetition_table[10000];  // 1000 is a number of plies (500 moves) in the entire game

extern const int positional_score[2][6][64];

#endif // !GLOBALS_H_
