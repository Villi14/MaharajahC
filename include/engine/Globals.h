#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "../../include/util/Defines.h"

// clang-format off

enum { P, N, B, R, Q, A, C, M, K, p, n, b, r, q, a, c, m, k };

enum { white, black, both };

enum { rook, bishop };

enum { wk = 0x1, wq = 0x2, bk = 0x4, bq = 0x8 };

enum { opening, endgame, middlegame };

enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ARCHBISHOP, CHANCELLOR, AMAZON };

enum {
  piece_count = 18,
  white_piece_count = 9,
  piece_kind_count = 9
};

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

enum
{
  all_moves,
  only_captures
};

enum
{
  infinity = 50000,
  mate_value = 49000,
  mate_score = 48000,
  max_ply = 0x40,
  max_moves = 0x100,
};

enum
{
  double_pawn_penalty_opening = -5,
  double_pawn_penalty_endgame = -10,
  isolated_pawn_penalty_opening = -5,
  isolated_pawn_penalty_endgame = -10,
  semi_open_file_score = 10,
  open_file_score = 15,
  bishop_unit = 4,
  queen_unit = 9,
  bishop_mobility_opening = 5,
  bishop_mobility_endgame = 5,
  queen_mobility_opening = 1,
  queen_mobility_endgame = 2,
  reduction_limit = 3,
  king_shield_bonus = 5,
  opening_phase_score = 6192,
  endgame_phase_score = 518
};

static const u64 not_a_file = 0xFEFEFEFEFEFEFEFEULL;

static const u64 not_h_file = 0x7F7F7F7F7F7F7F7FULL;

static const u64 not_hg_file = 0x3F3F3F3F3F3F3F3FULL;

static const u64 not_ab_file = 0xFCFCFCFCFCFCFCFCULL;

static const int full_depth_moves = 4;

extern const int passed_pawn_bonus[8];

extern const char *square_to_coordinates[0x40];

extern const char *default_pieces[piece_count];

extern const int char_pieces[0x80];

extern const char promoted_pieces[0x80];

extern const int bishop_relevant_bits[0x40];

extern const int rook_relevant_bits[0x40];

extern const int castling_rights[0x40];

extern const int material_score[2][piece_count];

extern const int pawn_score[0x40];

extern const int knight_score[0x40];

extern const int bishop_score[0x40];

extern const int rook_score[0x40];

extern const int king_score[0x40];

extern const int mirror_score[0x80];

extern int mvv_lva[piece_count][piece_count];

extern const int positional_score[2][6][0x40];

extern unsigned int random_state;

typedef struct
{
  int moves[0x100];
  int count;
} MoveList;

typedef struct
{
  u64 bitboards[piece_count];
  u64 occupancies[3];
  int side;
  int enpassant;
  int castle;
  int halfmove_clock;
  int standard_rules;
  // Per-side rules in a mixed game: side_variant[white]/side_variant[black].
  // 1 = that side plays variant rules (compound promotion A/C/M, pawn
  // double-step from any rank); 0 = classic (Q/R/B/N, home-rank double-step
  // only). Carried by the optional 7th FEN field (see parse_fen). board-wide
  // standard_rules still governs the draw clocks.
  int side_variant[2];
  // Squares currently holding a pawn that has never moved since game start,
  // used ONLY as the pawn double-step eligibility check when has_pawn_state is
  // set (see parse_fen / the optional 8th FEN field). Rank-based home-row
  // detection can't tell a virgin custom-army pawn (started off rank 2/7)
  // apart from one that already used its lifetime double-step and happens to
  // still sit off rank 2/7 too — this bitboard carries the real per-piece
  // "has moved" fact the engine has no other way to know from a bare FEN.
  // Populated from parse_fen and maintained by make_move (source and target
  // bits are cleared; take_back restores the previous value), so it stays
  // exact at every search depth and across mah_apply_move.
  u64 pawn_unmoved;
  int has_pawn_state;
  u64 hash_key;
} Board;

extern Board board;

typedef struct
{
  u64 pawn_attacks[2][0x40];
  u64 knight_attacks[0x40];
  u64 king_attacks[0x40];
  u64 bishop_masks[0x40];
  u64 rook_masks[0x40];
  u64 bishop_attacks[0x40][0x200];
  u64 rook_attacks[0x40][0x1000];
} AttackTables;

extern AttackTables attack_tables;

typedef struct
{
  u64 nodes;
  int ply;
  int best_move;
  int follow_pv;
  int score_pv;
  int repetition_index;
  int killer_moves[2][max_ply];
  int history_moves[piece_count][0x40];
  int pv_length[max_ply];
  int pv_table[max_ply][max_ply];
  int root_count;
  int root_moves[max_moves];
  int root_scores[max_moves];
  u64 repetition_table[10000];
} SearchContext;

extern SearchContext search_context;

typedef struct
{
  int quit;
  int movestogo;
  int movetime;
  int uci_time;
  int inc;
  int starttime;
  int stoptime;
  int timeset;
  int stopped;
  int stdin_polling_enabled;
} TimeControls;

extern TimeControls time_controls;

typedef struct
{
  int double_pawn_penalty_opening;
  int double_pawn_penalty_endgame;
  int isolated_pawn_penalty_opening;
  int isolated_pawn_penalty_endgame;
  int semi_open_file_score;
  int open_file_score;
  int bishop_unit;
  int queen_unit;
  int bishop_mobility_opening;
  int bishop_mobility_endgame;
  int queen_mobility_opening;
  int queen_mobility_endgame;
  int reduction_limit;
  int king_shield_bonus;
  int opening_phase_score;
  int endgame_phase_score;
  const int *passed_pawn_bonus;
  const int *pawn_score;
  const int *knight_score;
  const int *bishop_score;
  const int *rook_score;
  const int *king_score;
  const int *mirror_score;
  int (*mvv_lva)[piece_count];
  const int (*material_score)[piece_count];
  const int (*positional_score)[6][0x40];
} EvalTables;

extern const EvalTables eval_tables;

void init_mvv_lva(void);
int board_has_compound_pieces(void);

#endif // GLOBALS_H_
