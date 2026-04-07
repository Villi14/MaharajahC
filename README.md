# MaharajahC

C chess engine with UCI support and perft tooling.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/Maharajah
```

## Tests

```bash
ctest --test-dir build
```

## Usage
1. mah_init()
2. mah_set_position_startpos() або mah_set_position_fen(...)
3. (optional) mah_set_hash_mb(mb)
4. (loop) mah_apply_move(...) і mah_best_move_depth(...) / mah_best_move_time(...)
5. mah_shutdown() 