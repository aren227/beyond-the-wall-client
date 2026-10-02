#!/usr/bin/env bash
set -euo pipefail

PROJECT_NAME="beyond-the-wall"

BUILD_TYPE="${1:-release}"
case "$BUILD_TYPE" in
    debug|release) ;;
    *) echo "Usage: $0 [debug|release]" >&2; exit 1 ;;
esac

PRESET="standalone-$BUILD_TYPE"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="$REPO_ROOT/build/$PRESET"
DIST_DIR="$REPO_ROOT/dist"

cd "$REPO_ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET"

mkdir -p "$DIST_DIR"
case "$(uname -s)" in
    Darwin) PLATFORM=macos ;;
    Linux)  PLATFORM=linux ;;
    *)      PLATFORM=unknown ;;
esac

ZIP_PATH="$DIST_DIR/build-$PLATFORM-$BUILD_TYPE.zip"
rm -f "$ZIP_PATH"
(cd "$OUT_DIR" && zip -r "$ZIP_PATH" $PROJECT_NAME resources)

echo
echo "Standalone $BUILD_TYPE build ready: $ZIP_PATH"
