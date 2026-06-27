#ifndef NNUE_H_
#define NNUE_H_

#include <stddef.h>

#include "../../include/engine/Globals.h"

typedef struct
{
  int initialized;
  int last_error;
  char loaded_path[0x100];
  char weights_version[0x80];
  unsigned char* weights_blob;
  size_t weights_size;
} NnueState;

extern NnueState nnue_state;

void init_nnue(void);
int nnue_has_loaded_weights(void);
int nnue_backend_is_ready(void);
int nnue_load_weights(const char* path);
int nnue_load_weights_from_bytes(const unsigned char* bytes, size_t size,
                                 const char* weights_version_name);
void nnue_unload_weights(void);
int evaluate_nnue(void);
const char* nnue_status_string(void);
const char* nnue_weights_version(void);
const char* nnue_loaded_path(void);

#endif // NNUE_H_
