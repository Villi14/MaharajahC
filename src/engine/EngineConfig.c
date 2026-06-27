#include "../../include/engine/EngineConfig.h"
#include "../../include/engine/Globals.h"

typedef struct
{
  int max_depth_cap;
  int opening_variety_plies;
  int weak_move_margin_cp;
  int weak_move_candidates;
  int reverse_futility_margin_per_depth;
  int futility_margin_per_depth;
  int late_move_pruning_base;
  int late_move_pruning_scale;
  int history_bonus_scale;
} SkillProfile;

static const int ui_to_skill_level[5] = { 2, 4, 6, 8, 10 };

static const SkillProfile skill_profiles[10] = {
  { 1, 14, 240, 5, 80, 160, 6, 1, 1 },
  { 1, 14, 220, 5, 80, 150, 6, 1, 1 },
  { 2, 12, 205, 5, 85, 145, 7, 1, 1 },
  { 2, 12, 190, 4, 85, 140, 7, 1, 1 },
  { 3, 10, 170, 4, 90, 130, 8, 2, 1 },
  { 4, 10, 150, 4, 90, 125, 8, 2, 1 },
  { 5, 8, 120, 3, 95, 120, 8, 2, 1 },
  { 6, 6, 90, 3, 95, 120, 8, 2, 1 },
  { 8, 4, 50, 2, 100, 120, 8, 2, 1 },
  { max_ply, 0, 0, 1, 90, 120, 8, 2, 2 }
};

EngineSearchConfig engine_search_config;

EngineEvalConfig engine_eval_config = {
  .eval_mode = eval_mode_classic,
  .tempo_bonus = 12,
  .bishop_pair_bonus_opening = 35,
  .bishop_pair_bonus_endgame = 50,
  .rook_mobility_opening = 2,
  .rook_mobility_endgame = 4,
  .knight_mobility_opening = 4,
  .knight_mobility_endgame = 4
};

int clamp_skill_level(int skill_level) {
  if (skill_level < 1)
    return 1;
  if (skill_level > 10)
    return 10;
  return skill_level;
}

int clamp_ui_difficulty(int difficulty_level) {
  if (difficulty_level < 1)
    return 1;
  if (difficulty_level > 5)
    return 5;
  return difficulty_level;
}

int clamp_eval_mode(int eval_mode) {
  if (eval_mode == eval_mode_nnue)
    return eval_mode_nnue;
  return eval_mode_classic;
}

void set_engine_skill_level(int skill_level) {
  const int clamped_skill_level = clamp_skill_level(skill_level);
  const SkillProfile profile = skill_profiles[clamped_skill_level - 1];

  engine_search_config.ui_difficulty = 0;
  engine_search_config.skill_level = clamped_skill_level;
  engine_search_config.max_depth_cap = profile.max_depth_cap;
  engine_search_config.opening_variety_plies = profile.opening_variety_plies;
  engine_search_config.weak_move_margin_cp = profile.weak_move_margin_cp;
  engine_search_config.weak_move_candidates = profile.weak_move_candidates;
  engine_search_config.reverse_futility_margin_per_depth = profile.reverse_futility_margin_per_depth;
  engine_search_config.futility_margin_per_depth = profile.futility_margin_per_depth;
  engine_search_config.late_move_pruning_base = profile.late_move_pruning_base;
  engine_search_config.late_move_pruning_scale = profile.late_move_pruning_scale;
  engine_search_config.history_bonus_scale = profile.history_bonus_scale;
  engine_search_config.quiescence_see_prune_margin = 0;
}

void set_engine_ui_difficulty(int difficulty_level) {
  const int clamped_difficulty_level = clamp_ui_difficulty(difficulty_level);
  set_engine_skill_level(ui_to_skill_level[clamped_difficulty_level - 1]);
  engine_search_config.ui_difficulty = clamped_difficulty_level;
}

void reset_engine_config(void) {
  set_engine_ui_difficulty(5);
  set_engine_eval_mode(eval_mode_classic);
  engine_search_config.quiescence_see_prune_margin = 0;
}

void set_engine_eval_mode(int eval_mode) {
  engine_eval_config.eval_mode = clamp_eval_mode(eval_mode);
}
