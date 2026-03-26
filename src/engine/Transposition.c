#include <stdio.h>
#include <stdlib.h>

#include "Transposition.h"

TranspositionTable transposition_table = { .table = nullptr, .entries = 0 };

// clear TT (hash table)
void clear_hash_table() {
  // init hash table entry pointer
  tt* hash_entry;

  // loop over TT elements
  for (hash_entry = transposition_table.table; hash_entry < transposition_table.table + transposition_table.entries; hash_entry++) {
    // reset TT inner fields
    hash_entry->hash_key = 0;
    hash_entry->depth = 0;
    hash_entry->flag = 0;
    hash_entry->score = 0;
  }
}

// dynamically allocate memory for hash table
void init_hash_table(int mb) {
  // init hash size
  int hash_size = 0x100000 * mb;

  // init number of hash entries
  transposition_table.entries = hash_size / sizeof(tt);

  // free hash table if not empty
  if (transposition_table.table != nullptr) {
    printf("    Clearing hash memory...\n");

    // free hash table dynamic memory
    free(transposition_table.table);
  }

  // allocate memory
  transposition_table.table = (tt*)malloc(transposition_table.entries * sizeof(tt));

  // if allocation has failed
  if (transposition_table.table == nullptr) {
    printf("    Couldn't allocate memory for hash table, tryinr %dMB...", mb / 2);

    // try to allocate with half size
    init_hash_table(mb / 2);
  }

  // if allocation succeeded
  else {
    // clear hash table
    clear_hash_table();

    printf("    Hash table is initialied with %d entries\n", transposition_table.entries);
  }
}
