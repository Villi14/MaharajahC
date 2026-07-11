#include <stdlib.h>
#include <string.h>

#include "../../include/engine/Attacks.h"
#include "../../include/engine/Globals.h"
#include "../../include/engine/Moves.h"
#include "../../include/engine/See.h"
#include "../../include/util/Defines.h"
#include "../../include/util/Utils.h"

static int material_cp(int piece) { return abs(eval_tables.material_score[opening][piece]); }

static int piece_reaches(int pt, int from, int to, u64 occ) {
  const u64 t = 1ULL << to;
  if (pt == P)
    return (attack_tables.pawn_attacks[white][from] & t) != 0;
  if (pt == p)
    return (attack_tables.pawn_attacks[black][from] & t) != 0;
  if (pt == N || pt == n)
    return (attack_tables.knight_attacks[from] & t) != 0;
  if (pt == B || pt == b)
    return (get_bishop_attacks(from, occ) & t) != 0;
  if (pt == R || pt == r)
    return (get_rook_attacks(from, occ) & t) != 0;
  if (pt == A || pt == a)
    return (get_archbishop_attacks(from, occ) & t) != 0;
  if (pt == C || pt == c)
    return (get_chancellor_attacks(from, occ) & t) != 0;
  if (pt == Q || pt == q)
    return (get_queen_attacks(from, occ) & t) != 0;
  if (pt == M || pt == m)
    return (get_amazon_attacks(from, occ) & t) != 0;
  if (pt == K || pt == k)
    return (attack_tables.king_attacks[from] & t) != 0;
  return 0;
}

static int see_victim_value(int move) {
  const int to = get_move_target(move);
  u64 t;

  if (get_move_enpassant(move)) {
    t = 1ULL << (board.side == white ? to + 8 : to - 8);
  } else
    t = 1ULL << to;

  for (int p = 0; p < piece_count; ++p) {
    if (board.bitboards[p] & t)
      return material_cp(p);
  }
  return 0;
}

static int find_lva(int us, u64* bb, u64 oa, int target) {
  int best_from = -1, bestp = -1, bestm = 1 << 30;
  for (int pt = P; pt <= k; ++pt) {
    const int side_ok = (us == white) ? (pt <= (int)K) : (pt > (int)K);
    if (!side_ok)
      continue;
    u64 b = bb[pt];
    while (b) {
      const int from = get_ls1b_index(b);
      if (piece_reaches(pt, from, target, oa) && from != target) {
        const int m = material_cp(pt);
        if (m < bestm) {
          bestm = m;
          bestp = pt;
          best_from = from;
        }
      }
      pop_bit(b, from);
    }
  }
  return bestp >= 0 ? (bestp << 8) | best_from : -1;
}

static int see_swap(int target, u64 bb[18], u64* ow, u64* ob, int us) {
  u64 tmask = 1ULL << target;
  int victim = -1;
  for (int p = 0; p < piece_count; ++p) {
    if (bb[p] & tmask) {
      victim = p;
      break;
    }
  }
  if (victim < 0)
    return 0;
  if (victim == K || victim == k)
    return 0;

  const int victim_val = material_cp(victim);
  const u64 oa = *ow | *ob;
  const int lva = find_lva(us, bb, oa, target);
  if (lva < 0)
    return 0;
  const int from = lva & 0x3f;
  const int att = lva >> 8;

  pop_bit(bb[att], from);
  *ow = *ow & ~(1ULL << from);
  *ob = *ob & ~(1ULL << from);
  pop_bit(bb[victim], target);
  if (victim <= (int)K)
    *ow = *ow & ~tmask;
  else
    *ob = *ob & ~tmask;
  set_bit(bb[att], target);
  if (att <= (int)K)
    *ow |= tmask;
  else
    *ob |= tmask;

  // Stand pat: the side to move may decline the exchange, so this capture is
  // never worth less than not capturing at all (otherwise a forced losing
  // recapture here would poison the SEE of the whole preceding sequence).
  const int gain = victim_val - see_swap(target, bb, ow, ob, us ^ 1);
  return gain > 0 ? gain : 0;
}

int see_evaluate(int move) {
  if (!get_move_capture(move))
    return 0;
  const int v0 = see_victim_value(move);
  Board before;
  memcpy(&before, &board, sizeof(Board));
  if (!make_move(move, all_moves)) {
    memcpy(&board, &before, sizeof(Board));
    return 0;
  }
  u64 bb[18];
  memcpy(bb, board.bitboards, sizeof(bb));
  u64 ow = board.occupancies[white];
  u64 ob = board.occupancies[black];
  const int to = get_move_target(move);
  const int stm = board.side;
  memcpy(&board, &before, sizeof(Board));
  return v0 - see_swap(to, bb, &ow, &ob, stm);
}
