# MaharajahC

C chess engine with UCI support, an FFI boundary, and perft tooling.

Its distinguishing feature is support for **compound (fairy) pieces** throughout the
engine — move generation, SEE, evaluation, and draw rules:

- `A` / `a` — Archbishop (bishop + knight)
- `C` / `c` — Chancellor (rook + knight)
- `M` / `m` — Amazon / "Maharajah" (queen + knight)

## Credits

The engine is based on **BBC (Bitboard Chess Engine)** by Maksim Korzh,
specifically the reference version
[`bbc_1.2.c`](https://github.com/maksimKorzh/bbc/blob/master/src/old_versions/bbc_1.2.c).

The bit-packed move encoding, `copy_board()` / `take_back()` macros, magic
bitboards, MVV-LVA / killer / history ordering, PVS, null-move pruning, LMR,
and the aspiration window all originate there. MaharajahC extends that base
with compound pieces, a centralized `EngineConfig`, difficulty/skill layers,
an FFI surface, and an NNUE scaffold.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/Maharajah
```

## Check a position yourself

Wondering whether a move the app played was really the engine's idea? Ask it
directly. `--level` is the difficulty as shown in the app (1-5), and it sets
both the skill level and the thinking time exactly the way the app does:

```bash
python3 tools/ask_engine.py "2b3k1/2p1r1p1/2p2p1p/p1Pp4/8/1PP4P/P1N2PP1/2K1R3 b - - 0 26" --level 5
```

```
Position : 2b3k1/2p1r1p1/2p2p1p/p1Pp4/8/1PP4P/P1N2PP1/2K1R3 b - - 0 26
To move  : Black
Strength : level 5  ->  skill 10, 5000 ms of thinking time

  depth   1        +0.22   e7e1
  ...
  depth  12        -0.23   e7e1   c2e1 c8d7 e1f3 g8f7 f3d4 h6h5

Best move: e7e1   (e7 - e1)
```

Every line is one completed search iteration, so you can watch the engine
change or keep its mind as it looks deeper. `--startpos` uses the normal
starting position, `--moves "e2e4 e7e5"` plays moves out from the FEN, and
`--skill` / `--movetime` set strength and time directly instead of via
`--level`. Run with `--help` for the rest.

Variant rules come from the FEN itself: an optional 7th field carries per-side
variant rights (`V` for White, `v` for Black, `-` for neither), so
`... 0 1 Vv` asks the engine to think under variant rules for both sides.
Without that field the rules are derived from the material on the board.

## Tests

```bash
ctest --test-dir build
```

These include, among others: `perft_smoke`, `see_smoke` (SEE on representative
captures), `search_sanity_smoke`, `engine_config_smoke`, `draw_rules_smoke`,
`transposition_smoke`, and `special_moves_smoke`.

## Search / SEE notes

- Quiescence can prune clearly bad exchanges (SEE + stand-pat); aggressiveness
  is set via `quiescence_see_prune_margin` in `EngineSearchConfig`
  (tune in skill profiles / `reset_engine_config`).
- SEE implementation: `src/engine/See.c`, covering all standard and compound
  piece types.

## Usage (FFI)

1. `mah_init()`
2. `mah_set_position_startpos()` or `mah_set_position_fen(...)`
3. (optional) `mah_set_hash_mb(mb)`
4. (optional) `mah_set_difficulty_level(level)` // 1..5 for UI
5. (optional) `mah_set_skill_level(level)` // 1..10 for engine tuning
6. (optional) `mah_get_eval_status(...)`
7. (loop) `mah_apply_move(...)` and `mah_best_move_depth(...)` / `mah_best_move_time(...)`
8. `mah_shutdown()`

## API boundary

- Stable app-facing API: `mah_init`, `mah_set_position_*`, `mah_apply_move`,
  `mah_best_move_*`, `mah_generate_custom_position_fen`, `mah_set_difficulty_level`
- Lower-level tuning API: `mah_set_skill_level`, `mah_set_hash_mb`
- Stable offline API for `0.1.0`: `mah_init`, position setters, move application,
  search, difficulty, hash, `mah_get_eval_status`

For client applications the rules are simple:

- use `difficulty 1..5` for normal play
- do **not** treat `skill 1..10` as a stable product contract

## Architecture notes

Current engine direction:

- `difficulty 1..5` is the stable level for the UI
- `skill 1..10` remains the lower layer for experiments and tuning
- search and eval configuration are centralized in `EngineConfig`

In practice that means:

- strength-related parameters should not sprawl across `Search.c` and `Evaluate.c`
- prefer new search heuristics through the search profile
- prefer new eval coefficients through the eval config
- engine-level `Threads` are planned later, after the single-thread baseline is
  stable and measured

## 0.1.0 direction

`0.1.0` intentionally ships as a fully offline classic-engine release. Future
NNUE work can stay in the codebase as scaffolding, but it is not part of the
production API or gameplay contract for this version:

- `evaluate()` consistently routes to the classic path
- `evaluate_classic()` is the production path for `0.1.0`
- `mah_get_eval_status(...) == "classic_ready"` is a normal runtime status for
  release builds

## Roadmap / known gaps

Planned work, correctness priorities, and known gaps live in
[CHANGELOG.md](CHANGELOG.md) under `[Unreleased]`.

> Note: `build/` and `.DS_Store` are already covered by `.gitignore` and are
> not tracked.

## Regression intake

For `0.1.0` the practical way to improve quality is to collect local blunder
reports (FEN, move list, short note) and turn them into smoke/regression tests.

## License

Released under the [MIT License](LICENSE). The engine derives from the
WTFPL-licensed BBC `bbc_1.2.c` (see [Credits](#credits)); the WTFPL places no
restrictions on redistribution, so the derived work is distributed here under
the MIT License.
</content>
</invoke>
