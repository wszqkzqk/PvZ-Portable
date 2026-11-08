#!/bin/bash
# Build PvZ-Portable for WebAssembly using Emscripten.
#
# Usage:
#   source /path/to/emsdk/emsdk_env.sh
#   ./wasm/build-wasm.sh            # use dependencies already in build-wasm
#   ./wasm/build-wasm.sh --deps     # also build libopenmpt and fetch SDL3_mixer
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
OPENMPT_PREFIX="${BUILD_DIR}/openmpt-prefix"
MIXER_SRC="${BUILD_DIR}/SDL_mixer-src"
SYSROOT="$(em-config EMSCRIPTEN_ROOT)/cache/sysroot"

if [[ "${1:-}" != "--deps" && ! -d "${MIXER_SRC}" ]]; then
    echo "SDL3_mixer sources are missing. Run ./wasm/build-wasm.sh --deps first." >&2
    exit 1
fi

embuilder build sdl3 vorbis

if [[ "${1:-}" == "--deps" ]]; then
    if [ ! -f "${OPENMPT_PREFIX}/lib/libopenmpt.a" ]; then
        echo "Building libopenmpt for Emscripten..."
        OPENMPT_SRC="${BUILD_DIR}/libopenmpt-src"
        OPENMPT_TAG=$(git ls-remote --tags https://github.com/OpenMPT/openmpt.git 'refs/tags/libopenmpt-*' \
            | grep -v '\^{}' | sed 's|.*refs/tags/libopenmpt-||' \
            | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | sort -V | tail -1)
        if [ -z "$OPENMPT_TAG" ]; then
            OPENMPT_TAG="0.8.4"
        fi
        echo "Using libopenmpt version: $OPENMPT_TAG"
        if [ ! -d "${OPENMPT_SRC}" ]; then
            git clone --depth=1 --branch "libopenmpt-${OPENMPT_TAG}" https://github.com/OpenMPT/openmpt.git "${OPENMPT_SRC}"
        fi
        cd "${OPENMPT_SRC}"
        emmake make CONFIG=emscripten EMSCRIPTEN_PORTS=1 \
            STATIC_LIB=1 SHARED_LIB=0 \
            EXAMPLES=0 OPENMPT123=0 TEST=0 \
            NO_PORTAUDIO=1 NO_PORTAUDIOCPP=1 NO_PULSEAUDIO=1 \
            NO_SDL2=1 NO_FLAC=1 NO_SNDFILE=1 \
            -j"${NPROC}"
        emmake make CONFIG=emscripten EMSCRIPTEN_PORTS=1 \
            STATIC_LIB=1 SHARED_LIB=0 \
            EXAMPLES=0 OPENMPT123=0 TEST=0 \
            PREFIX="${OPENMPT_PREFIX}" install
    fi

    if [ ! -d "${MIXER_SRC}" ]; then
        echo "Fetching SDL3_mixer for Emscripten..."
        MIXER_TAG=$(git ls-remote --tags https://github.com/libsdl-org/SDL_mixer.git 'refs/tags/release-*' \
            | sed 's|.*refs/tags/release-||' | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$' | sort -V | tail -1)
        if [ -z "$MIXER_TAG" ]; then
            MIXER_TAG="3.2.4"
        fi
        echo "Using SDL3_mixer version: $MIXER_TAG"
        git clone --depth=1 --branch "release-${MIXER_TAG}" https://github.com/libsdl-org/SDL_mixer.git "${MIXER_SRC}"
    fi
fi

emcmake cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_FIND_ROOT_PATH="${OPENMPT_PREFIX};${SYSROOT}" \
    -DPVZP_SDL3_MIXER_SOURCE_DIR="${MIXER_SRC}" \
    -DSDLMIXER_VENDORED=OFF \
    -DSDLMIXER_TESTS=OFF \
    -DSDLMIXER_EXAMPLES=OFF \
    -DSDLMIXER_AIFF=OFF -DSDLMIXER_VOC=OFF \
    -DSDLMIXER_FLAC=OFF -DSDLMIXER_GME=OFF \
    -DSDLMIXER_MOD=OFF -DSDLMIXER_MIDI=OFF \
    -DSDLMIXER_OPUS=OFF -DSDLMIXER_WAVPACK=OFF \
    -DSDLMIXER_MP3_MPG123=OFF \
    -DSDLMIXER_VORBIS_STB=ON \
    -DSDLMIXER_VORBIS_VORBISFILE=OFF

cmake --build "${BUILD_DIR}" -j"${NPROC}"

echo ""
echo "Build complete! Output files:"
echo "  ${BUILD_DIR}/pvz-portable.html"
echo "  ${BUILD_DIR}/pvz-portable.js"
echo "  ${BUILD_DIR}/pvz-portable.wasm"
echo ""
echo "To test locally:"
echo "  python3 -m http.server 8080 --directory ${BUILD_DIR}"
echo "  Then open http://localhost:8080/pvz-portable.html"
