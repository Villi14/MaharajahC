#include <stdio.h>
#include <string.h>

#include "../include/engine/Globals.h"
#include "../include/engine/Search.h"
#include "../include/engine/Transposition.h"
#include "../include/ffi/maharajah_ffi.h"

static int fail(const char *message) {
  fprintf(stderr, "%s\n", message);
  mah_shutdown();
  return 1;
}

static int row_has_piece(const char *board_part, int target_row) {
  int row = 0;

  for (const char *cursor = board_part; *cursor != '\0' && *cursor != ' ';
       ++cursor) {
    if (*cursor == '/') {
      ++row;
      continue;
    }

    if (*cursor >= '1' && *cursor <= '8') {
      continue;
    }

    if (row == target_row) {
      return 1;
    }
  }

  return 0;
}

static int row_is_full(const char *board_part, int target_row) {
  int row = 0;
  int occupied = 0;

  for (const char *cursor = board_part; *cursor != '\0' && *cursor != ' ';
       ++cursor) {
    if (*cursor == '/') {
      if (row == target_row) {
        return occupied == 8;
      }
      ++row;
      occupied = 0;
      continue;
    }

    if (*cursor >= '1' && *cursor <= '8') {
      continue;
    }

    ++occupied;
  }

  return row == target_row && occupied == 8;
}

static int respects_row_progression(const char *fen) {
  int found_incomplete = 0;

  for (int row = 7; row >= 4; --row) {
    const int has_piece = row_has_piece(fen, row);
    const int is_full = row_is_full(fen, row);
    if (found_incomplete && has_piece) {
      return 0;
    }
    if (!is_full) {
      found_incomplete = 1;
    }
  }

  found_incomplete = 0;
  for (int row = 0; row <= 3; ++row) {
    const int has_piece = row_has_piece(fen, row);
    const int is_full = row_is_full(fen, row);
    if (found_incomplete && has_piece) {
      return 0;
    }
    if (!is_full) {
      found_incomplete = 1;
    }
  }

  return 1;
}

int main(void) {
  char generated_fen[128] = {0};
  char out_move[16] = {0};
  char eval_status[64] = {0};
  char nnue_info[512] = {0};

  if (!mah_init())
    return fail("ffi_api_smoke failed: mah_init returned false.");

  if (!mah_set_position_startpos())
    return fail(
        "ffi_api_smoke failed: mah_set_position_startpos returned false.");

  if (!mah_apply_move("e2e4"))
    return fail("ffi_api_smoke failed: valid move e2e4 was rejected.");

  if (mah_apply_move("e7e5x"))
    return fail("ffi_api_smoke failed: move string with trailing garbage was "
                "accepted.");

  if (mah_apply_move("e7e3"))
    return fail("ffi_api_smoke failed: illegal move was accepted.");

  {
    char fen_out[128] = {0};

    // A double pawn push must publish the en passant target square and switch
    // the side to move; the caller-supplied fullmove number is written verbatim.
    if (!mah_set_position_startpos())
      return fail("ffi_api_smoke failed: get_fen start position not set.");
    if (!mah_apply_move("e2e4"))
      return fail("ffi_api_smoke failed: get_fen setup move e2e4 rejected.");
    if (!mah_get_fen(fen_out, (int)sizeof(fen_out), 1))
      return fail("ffi_api_smoke failed: mah_get_fen returned false.");
    if (strcmp(fen_out,
               "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 -") !=
        0)
      return fail("ffi_api_smoke failed: mah_get_fen produced an unexpected FEN "
                  "after a double pawn push.");

    // Moving the king must clear both castling rights for that side, the
    // halfmove clock must increment for a quiet non-pawn move, and the caller's
    // fullmove number is echoed back unchanged.
    if (!mah_set_position_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 5 9 "))
      return fail("ffi_api_smoke failed: get_fen castling FEN not set.");
    if (!mah_apply_move("e1f1"))
      return fail("ffi_api_smoke failed: get_fen king move rejected.");
    if (!mah_get_fen(fen_out, (int)sizeof(fen_out), 9))
      return fail("ffi_api_smoke failed: mah_get_fen returned false after king "
                  "move.");
    if (strcmp(fen_out, "r3k2r/8/8/8/8/8/8/R4K1R b kq - 6 9 -") != 0)
      return fail("ffi_api_smoke failed: mah_get_fen did not drop castling "
                  "rights after a king move.");

    if (mah_get_fen(fen_out, 8, 1))
      return fail("ffi_api_smoke failed: mah_get_fen accepted an undersized "
                  "output buffer.");

    // Field 8 (unmoved-pawn state) must survive a get_fen -> set_fen
    // round-trip, and make_move must keep it exact: once the h6 pawn moves,
    // its unmoved right is spent and field 8 collapses to "-".
    if (!mah_set_position_fen("8/5p2/7p/1K6/4a3/5k2/8/8 b - - 25 44 Vv h6"))
      return fail("ffi_api_smoke failed: pawn-state FEN could not be set.");
    if (!mah_get_fen(fen_out, (int)sizeof(fen_out), 44))
      return fail("ffi_api_smoke failed: mah_get_fen failed for a pawn-state "
                  "position.");
    if (strcmp(fen_out, "8/5p2/7p/1K6/4a3/5k2/8/8 b - - 25 44 Vv h6") != 0)
      return fail("ffi_api_smoke failed: mah_get_fen did not round-trip "
                  "field 8.");
    if (!mah_apply_move("h6h5"))
      return fail("ffi_api_smoke failed: pawn-state setup move h6h5 rejected.");
    if (!mah_get_fen(fen_out, (int)sizeof(fen_out), 45))
      return fail("ffi_api_smoke failed: mah_get_fen failed after the "
                  "pawn-state pawn moved.");
    if (strcmp(fen_out, "8/5p2/8/1K5p/4a3/5k2/8/8 w - - 0 45 Vv -") != 0)
      return fail("ffi_api_smoke failed: field 8 was not cleared after the "
                  "unmoved pawn moved.");
    if (!mah_set_position_fen(fen_out))
      return fail("ffi_api_smoke failed: round-tripped pawn-state FEN could "
                  "not be reloaded.");
  }

  // A FEN without exactly one king per side must be rejected, leaving the
  // engine on a safe, usable startpos.
  if (mah_set_position_fen("8/8/8/8/8/8/8/8 w - - 0 1 "))
    return fail("ffi_api_smoke failed: kingless FEN was accepted.");
  if (mah_set_position_fen("kk6/8/8/8/8/8/8/K7 w - - 0 1 "))
    return fail("ffi_api_smoke failed: FEN with two black kings was accepted.");
  if (mah_game_status() != mah_status_ongoing)
    return fail("ffi_api_smoke failed: engine was not left on a usable "
                "position after rejecting an invalid FEN.");

  if (!mah_set_position_fen("7k/P7/8/8/8/8/8/K7 w - - 0 1 "))
    return fail("ffi_api_smoke failed: mah_set_position_fen returned false.");

  if (!mah_set_position_fen_with_rules("7k/8/8/8/3M4/8/8/7K w - - 150 1 ",
                                       mah_rules_standard))
    return fail("ffi_api_smoke failed: explicit standard-rules FEN load "
                "returned false.");
  if (board.standard_rules != 0 || is_fifty_move_rule())
    return fail("ffi_api_smoke failed: compound-piece FEN incorrectly enabled "
                "standard rules.");

  if (!mah_set_position_fen_with_rules("7k/8/8/8/8/8/6N1/7K w - - 150 1 ",
                                       mah_rules_variant))
    return fail("ffi_api_smoke failed: explicit variant-rules FEN load "
                "returned false.");
  if (board.standard_rules != 0 || is_fifty_move_rule() ||
      is_seventy_five_move_rule())
    return fail("ffi_api_smoke failed: explicit variant-rules FEN did not "
                "disable standard draw rules.");

  // Amazon promotion is a variant-rules privilege, so load the position under
  // the variant profile (this bare board carries no compound piece for the
  // standard/variant inference to key off).
  if (!mah_set_position_fen_with_rules("7k/P7/8/8/8/8/8/K7 w - - 0 1 ",
                                       mah_rules_variant))
    return fail("ffi_api_smoke failed: promotion FEN reload returned false.");

  if (!mah_apply_move("a7a8m"))
    return fail("ffi_api_smoke failed: valid amazon promotion was rejected.");

  if (!mah_set_position_fen("p3k3/8/8/8/8/8/8/4K2P w - - 0 1 "))
    return fail("ffi_api_smoke failed: FEN with pawns on home edge ranks was "
                "rejected.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail(
        "ffi_api_smoke failed: home-edge pawn position produced no best move.");

  if (mah_set_position_fen("7k/P7/8/8/8/8/8/K7 w - - 0 1 ") &&
      mah_apply_move("a7a8mx"))
    return fail("ffi_api_smoke failed: promotion move with trailing garbage "
                "was accepted.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_fen("k7/5q2/8/4A3/8/8/8/K7 w - - 0 1 "))
    return fail("ffi_api_smoke failed: search FEN could not be set.");
  if (!mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail(
        "ffi_api_smoke failed: mah_best_move_depth did not return a move.");
  if (strcmp(out_move, "e5f7") != 0)
    return fail("ffi_api_smoke failed: mah_best_move_depth returned an "
                "unexpected move.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_fen("k7/2Q5/1K6/8/8/8/8/8 b - - 0 1 "))
    return fail("ffi_api_smoke failed: stalemate FEN could not be set.");
  if (mah_game_status() != mah_status_stalemate)
    return fail("ffi_api_smoke failed: mah_game_status did not report stalemate "
                "for a no-legal-moves, not-in-check position.");
  if (mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: mah_best_move_depth returned a move in "
                "stalemate.");
  if (mah_best_move_time(10, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: mah_best_move_time returned a move in "
                "stalemate.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_fen("k7/1M6/2K5/8/8/8/8/8 b - - 0 1 "))
    return fail("ffi_api_smoke failed: checkmate FEN could not be set.");
  if (mah_game_status() != mah_status_checkmate)
    return fail("ffi_api_smoke failed: mah_game_status did not report checkmate "
                "for a no-legal-moves, in-check position.");
  if (mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: mah_best_move_depth returned a move in "
                "checkmate.");
  if (mah_best_move_time(10, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: mah_best_move_time returned a move in "
                "checkmate.");

  // Regression guard for the online checkmate-hang: the exact position reached
  // by Be5xh2# in the failing online smoke match must classify as checkmate so
  // the server can terminate the session instead of leaving both clients
  // waiting for a completion that never comes.
  if (!mah_set_position_fen(
          "2kr3r/1qp2p2/p1p1p3/3p2B1/4P3/P1N2b2/1PP2P1b/1RQ2RK1 w - - 0 20 -"))
    return fail("ffi_api_smoke failed: online mate FEN could not be set.");
  if (mah_game_status() != mah_status_checkmate)
    return fail("ffi_api_smoke failed: online mate position (Be5xh2#) was not "
                "classified as checkmate.");

  // A fresh start position must classify as ongoing.
  if (!mah_set_position_startpos())
    return fail("ffi_api_smoke failed: ongoing-status start position could not "
                "be set.");
  if (mah_game_status() != mah_status_ongoing)
    return fail("ffi_api_smoke failed: start position was not classified as "
                "ongoing.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_startpos())
    return fail(
        "ffi_api_smoke failed: timed-search start position could not be set.");
  if (!mah_best_move_time(10, out_move, (int)sizeof(out_move)))
    return fail(
        "ffi_api_smoke failed: mah_best_move_time did not return a move.");
  if (out_move[0] == '\0')
    return fail(
        "ffi_api_smoke failed: mah_best_move_time returned an empty move.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_fen("7k/8/8/8/8/8/6N1/7K w - - 0 1 "))
    return fail("ffi_api_smoke failed: repetition-history FEN could not be set.");
  if (!mah_apply_move("g2f4") || !mah_apply_move("h8g8") ||
      !mah_apply_move("f4g2") || !mah_apply_move("g8h8") ||
      !mah_apply_move("g2f4") || !mah_apply_move("h8g8"))
    return fail("ffi_api_smoke failed: could not build repetition-history sequence.");
  if (!mah_best_move_depth(2, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: repetition-history search returned no move.");
  if (strcmp(out_move, "f4g2") == 0)
    return fail("ffi_api_smoke failed: best-move search still selected the repeating move.");

  if (mah_best_move_depth(1, out_move, 4))
    return fail("ffi_api_smoke failed: mah_best_move_depth accepted an "
                "undersized output buffer.");

  if (!mah_set_hash_mb(4))
    return fail("ffi_api_smoke failed: mah_set_hash_mb returned false.");

  if (!mah_set_difficulty_level(3))
    return fail(
        "ffi_api_smoke failed: mah_set_difficulty_level returned false.");

  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info failed after setting "
                "difficulty.");
  if (strstr(nnue_info, "\"uiDifficulty\":3") == NULL ||
      strstr(nnue_info, "\"skillLevel\":6") == NULL)
    return fail("ffi_api_smoke failed: difficulty level did not map to the "
                "expected search profile.");

  if (!mah_set_skill_level(3))
    return fail("ffi_api_smoke failed: mah_set_skill_level returned false.");

  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info failed after setting "
                "skill level.");
  if (strstr(nnue_info, "\"uiDifficulty\":0") == NULL ||
      strstr(nnue_info, "\"skillLevel\":3") == NULL)
    return fail("ffi_api_smoke failed: direct skill tuning did not clear UI "
                "difficulty ownership.");

  if (!mah_set_difficulty_level(0))
    return fail("ffi_api_smoke failed: mah_set_difficulty_level lower clamp "
                "returned false.");
  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info failed after lower "
                "clamp difficulty.");
  if (strstr(nnue_info, "\"uiDifficulty\":1") == NULL ||
      strstr(nnue_info, "\"skillLevel\":2") == NULL)
    return fail("ffi_api_smoke failed: lower-clamped difficulty did not map to "
                "the weakest UI profile.");

  if (!mah_set_skill_level(99))
    return fail("ffi_api_smoke failed: mah_set_skill_level upper clamp "
                "returned false.");
  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info failed after upper "
                "clamp skill.");
  if (strstr(nnue_info, "\"uiDifficulty\":0") == NULL ||
      strstr(nnue_info, "\"skillLevel\":10") == NULL)
    return fail("ffi_api_smoke failed: upper-clamped skill did not restore "
                "full-strength tuning.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_startpos())
    return fail("ffi_api_smoke failed: could not reset start position after "
                "skill change.");
  if (!mah_best_move_depth(6, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: mah_best_move_depth returned no move "
                "after skill change.");

  if (!mah_set_skill_level(10))
    return fail("ffi_api_smoke failed: mah_set_skill_level could not restore "
                "full strength.");

  if (!mah_set_difficulty_level(5))
    return fail("ffi_api_smoke failed: mah_set_difficulty_level could not "
                "restore UI strength.");

  if (!mah_set_eval_mode(1))
    return fail(
        "ffi_api_smoke failed: mah_set_eval_mode(nnue) returned false.");

  memset(eval_status, 0, sizeof(eval_status));
  if (!mah_get_eval_status(eval_status, (int)sizeof(eval_status)))
    return fail("ffi_api_smoke failed: mah_get_eval_status returned false "
                "after eval mode change.");
  if (strcmp(eval_status, "classic_ready") != 0)
    return fail("ffi_api_smoke failed: eval status should be classic_ready "
                "while NNUE is requested but not active.");

  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info returned false.");
  if (strstr(nnue_info, "\"status\":\"classic_ready\"") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info did not report "
                "classic_ready status.");
  if (strstr(nnue_info, "\"requestedEvalMode\":\"nnue\"") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info should report "
                "requested eval mode nnue after set_eval_mode(1).");
  if (strstr(nnue_info, "\"activeEvalMode\":\"classic\"") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info did not report "
                "active classic mode.");
  if (strstr(nnue_info, "\"usingClassicFallback\":1") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info should report "
                "classic fallback while NNUE is not running.");
  if (strstr(nnue_info, "\"weightsLoaded\":0") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info reported unexpected "
                "loaded weights in offline mode.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_set_position_fen("k7/5q2/8/4A3/8/8/8/K7 w - - 0 1 "))
    return fail("ffi_api_smoke failed: classic search FEN could not be set "
                "after eval mode change.");
  if (!mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: classic eval mode produced no best "
                "move after eval mode change.");
  if (strcmp(out_move, "e5f7") != 0)
    return fail("ffi_api_smoke failed: eval mode clamp did not preserve "
                "classic move selection.");

  {
    FILE *weights_file = fopen("ffi_api_smoke_weights.nnue", "wb");
    if (weights_file == NULL)
      return fail(
          "ffi_api_smoke failed: could not create temporary weights file.");
    fputs("stub nnue weights\n", weights_file);
    fclose(weights_file);
  }

  if (!mah_load_weights("ffi_api_smoke_weights.nnue"))
    return fail("ffi_api_smoke failed: mah_load_weights returned false for a "
                "readable file path.");
  if (!mah_unload_weights())
    return fail("ffi_api_smoke failed: mah_unload_weights returned false.");
  if (!mah_load_weights_from_bytes((const unsigned char *)"NNUE", 4,
                                   "memory-v1"))
    return fail(
        "ffi_api_smoke failed: mah_load_weights_from_bytes returned false.");
  if (!mah_unload_weights())
    return fail("ffi_api_smoke failed: mah_unload_weights returned false after "
                "bytes load.");

  memset(nnue_info, 0, sizeof(nnue_info));
  if (!mah_get_nnue_info(nnue_info, (int)sizeof(nnue_info)))
    return fail("ffi_api_smoke failed: mah_get_nnue_info failed after offline "
                "weight lifecycle calls.");
  if (strstr(nnue_info, "\"weightsLoaded\":0") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info did not report "
                "cleared weight state.");
  if (strstr(nnue_info, "\"uiDifficulty\":5") == NULL ||
      strstr(nnue_info, "\"skillLevel\":10") == NULL)
    return fail("ffi_api_smoke failed: mah_get_nnue_info did not report "
                "current engine config.");

  if (!mah_set_eval_mode(0))
    return fail(
        "ffi_api_smoke failed: mah_set_eval_mode(classic) returned false.");

  if (!mah_set_eval_mode(99))
    return fail("ffi_api_smoke failed: mah_set_eval_mode invalid value "
                "returned false.");
  memset(eval_status, 0, sizeof(eval_status));
  if (!mah_get_eval_status(eval_status, (int)sizeof(eval_status)))
    return fail("ffi_api_smoke failed: mah_get_eval_status failed after "
                "invalid eval mode clamp.");
  if (strcmp(eval_status, "classic_ready") != 0)
    return fail("ffi_api_smoke failed: invalid eval mode did not clamp back to "
                "classic.");

  remove("ffi_api_smoke_weights.nnue");

  if (!mah_generate_custom_position_fen(-1, 7U, generated_fen,
                                        (int)sizeof(generated_fen)))
    return fail("ffi_api_smoke failed: mah_generate_custom_position_fen "
                "returned false.");

  if (generated_fen[0] == '\0')
    return fail("ffi_api_smoke failed: generated custom FEN was empty.");

  if (!respects_row_progression(generated_fen))
    return fail("ffi_api_smoke failed: generated custom FEN violated the "
                "full-rank progression rule.");

  if (!mah_set_position_fen(generated_fen))
    return fail(
        "ffi_api_smoke failed: generated custom FEN could not be loaded.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: generated custom position produced no "
                "best move.");

  if (!mah_set_position_fen("4k3/8/8/4M3/8/8/8/4K3 w - - 0 1 "))
    return fail("ffi_api_smoke failed: king-capture FEN could not be set.");

  if (mah_apply_move("e5e8"))
    return fail(
        "ffi_api_smoke failed: move that captures the king was accepted.");

  memset(out_move, 0, sizeof(out_move));
  if (!mah_best_move_depth(1, out_move, (int)sizeof(out_move)))
    return fail("ffi_api_smoke failed: search failed on a position with a "
                "pseudo-legal king capture.");

  if (mah_generate_custom_position_fen(0, 7U, generated_fen, 8))
    return fail("ffi_api_smoke failed: mah_generate_custom_position_fen "
                "accepted an undersized output buffer.");

  if (!mah_shutdown()) {
    fprintf(stderr, "ffi_api_smoke failed: mah_shutdown returned false.\n");
    return 1;
  }

  return 0;
}
