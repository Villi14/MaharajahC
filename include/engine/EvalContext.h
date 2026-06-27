#ifndef EVAL_CONTEXT_H_
#define EVAL_CONTEXT_H_

typedef struct
{
  int requested_eval_mode;
  int active_eval_mode;
  int side_to_move;
  int weights_loaded;
  int nnue_backend_ready;
  int using_classic_fallback;
  int accumulator_initialized;
  char weights_version[0x80];
  char loaded_path[0x100];
} EvalContext;

extern EvalContext eval_context;

void reset_eval_context(void);
void update_eval_context(void);

#endif // EVAL_CONTEXT_H_
