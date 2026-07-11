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
  // Clamp to [1, 1024] MB: below keeps the table usable, above keeps the
  // byte count inside 32-bit range (0x100000 * mb would overflow int at
  // 2048 MB) and inside what the engine sensibly needs.
  int clamped_mb = mb;
  if (clamped_mb < 1)
    clamped_mb = 1;
  if (clamped_mb > 1024)
    clamped_mb = 1024;

  const size_t hash_size = (size_t)0x100000 * (size_t)clamped_mb;
  transposition_table.entries = (int)(hash_size / sizeof(tt));

  if (transposition_table.table != NULL) {
    fprintf(stderr, "    Clearing hash memory...\n");
    free(transposition_table.table);
  }

  transposition_table.table = (tt*)malloc((size_t)transposition_table.entries * sizeof(tt));

  if (transposition_table.table == NULL) {
    if (clamped_mb <= 1) {
      // Out of memory even for the minimum size: leave the table disabled;
      // read_hash_entry / write_hash_entry no-op on an empty table.
      transposition_table.entries = 0;
      fprintf(stderr, "    Couldn't allocate hash table memory, running without one\n");
      return;
    }
    fprintf(stderr, "    Couldn't allocate memory for hash table, trying %dMB...\n", clamped_mb / 2);
    init_hash_table(clamped_mb / 2);
  } else {
    clear_hash_table();
    fprintf(stderr, "    Hash table is initialized with %d entries\n", transposition_table.entries);
  }
}
