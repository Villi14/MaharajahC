#include <stdio.h>
#include <stdlib.h>

#include "../../include/engine/Transposition.h"

TranspositionTable transposition_table = { .table = NULL, .entries = 0 };

void clear_hash_table(void) {
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

  if (transposition_table.table != NULL) {
    fprintf(stderr, "    Clearing hash memory...\n");
    free(transposition_table.table);
  }

  transposition_table.table = (tt*)malloc(transposition_table.entries * sizeof(tt));

  if (transposition_table.table == NULL) {
    fprintf(stderr, "    Couldn't allocate memory for hash table, tryinr %dMB...", mb / 2);
    init_hash_table(mb / 2);
  } else {
    clear_hash_table();
    fprintf(stderr, "    Hash table is initialed with %d entries\n", transposition_table.entries);
  }
}
