# Changelog

All notable changes to MaharajahC are documented here. This file also tracks
planned work and known gaps that were previously listed in the README.

## [Unreleased]

### Correctness (highest priority for playing strength)

- **Legality model.** Moves are pseudo-legal and legality is inferred from the
  king-capture rule (`own_king == 0` → mate in `negamax`, king captures rejected
  in `make_move`). This degrades mate-score accuracy and pruning at the margins.
  Either restore a real post-move `is_square_attacked` check, or validate mate
  scoring against test positions.
- **Null-move zugzwang guard.** NMP in `negamax` fires on
  `depth >= 3 && !in_check` without checking for non-pawn material. Add a
  zugzwang guard to avoid errors in pawn endgames.

### Tuning / config hygiene

- Move hard-coded search constants (anti-repeat margin `150`, tactical margin
  `120`, etc.) into `EngineConfig`, per the "no sprawl" goal.
- Revisit compound-piece valuation: mobility/PST are currently the linear sum of
  components, which over/under-values the move synergy of A/C/M.

### Future work

- Engine-level `Threads` (after the single-thread baseline is measured).
- NNUE evaluation path (scaffold exists in `src/engine/NNUE.c`).

## [0.1.0]

Initial release. Ships as a fully offline classic-engine release:

- C chess engine with UCI support, an FFI boundary, and perft tooling.
- Support for compound (fairy) pieces throughout move generation, SEE,
  evaluation, and draw rules: Archbishop (`A`/`a`), Chancellor (`C`/`c`),
  Amazon/"Maharajah" (`M`/`m`).
- `evaluate()` consistently routes to the classic path; `evaluate_classic()` is
  the production path.
- Centralized `EngineConfig`, difficulty (`1..5`) and skill (`1..10`) layers,
  and an NNUE scaffold (not part of the production API for this version).
