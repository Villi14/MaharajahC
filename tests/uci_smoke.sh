#!/usr/bin/env sh
set -eu

engine_path="$1"
output="$(printf 'uci\nisready\nposition startpos\ngo depth 1\nquit\n' | "$engine_path")"
stalemate_output="$(printf 'uci\nisready\nposition fen k7/2Q5/1K6/8/8/8/8/8 b - - 0 1\ngo depth 1\nquit\n' | "$engine_path")"

printf '%s\n' "$output" | grep -q '^id name Maharajah '
printf '%s\n' "$output" | grep -q '^id author '
printf '%s\n' "$output" | grep -q '^uciok$'
printf '%s\n' "$output" | grep -q '^readyok$'
printf '%s\n' "$output" | grep -q '^bestmove '
printf '%s\n' "$stalemate_output" | grep -q '^bestmove (none)$'

if printf '%s\n' "$output" | grep -q 'Hash table is initialed'; then
  echo "uci_smoke failed: unexpected non-UCI log line in stdout" >&2
  exit 1
fi

if printf '%s\n' "$stalemate_output" | grep -q 'Hash table is initialed'; then
  echo "uci_smoke failed: unexpected non-UCI log line in stalemate stdout" >&2
  exit 1
fi

if printf '%s\n' "$output" | grep -q '^bestmove a8a8$'; then
  echo "uci_smoke failed: invalid null bestmove from non-terminal position" >&2
  exit 1
fi
