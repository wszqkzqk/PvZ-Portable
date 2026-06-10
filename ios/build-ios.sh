#!/bin/bash
# Build PvZ-Portable for iOS.
# Usage: ./ios/build-ios.sh [Debug|Release]
# Requires: Xcode 15+ with iOS SDK, CMake 3.21+, vcpkg
# VCPKG_ROOT must be set to the vcpkg installation directory.

set -euo pipefail

BUILD_TYPE="${1:-Release}"
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-ios"

echo "=== PvZ-Portable iOS Build ($BUILD_TYPE) ==="

if [ -z "${VCPKG_ROOT:-}" ]; then
    echo "Error: VCPKG_ROOT is not set. Install vcpkg and set VCPKG_ROOT."
    exit 1
fi

IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null || true)
if [ -z "$IOS_SDK" ]; then
    echo "Error: iOS SDK not found. Install Xcode and run: xcode-select --install"
    exit 1
fi

mkdir -p "$BUILD_DIR"

# Build game (vcpkg handles all dependencies via manifest mode)
echo "--- Building PvZ-Portable ---"
cmake -B "$BUILD_DIR/game" -S "$PROJECT_ROOT" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=arm64-ios \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -G Xcode

cmake --build "$BUILD_DIR/game" --config "$BUILD_TYPE" -- \
    -sdk iphoneos \
    CODE_SIGN_IDENTITY="-" \
    CODE_SIGNING_ALLOWED=NO

# Create unsigned IPA
APP_PATH=$(find "$BUILD_DIR/game" -name "pvz-portable.app" -path "*${BUILD_TYPE}*" | head -1)
if [ -z "$APP_PATH" ]; then
    echo "Warning: .app bundle not found, skipping IPA creation"
else
    IPA_DIR="$BUILD_DIR/ipa"
    mkdir -p "$IPA_DIR/Payload"
    cp -R "$APP_PATH" "$IPA_DIR/Payload/"
    cd "$IPA_DIR"
    zip -r -y "$BUILD_DIR/pvz-portable-ios.ipa" Payload/
    rm -rf "$IPA_DIR"
    echo "IPA created: $BUILD_DIR/pvz-portable-ios.ipa"
fi

echo "=== iOS Build Complete ==="
