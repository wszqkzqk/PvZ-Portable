#!/usr/bin/env bash
# Prepares native dependencies for the HarmonyOS build in harmony/deps/:
#   - SDL3 (upstream, with its HarmonyOS backend) and sdl2-compat sources, built by the HAP's CMake;
#   - libpng, libjpeg-turbo and libopenmpt, cross-compiled here as static libraries per ABI.
#
# Usage: harmony/build-deps.sh [abi...]   (default: arm64-v8a; x86_64 is the emulator ABI)
#
# Needs the OpenHarmony native SDK (from DevEco Studio or the command line tools). Set
# OHOS_NATIVE_HOME to its "native" directory if it isn't found automatically.

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
DEPS_DIR="$SCRIPT_DIR/deps"
SRC_DIR="$DEPS_DIR/src"
WORK_DIR="$DEPS_DIR/work"

# Follow upstream main by default; set explicit commits only when reproducing a build or debugging.
SDL3_COMMIT=${SDL3_COMMIT:-main}
SDL2_COMPAT_COMMIT=${SDL2_COMPAT_COMMIT:-main}
OPENMPT_VERSION=${OPENMPT_VERSION:-0.8.4}
LIBPNG_VERSION=${LIBPNG_VERSION:-1.6.58}
JPEG_TURBO_VERSION=${JPEG_TURBO_VERSION:-3.1.3}

# The lowest SDK level SDL3 supports (HarmonyOS 5.0.4, API 16).
OHOS_SDK_VERSION=${OHOS_SDK_VERSION:-16.0.0}

find_native_sdk() {
	local candidates=(
		"${OHOS_NATIVE_HOME:-}"
		"${OHOS_SDK_HOME:+$OHOS_SDK_HOME/native}"
		"${DEVECO_SDK_HOME:+$DEVECO_SDK_HOME/default/openharmony/native}"
		"/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native"
		"$HOME/command-line-tools/sdk/default/openharmony/native"
		"$HOME/HarmonyOS-SDK/command-line-tools/sdk/default/openharmony/native"
	)
	local dir
	for dir in "${candidates[@]}"; do
		if [ -n "$dir" ] && [ -f "$dir/build/cmake/ohos.toolchain.cmake" ]; then
			echo "$dir"
			return 0
		fi
	done
	return 1
}

OHOS_NATIVE=$(find_native_sdk) || {
	echo "error: OpenHarmony native SDK not found; set OHOS_NATIVE_HOME to <sdk>/default/openharmony/native" >&2
	exit 1
}
echo "Using OpenHarmony native SDK: $OHOS_NATIVE"

# Stock CMake doesn't know the OHOS platform; use the one bundled with the SDK.
CMAKE="$OHOS_NATIVE/build-tools/cmake/bin/cmake"
NINJA="$OHOS_NATIVE/build-tools/cmake/bin/ninja"
TOOLCHAIN="$OHOS_NATIVE/build/cmake/ohos.toolchain.cmake"
LLVM_BIN="$OHOS_NATIVE/llvm/bin"
SYSROOT="$OHOS_NATIVE/sysroot"

if command -v nproc >/dev/null 2>&1; then
	JOBS=$(nproc)
else
	JOBS=$(sysctl -n hw.ncpu)
fi

fetch_git_commit() {
	local url=$1 commit=$2 dest=$3
	if [ -d "$dest/.git" ]; then
		if [ "$(git -C "$dest" rev-parse HEAD)" = "$commit" ]; then
			git -C "$dest" reset --hard -q "$commit"
		else
			# Branch names such as main must be fetched again to discover new upstream commits.
			git -C "$dest" fetch -q --depth 1 "$url" "$commit"
			git -C "$dest" reset --hard -q FETCH_HEAD
		fi
		git -C "$dest" clean -fdq
		return 0
	fi
	rm -rf "$dest"
	git init -q "$dest"
	git -C "$dest" fetch -q --depth 1 "$url" "$commit"
	git -C "$dest" checkout -q FETCH_HEAD
}

fetch_tarball() {
	local url=$1 dest=$2
	local name
	name=$(basename "$dest")
	mkdir -p "$WORK_DIR"
	local marker="$WORK_DIR/$name.source-url"
	if [ -d "$dest" ] && [ -f "$marker" ] && [ "$(cat "$marker")" = "$url" ]; then
		return 0
	fi

	local archive="$WORK_DIR/$(basename "$url")"
	local extracted="$WORK_DIR/$name.extract"
	rm -rf "$extracted"
	curl -fL --retry 3 -o "$archive" "$url"
	mkdir -p "$extracted"
	tar xf "$archive" -C "$extracted" --strip-components=1
	rm -rf "$dest"
	mv "$extracted" "$dest"
	printf '%s\n' "$url" > "$marker"
}

mkdir -p "$SRC_DIR"

echo "==> Fetching SDL3 ($SDL3_COMMIT) and sdl2-compat ($SDL2_COMPAT_COMMIT)"
fetch_git_commit https://github.com/libsdl-org/SDL.git "$SDL3_COMMIT" "$SRC_DIR/SDL"
fetch_git_commit https://github.com/libsdl-org/sdl2-compat.git "$SDL2_COMPAT_COMMIT" "$SRC_DIR/sdl2-compat"

echo "==> Fetching libpng $LIBPNG_VERSION, libjpeg-turbo $JPEG_TURBO_VERSION, libopenmpt $OPENMPT_VERSION"
fetch_tarball "https://github.com/pnggroup/libpng/archive/refs/tags/v${LIBPNG_VERSION}.tar.gz" "$SRC_DIR/libpng"
fetch_tarball "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/${JPEG_TURBO_VERSION}.tar.gz" "$SRC_DIR/libjpeg-turbo"
# The autotools tarball cross-compiles cleanly; the upstream Makefile assumes a native host.
fetch_tarball "https://lib.openmpt.org/files/libopenmpt/src/libopenmpt-${OPENMPT_VERSION}+release.autotools.tar.gz" "$SRC_DIR/libopenmpt"

cmake_dep() {
	local abi=$1 name=$2 prefix=$3
	shift 3
	"$CMAKE" -G Ninja -DCMAKE_MAKE_PROGRAM="$NINJA" \
		-B "$WORK_DIR/$abi/$name" -S "$SRC_DIR/$name" \
		-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
		-DOHOS_ARCH="$abi" \
		-DOHOS_COMPATIBLE_SDK_VERSION="$OHOS_SDK_VERSION" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$prefix" \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		"$@"
	"$CMAKE" --build "$WORK_DIR/$abi/$name" -j"$JOBS"
	"$CMAKE" --install "$WORK_DIR/$abi/$name"
}

build_abi() {
	local abi=$1 triple
	case "$abi" in
		arm64-v8a) triple=aarch64-linux-ohos ;;
		x86_64) triple=x86_64-linux-ohos ;;
		*) echo "error: unsupported ABI $abi" >&2; exit 1 ;;
	esac
	local prefix="$DEPS_DIR/$abi"

	echo "==> [$abi] libpng"
	# libpng runs the preprocessor by hand to generate pnglibconf.h, which drops the toolchain's
	# CMAKE_C_COMPILER_TARGET, so repeat the target in the flags.
	cmake_dep "$abi" libpng "$prefix" \
		-DCMAKE_C_FLAGS="--target=$triple" \
		-DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_EXECUTABLES=OFF -DPNG_TESTS=OFF

	echo "==> [$abi] libjpeg-turbo"
	cmake_dep "$abi" libjpeg-turbo "$prefix" \
		-DENABLE_SHARED=OFF -DENABLE_STATIC=ON -DWITH_TURBOJPEG=OFF

	echo "==> [$abi] libopenmpt"
	local build="$WORK_DIR/$abi/libopenmpt"
	rm -rf "$build"
	mkdir -p "$build"
	# config.sub may not know "ohos", so name a musl host; the compiler flags carry the real target.
	local host=${triple%-ohos}-musl
	(
		cd "$build"
		CC="$LLVM_BIN/clang --target=$triple --sysroot=$SYSROOT" \
		CXX="$LLVM_BIN/clang++ --target=$triple --sysroot=$SYSROOT" \
		AR="$LLVM_BIN/llvm-ar" \
		RANLIB="$LLVM_BIN/llvm-ranlib" \
		CFLAGS="-O2 -fPIC" \
		CXXFLAGS="-O2 -fPIC" \
		PKG_CONFIG_LIBDIR="" \
		"$SRC_DIR/libopenmpt/configure" \
			--host="$host" \
			--prefix="$prefix" \
			--enable-static --disable-shared \
			--disable-openmpt123 --disable-examples --disable-tests --disable-doxygen-doc \
			--without-zlib --without-mpg123 --without-ogg --without-vorbis --without-vorbisfile \
			--without-portaudio --without-portaudiocpp --without-pulseaudio \
			--without-sndfile --without-flac --without-sdl2
		make -j"$JOBS"
		make install
	)
}

if [ $# -eq 0 ]; then
	set -- arm64-v8a
fi
for abi in "$@"; do
	build_abi "$abi"
done

echo "Done. Dependencies are in $DEPS_DIR"
