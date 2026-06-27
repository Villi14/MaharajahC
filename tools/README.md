# maharajah_tool

Small native CLI for Maharajah custom-position generation and engine self-play.

## Build

From the plugin root:

```bash
cmake -S src -B src/build
cmake --build src/build --target maharajah_tool
```

## Commands

Generate custom start positions:

```bash
./src/build/maharajah_tool generate [count] [seed]
```

Run self-play from generated custom starts:

```bash
./src/build/maharajah_tool selfplay [games] [depth] [max_plies] [seed] [json_path]
```

Run the small built-in benchmark suite:

```bash
./src/build/maharajah_tool bench
```

## Notes

- The generator enforces the current custom-setup constraints used by the native engine.
- It uses lightweight heuristics so generated armies are more reasonable than pure random fills.
- `json_path` is optional. When present, self-play writes a JSON array of played games with start FEN, result, end FEN, and move list.
- `bench` is a practical single-thread baseline check for MVP work. It prints best move, nodes, and time for a fixed suite of positions.
