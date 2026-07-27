#!/bin/bash
# Build PvZ-Portable for WebAssembly using Emscripten
#
# Prerequisites:
#   - Emscripten SDK (emsdk) installed and activated
#   - libopenmpt must be pre-built for Emscripten (no Emscripten port exists);
#     other audio codecs (ogg, vorbis, mpg123) are provided via Emscripten ports.
#
# Usage:
#   source /path/to/emsdk/emsdk_env.sh
#   ./wasm/build-wasm.sh            # assumes libopenmpt already installed
#   ./wasm/build-wasm.sh --deps     # also builds and installs libopenmpt
#
# Output:
#   build-wasm/pvz-portable.html  — the main page
#   build-wasm/pvz-portable.js    — Emscripten glue
#   build-wasm/pvz-portable.wasm  — WebAssembly binary
#
# To serve locally:
#   python3 -m http.server 8080 --directory build-wasm
#   Then open http://localhost:8080/pvz-portable.html

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-wasm"

if ! command -v emcmake &> /dev/null; then
    echo "Error: emcmake not found. Please activate the Emscripten SDK first:"
    echo "  source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

NPROC="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# Local prefix for libraries built from source (avoids polluting the sysroot)
OPENMPT_PREFIX="${BUILD_DIR}/openmpt-prefix"

# Build libopenmpt if --deps is passed
if [[ "${1:-}" == "--deps" ]]; then
    OPENMPT_SRC="${BUILD_DIR}/libopenmpt-src"

    if [ ! -f "${OPENMPT_PREFIX}/lib/libopenmpt.a" ]; then
        echo "Building libopenmpt for Emscripten..."
        OPENMPT_TAG=$(git ls-remote --tags https://github.com/OpenMPT/openmpt.git 'refs/tags/libopenmpt-*' \
            | grep -v '\^{}' | sed 's|.*refs/tags/libopenmpt-||' \
            | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | sort -V | tail -1)
        if [ -z "$OPENMPT_TAG" ]; then
            OPENMPT_TAG="0.8.4" # Fallback to a known stable version if tag retrieval fails
        fi
        echo "Using libopenmpt version: $OPENMPT_TAG"
        if [ ! -d "${OPENMPT_SRC}" ]; then
            git clone --depth=1 --branch "libopenmpt-${OPENMPT_TAG}" https://github.com/OpenMPT/openmpt.git "${OPENMPT_SRC}"
        fi
        cd "${OPENMPT_SRC}"
        emmake make CONFIG=emscripten \
            STATIC_LIB=1 SHARED_LIB=0 \
            EXAMPLES=0 OPENMPT123=0 TEST=0 \
            NO_ZLIB=1 NO_MPG123=1 NO_OGG=1 NO_VORBIS=1 NO_VORBISFILE=1 \
            NO_PORTAUDIO=1 NO_PORTAUDIOCPP=1 NO_PULSEAUDIO=1 \
            NO_SDL2=1 NO_FLAC=1 NO_SNDFILE=1 \
            -j"${NPROC}"
        emmake make CONFIG=emscripten \
            STATIC_LIB=1 SHARED_LIB=0 \
            EXAMPLES=0 OPENMPT123=0 TEST=0 \
            PREFIX="${OPENMPT_PREFIX}" install
        echo "libopenmpt installed to ${OPENMPT_PREFIX}"
    else
        echo "libopenmpt already built, skipping."
    fi
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

emcmake cmake "${PROJECT_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_FIND_ROOT_PATH="${OPENMPT_PREFIX}" \
    -G Ninja

cmake --build . -j"${NPROC}"

echo ""
echo "Build complete! Output files:"
echo "  ${BUILD_DIR}/pvz-portable.html"
echo "  ${BUILD_DIR}/pvz-portable.js"
echo "  ${BUILD_DIR}/pvz-portable.wasm"
echo ""
echo "To test locally:"
echo "  python3 -m http.server 8080 --directory ${BUILD_DIR}"
echo "  Then open http://localhost:8080/pvz-portable.html"
