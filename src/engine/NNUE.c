#include "../../include/engine/NNUE.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NnueState nnue_state = {
  .initialized = 0,
  .last_error = 0,
  .loaded_path = {0},
  .weights_version = {0},
  .weights_blob = NULL,
  .weights_size = 0
};

static void nnue_clear_loaded_state(void) {
  if (nnue_state.weights_blob != NULL) {
    free(nnue_state.weights_blob);
    nnue_state.weights_blob = NULL;
  }
  nnue_state.weights_size = 0;
  nnue_state.initialized = 0;
  nnue_state.loaded_path[0] = '\0';
  nnue_state.weights_version[0] = '\0';
}

void init_nnue(void) {
  nnue_clear_loaded_state();
  nnue_state.last_error = 1;
}

int nnue_has_loaded_weights(void) {
  return nnue_state.initialized;
}

int nnue_backend_is_ready(void) {
  return 0;
}

static void copy_trimmed_string(char* out, size_t out_len, const char* value) {
  if (out == NULL || out_len == 0)
    return;

  if (value == NULL) {
    out[0] = '\0';
    return;
  }

  snprintf(out, out_len, "%s", value);
}

static const char* filename_from_path(const char* path) {
  const char* slash = strrchr(path, '/');
#ifdef _WIN32
  const char* backslash = strrchr(path, '\\');
  if (backslash != NULL && (slash == NULL || backslash > slash))
    slash = backslash;
#endif
  return slash != NULL ? slash + 1 : path;
}

static int nnue_set_loaded_blob(const unsigned char* bytes,
                                size_t size,
                                const char* weights_version_name,
                                const char* path) {
  if (bytes == NULL || size == 0) {
    nnue_state.last_error = 4;
    return 0;
  }

  unsigned char* copy = (unsigned char*)malloc(size);
  if (copy == NULL) {
    nnue_state.last_error = 5;
    return 0;
  }

  memcpy(copy, bytes, size);
  nnue_clear_loaded_state();
  nnue_state.weights_blob = copy;
  nnue_state.weights_size = size;
  nnue_state.initialized = 1;
  nnue_state.last_error = 0;
  copy_trimmed_string(nnue_state.loaded_path, sizeof(nnue_state.loaded_path), path);
  copy_trimmed_string(nnue_state.weights_version, sizeof(nnue_state.weights_version),
                      weights_version_name != NULL && *weights_version_name != '\0'
                          ? weights_version_name
                          : "memory");
  return 1;
}

int nnue_load_weights(const char* path) {
  if (path == NULL || *path == '\0') {
    nnue_state.last_error = 2;
    return 0;
  }

  FILE* weights_file = fopen(path, "rb");
  if (weights_file == NULL) {
    nnue_clear_loaded_state();
    nnue_state.last_error = 3;
    return 0;
  }

  if (fseek(weights_file, 0, SEEK_END) != 0) {
    fclose(weights_file);
    nnue_clear_loaded_state();
    nnue_state.last_error = 3;
    return 0;
  }

  long file_size = ftell(weights_file);
  if (file_size <= 0) {
    fclose(weights_file);
    nnue_clear_loaded_state();
    nnue_state.last_error = 4;
    return 0;
  }

  rewind(weights_file);

  unsigned char* bytes = (unsigned char*)malloc((size_t)file_size);
  if (bytes == NULL) {
    fclose(weights_file);
    nnue_state.last_error = 5;
    return 0;
  }

  const size_t read_size = fread(bytes, 1, (size_t)file_size, weights_file);
  fclose(weights_file);
  if (read_size != (size_t)file_size) {
    free(bytes);
    nnue_clear_loaded_state();
    nnue_state.last_error = 3;
    return 0;
  }

  const int loaded = nnue_set_loaded_blob(bytes, (size_t)file_size, filename_from_path(path), path);
  free(bytes);
  return loaded;
}

int nnue_load_weights_from_bytes(const unsigned char* bytes, size_t size,
                                 const char* weights_version_name) {
  return nnue_set_loaded_blob(bytes, size, weights_version_name, "<memory>");
}

void nnue_unload_weights(void) {
  nnue_clear_loaded_state();
  nnue_state.last_error = 1;
}

int evaluate_nnue(void) {
  nnue_state.last_error = 1;
  return 0;
}

const char* nnue_status_string(void) {
  if (nnue_state.initialized)
    return "ready";

  switch (nnue_state.last_error) {
  case 1:
    return "nnue_not_loaded";
  case 2:
    return "invalid_weights_path";
  case 3:
    return "weights_file_unreadable";
  case 4:
    return "weights_payload_invalid";
  case 5:
    return "weights_allocation_failed";
  default:
    return "uninitialized";
  }
}

const char* nnue_weights_version(void) {
  return nnue_state.weights_version;
}

const char* nnue_loaded_path(void) {
  return nnue_state.loaded_path;
}
