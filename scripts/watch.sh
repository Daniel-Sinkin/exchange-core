#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
TARGET="${TARGET:-main}"
CXX_COMPILER="${CXX_COMPILER:-/opt/homebrew/opt/llvm/bin/clang++}"
BUILD_TESTING="${BUILD_TESTING:-OFF}"
EXPORT_COMPILE_COMMANDS="${EXPORT_COMPILE_COMMANDS:-ON}"
INPUT_FILE="${INPUT_FILE:-$ROOT_DIR/tests/data/input.csv}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
  -DBUILD_TESTING="$BUILD_TESTING" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS="$EXPORT_COMPILE_COMMANDS"

if [[ "$EXPORT_COMPILE_COMMANDS" == "ON" && -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sfn "$BUILD_DIR/compile_commands.json" "$ROOT_DIR/compile_commands.json"
fi

run_cmd="\"$BUILD_DIR/$TARGET\" < \"$INPUT_FILE\""
cycle_cmd="cmake --build \"$BUILD_DIR\" --target \"$TARGET\" && { [ -t 1 ] && clear || true; } && $run_cmd"

"$SHELL" -lc "$cycle_cmd"

if command -v watchexec >/dev/null 2>&1; then
  exec watchexec \
    --watch "$ROOT_DIR/src" \
    --exts cpp,cxx,cc,h,hpp,hh \
    --shell=none \
    -- "$SHELL" -lc "$cycle_cmd"
fi

if command -v entr >/dev/null 2>&1; then
  while true; do
    find "$ROOT_DIR/src" -type f | entr -d "$SHELL" -lc "$cycle_cmd"
  done
fi

if command -v fswatch >/dev/null 2>&1; then
  fswatch -o "$ROOT_DIR/src" | while read -r _; do
    "$SHELL" -lc "$cycle_cmd"
  done
  exit 0
fi

echo "No watcher found. Install watchexec, entr, or fswatch." >&2
exit 1
