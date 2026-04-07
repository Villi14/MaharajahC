#ifndef ENGINE_FFI_H_
#define ENGINE_FFI_H_

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif

// Initialize engine tables and set position to startpos.
// Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_init(void);

// Set position from a FEN string.
// Returns 1 on success, 0 on failure.
FFI_PLUGIN_EXPORT int mah_set_position_fen(const char* fen);

// Set position to standard chess startpos.
// Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_position_startpos(void);

// Apply a move in coordinate notation (e.g. "e2e4", "e7e8q").
// Returns 1 if the move was applied, 0 if invalid/illegal.
FFI_PLUGIN_EXPORT int mah_apply_move(const char* move);

// Find best move by depth. Writes move string to out_move.
// out_len must be at least 5 (or 6 for promotions). Returns 1 if a move is found.
FFI_PLUGIN_EXPORT int mah_best_move_depth(int depth, char* out_move, int out_len);

// Find best move by movetime in milliseconds. Writes move string to out_move.
// out_len must be at least 5 (or 6 for promotions). Returns 1 if a move is found.
FFI_PLUGIN_EXPORT int mah_best_move_time(int movetime_ms, char* out_move, int out_len);

// Resize hash table (MB). Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_set_hash_mb(int mb);

// Release engine resources. Returns 1 on success.
FFI_PLUGIN_EXPORT int mah_shutdown(void);

#endif // ENGINE_FFI_H_
