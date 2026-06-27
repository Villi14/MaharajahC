#ifndef ENGINE_FFI_H_
#define ENGINE_FFI_H_

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT                                                      \
  __attribute__((visibility("default"))) __attribute__((used))
#endif

enum {
  mah_rules_auto = 0,
  mah_rules_standard = 1,
  mah_rules_variant = 2,
};

// Initialize engine tables and set position to startpos.
// Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_init(void);

// Set position from a FEN string.
// Returns 1 on success, 0 on failure.
FFI_PLUGIN_EXPORT int mah_set_position_fen(const char *fen);

// Set position from a FEN string with an explicit rules profile.
// rules_profile: 0 = auto, 1 = standard, 2 = variant.
// Returns 1 on success, 0 on failure.
FFI_PLUGIN_EXPORT int mah_set_position_fen_with_rules(const char *fen,
                                                      int rules_profile);

// Set position to standard chess startpos.
// Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_position_startpos(void);

// Apply a move in coordinate notation (e.g. "e2e4", "e7e8q").
// Returns 1 if the move was applied, 0 if invalid/illegal.
FFI_PLUGIN_EXPORT int mah_apply_move(const char *move);

// Find best move by depth. Writes move string to out_move.
// out_len must be at least 5 (or 6 for promotions). Returns 1 if a move is
// found.
FFI_PLUGIN_EXPORT int mah_best_move_depth(int depth, char *out_move,
                                          int out_len);

// Find best move by movetime in milliseconds. Writes move string to out_move.
// out_len must be at least 5 (or 6 for promotions). Returns 1 if a move is
// found.
FFI_PLUGIN_EXPORT int mah_best_move_time(int movetime_ms, char *out_move,
                                         int out_len);

// Generate a reasonable custom-start FEN.
// side_to_move: 0 = white, 1 = black, any other value = random.
// Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_generate_custom_position_fen(int side_to_move,
                                                       unsigned int seed,
                                                       char *out_fen,
                                                       int out_len);

// Resize hash table (MB). Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_hash_mb(int mb);

// Set engine skill level in the range [1..10]. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_skill_level(int skill_level);

// Set UI difficulty level in the range [1..5]. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_difficulty_level(int difficulty_level);

// Set evaluation mode: 0 = classic, 1 = nnue.
// Current NNUE mode is a staged integration path and may still run classic
// fallback. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_eval_mode(int eval_mode);

// Load NNUE weights from a file path for the staged NNUE integration path.
// Loading weights does not imply that NNUE inference is already active. Returns
// 1 on success.
FFI_PLUGIN_EXPORT int mah_load_weights(const char *path);

// Load NNUE weights from a bytes buffer for the staged NNUE integration path.
// Loading weights does not imply that NNUE inference is already active. Returns
// 1 on success.
FFI_PLUGIN_EXPORT int
mah_load_weights_from_bytes(const unsigned char *bytes, int len,
                            const char *weights_version_name);

// Unload current NNUE weights and reset to fallback state. Returns 1 on
// success.
FFI_PLUGIN_EXPORT int mah_unload_weights(void);

// Get current eval backend status string.
// Example values: classic_ready, nnue_not_loaded, nnue_stub_fallback,
// nnue_ready. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_get_eval_status(char *out_status, int out_len);

// Get current loaded weights version string, or empty if none. Returns 1 on
// success.
FFI_PLUGIN_EXPORT int mah_get_weights_version(char *out_version, int out_len);

// Get a compact JSON description of eval/NNUE runtime state, including
// requested vs active backend. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_get_nnue_info(char *out_info, int out_len);

// Release engine resources. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_shutdown(void);

#endif // ENGINE_FFI_H_
