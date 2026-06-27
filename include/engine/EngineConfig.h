#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

enum
{
  eval_mode_classic,
  eval_mode_nnue
};

typedef struct
{
  int ui_difficulty;
  int skill_level;
  int max_depth_cap;
  int opening_variety_plies;
  int weak_move_margin_cp;
  int weak_move_candidates;
  int reverse_futility_margin_per_depth;
  int futility_margin_per_depth;
  int late_move_pruning_base;
  int late_move_pruning_scale;
  int history_bonus_scale;
  /* Prune in quiescence when see < -margin and stand_pat+see < alpha (margin=0 => see < 0). */
  int quiescence_see_prune_margin;
} EngineSearchConfig;

typedef struct
{
  int eval_mode;
  int tempo_bonus;
  int bishop_pair_bonus_opening;
  int bishop_pair_bonus_endgame;
  int rook_mobility_opening;
  int rook_mobility_endgame;
  int knight_mobility_opening;
  int knight_mobility_endgame;
} EngineEvalConfig;

extern EngineSearchConfig engine_search_config;
extern EngineEvalConfig engine_eval_config;

int clamp_skill_level(int skill_level);
int clamp_ui_difficulty(int difficulty_level);
int clamp_eval_mode(int eval_mode);
void reset_engine_config(void);
void set_engine_skill_level(int skill_level);
void set_engine_ui_difficulty(int difficulty_level);
void set_engine_eval_mode(int eval_mode);

#endif // ENGINE_CONFIG_H_
