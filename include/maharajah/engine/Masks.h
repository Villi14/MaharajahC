#ifndef MASKS_H_
#define MASKS_H_

#include "maharajah/util/Defines.h"
#include "maharajah/engine/Globals.h"
#include "maharajah/util/Utils.h"

extern u64 file_masks[0x40];

extern u64 rank_masks[0x40];

extern u64 isolated_masks[0x40];

extern u64 white_passed_masks[0x40];

extern u64 black_passed_masks[0x40];

extern const int get_rank[0x40];

int get_game_phase_score(void);

void init_evaluation_masks(void);

u64 set_file_rank_mask(int file_number, int rank_number);

#endif // MASKS_H_
