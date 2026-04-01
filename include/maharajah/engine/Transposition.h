#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "maharajah/util/Defines.h"
#include "maharajah/engine/Globals.h"

enum {
  hash_flag_exact = 0,
  hash_flag_alpha = 1,
  hash_flag_beta = 2,
  no_hash_entry = 100000,
};

typedef struct {
  u64 hash_key;
  int depth;
  int flag;
  int score;
} tt;

typedef struct {
  tt* table;
  int entries;
} TranspositionTable;

extern TranspositionTable transposition_table;

void clear_hash_table();

void init_hash_table(int mb);

static inline int read_hash_entry(int alpha, int beta, int depth) {
  tt* hash_entry = &transposition_table.table[board.hash_key % transposition_table.entries];

  if (hash_entry->hash_key == board.hash_key) {
    if (hash_entry->depth >= depth) {
      int score = hash_entry->score;

      if (score < -mate_score)
        score += search_context.ply;

      if (score > mate_score)
        score -= search_context.ply;

      if (hash_entry->flag == hash_flag_exact)
        return score;

      if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha))
        return alpha;

      if ((hash_entry->flag == hash_flag_beta) && (score >= beta))
        return beta;
    }
  }
  
  return no_hash_entry;
}

static inline void write_hash_entry(int score, int depth, int hash_flag) {
  tt* hash_entry = &transposition_table.table[board.hash_key % transposition_table.entries];
  if (score < -mate_score)
    score -= search_context.ply;

  if (score > mate_score)
    score += search_context.ply;

  hash_entry->hash_key = board.hash_key;
  hash_entry->score = score;
  hash_entry->flag = hash_flag;
  hash_entry->depth = depth;
}

#endif // !TRANSPOSITION_H_
