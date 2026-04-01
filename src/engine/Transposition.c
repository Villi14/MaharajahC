#include <stdio.h>
#include <stdlib.h>

#include "maharajah/engine/Transposition.h"

TranspositionTable transposition_table = { .table = nullptr, .entries = 0 };

void clear_hash_table() {
  tt* hash_entry;

  for (hash_entry = transposition_table.table; hash_entry < transposition_table.table + transposition_table.entries; ++hash_entry) {
    hash_entry->hash_key = 0;
    hash_entry->depth = 0;
    hash_entry->flag = 0;
    hash_entry->score = 0;
  }
}

void init_hash_table(const int mb) {
  int hash_size = 0x100000 * mb;
  transposition_table.entries = hash_size / sizeof(tt);

  if (transposition_table.table != nullptr) {
    printf("    Clearing hash memory...\n");
    free(transposition_table.table);
  }

  transposition_table.table = (tt*)malloc(transposition_table.entries * sizeof(tt));

  if (transposition_table.table == nullptr) {
    printf("    Couldn't allocate memory for hash table, tryinr %dMB...", mb / 2);
    init_hash_table(mb / 2);
  } else {
    clear_hash_table();
    printf("    Hash table is initialed with %d entries\n", transposition_table.entries);
  }
}
