# Tools

- [`ask_engine.py`](#ask_enginepy) — ask the engine for its best move in any position.
- [`maharajah_tool`](#maharajah_tool) — custom-position generation and engine self-play.

# ask_engine.py

Drives the UCI binary through a single search and prints what it would play.
Useful for checking a move an app made against what the engine actually chose.

```bash
python3 tools/ask_engine.py "<fen>" --level 5
python3 tools/ask_engine.py --startpos --level 3
python3 tools/ask_engine.py "<fen>" --skill 10 --movetime 30000 --raw
```

`--level` is the difficulty as shown in the app (1-5) and maps to skill level
and thinking time the way the app does (`ui_to_skill_level` in
`EngineConfig.c`). `--skill` and `--movetime` override it. The engine binary is
auto-detected across build layouts — the freshest build wins — or name it with
`--engine PATH` or `$MAHARAJAH_ENGINE`. Needs only Python 3, no packages.

It feeds UCI commands with deliberate pauses. Piping them all at once makes
`input_waiting()` see the unread input, abort the search and answer
`bestmove (none)`, which looks like a broken engine but is not.

# maharajah_tool

Small native CLI for Maharajah custom-position generation and engine self-play.

## Build

The target is part of the normal build:

```bash
cmake -S . -B build
cmake --build build --target maharajah_tool
```

## Commands

Generate custom start positions:

```bash
./build/maharajah_tool generate [count] [seed]
```

Run self-play from generated custom starts:

```bash
./build/maharajah_tool selfplay [games] [depth] [max_plies] [seed] [json_path]
```

Run the small built-in benchmark suite:

```bash
./build/maharajah_tool bench
```

## Notes

- The generator enforces the current custom-setup constraints used by the native engine.
- It uses lightweight heuristics so generated armies are more reasonable than pure random fills.
- `json_path` is optional. When present, self-play writes a JSON array of played games with start FEN, result, end FEN, and move list.
- `bench` is a practical single-thread baseline check for MVP work. It prints best move, nodes, and time for a fixed suite of positions.
