#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-release}"
case "$BUILD_TYPE" in
    debug|release) ;;
    *) echo "Usage: $0 [debug|release]" >&2; exit 1 ;;
esac

if [ -z "${EMSDK:-}" ]; then
    if [ -d "$HOME/emsdk" ]; then
        EMSDK="$HOME/emsdk"
    else
        echo "EMSDK not set and ~/emsdk not found. Install emsdk or set EMSDK env var." >&2
        exit 1
    fi
fi
# shellcheck disable=SC1091
source "$EMSDK/emsdk_env.sh" > /dev/null

PRESET="web-$BUILD_TYPE"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="$REPO_ROOT/build/$PRESET"
DIST_DIR="$REPO_ROOT/dist"

cd "$REPO_ROOT"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET"

mkdir -p "$DIST_DIR"
ZIP_PATH="$DIST_DIR/build-web-$BUILD_TYPE.zip"
rm -f "$ZIP_PATH"
(cd "$OUT_DIR" && zip -j "$ZIP_PATH" index.html index.js index.wasm index.data)

echo
echo "Web $BUILD_TYPE build ready: $ZIP_PATH"
