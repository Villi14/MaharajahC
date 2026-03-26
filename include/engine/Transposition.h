#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "Defines.h"
#include "Globals.h"

// no hash entry found constant
#define no_hash_entry 100000

// transposition table hash flags
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

// transposition table data structure
typedef struct {
  U64 hash_key; // "almost" unique chess position identifier
  int depth; // current search depth
  int flag; // flag the type of node (fail-low/fail-high/PV)
  int score; // score (alpha/beta/PV)
} tt; // transposition table (TT aka hash table)

typedef struct {
  tt* table;
  int entries;
} TranspositionTable;

extern TranspositionTable transposition_table;

// clear TT (hash table)
void clear_hash_table();

// dynamically allocate memory for hash table
void init_hash_table(int mb);

// read hash entry data
static inline int read_hash_entry(int alpha, int beta, int depth) {
  // create a TT instance pointer to particular hash entry storing
  // the scoring data for the current board position if available
  tt* hash_entry = &transposition_table.table[board.hash_key % transposition_table.entries];

  // make sure we're dealing with the exact position we need
  if (hash_entry->hash_key == board.hash_key) {
    // make sure that we match the exact depth our search is now at
    if (hash_entry->depth >= depth) {
      // extract stored score from TT entry
      int score = hash_entry->score;

      // retrieve score independent from the actual path
      // from root node (position) to current node (position)
      if (score < -mate_score)
        score += search_context.ply;
      if (score > mate_score)
        score -= search_context.ply;

      // match the exact (PV node) score
      if (hash_entry->flag == hash_flag_exact)
        // return exact (PV node) score
        return score;

      // match alpha (fail-low node) score
      if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha))
        // return alpha (fail-low node) score
        return alpha;

      // match beta (fail-high node) score
      if ((hash_entry->flag == hash_flag_beta) && (score >= beta))
        // return beta (fail-high node) score
        return beta;
    }
  }

  // if hash entry doesn't exist
  return no_hash_entry;
}

// write hash entry data
static inline void write_hash_entry(int score, int depth, int hash_flag) {
  // create a TT instance pointer to particular hash entry storing
  // the scoring data for the current board position if available
  tt* hash_entry = &transposition_table.table[board.hash_key % transposition_table.entries];

  // store score independent from the actual path
  // from root node (position) to current node (position)
  if (score < -mate_score)
    score -= search_context.ply;
  if (score > mate_score)
    score += search_context.ply;

  // write hash entry data
  hash_entry->hash_key = board.hash_key;
  hash_entry->score = score;
  hash_entry->flag = hash_flag;
  hash_entry->depth = depth;
}

#endif // !TRANSPOSITION_H_
