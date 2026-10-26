#!/bin/sh
# Packs build/pvz-portable into "PvZ Portable.app": the executable with its dylibs (dylibbundler, plus
# SDL3, which Homebrew's sdl2-compat loads with dlopen()), an icon, Info.plist and a small launcher that
# passes -resdir, because inside a bundle the executable's directory is Contents/MacOS.
#
# Usage: scripts/make-macos-app.sh [OUTPUT_DIR] [GAME_DATA_DIR]
#   OUTPUT_DIR     where the .app goes (default: dist)
#   GAME_DATA_DIR  optional folder with main.pak and properties/, copied into the app's Resources.
#                  Without it, put main.pak and properties/ next to the .app.
# Saves and settings stay in the usual per-user folder,
# ~/Library/Application Support/io.github.wszqkzqk/PvZPortable/.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/dist}"
DATA_DIR="$2"
APP="$OUT_DIR/PvZ Portable.app"
VERSION="$(git -C "$ROOT" describe --tags --abbrev=0 2>/dev/null || echo 0)"

[ -x "$ROOT/build/pvz-portable" ] || { echo "Build first: cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build" >&2; exit 1; }
if [ -n "$DATA_DIR" ] && { [ ! -e "$DATA_DIR/main.pak" ] || [ ! -d "$DATA_DIR/properties" ]; }; then
	echo "$DATA_DIR has no main.pak and properties/" >&2
	exit 1
fi

rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS/libs" "$APP/Contents/Resources"

cp "$ROOT/build/pvz-portable" "$APP/Contents/MacOS/"
strip -x "$APP/Contents/MacOS/pvz-portable"
dylibbundler -of -b -x "$APP/Contents/MacOS/pvz-portable" -d "$APP/Contents/MacOS/libs" -p @executable_path/libs/ >/dev/null
install -m 755 "$(brew --prefix sdl3)/lib/libSDL3.dylib" "$APP/Contents/MacOS/libs/libSDL3.dylib"

if [ -n "$DATA_DIR" ]; then
	cp -R "$DATA_DIR/main.pak" "$DATA_DIR/properties" "$APP/Contents/Resources/"
fi

cat > "$APP/Contents/MacOS/PvZ Portable" <<'EOF'
#!/bin/sh
# Game data inside the app if it was packed with it, otherwise next to the app.
DIR="$(cd "$(dirname "$0")" && pwd)"
if [ -e "$DIR/../Resources/main.pak" ]; then
	RES="$(cd "$DIR/../Resources" && pwd)"
else
	RES="$(cd "$DIR/../../.." && pwd)"
fi
exec "$DIR/pvz-portable" -resdir="$RES" "$@"
EOF
chmod 755 "$APP/Contents/MacOS/PvZ Portable"

ICONSET="$(mktemp -d)/AppIcon.iconset"
mkdir -p "$ICONSET"
for s in 16 32 128 256 512; do
	sips -z $s $s "$ROOT/icon.png" --out "$ICONSET/icon_${s}x${s}.png" >/dev/null
	sips -z $((s * 2)) $((s * 2)) "$ROOT/icon.png" --out "$ICONSET/icon_${s}x${s}@2x.png" >/dev/null
done
iconutil -c icns "$ICONSET" -o "$APP/Contents/Resources/AppIcon.icns"
rm -rf "$(dirname "$ICONSET")"

cat > "$APP/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleName</key><string>PvZ Portable</string>
	<key>CFBundleDisplayName</key><string>PvZ Portable</string>
	<key>CFBundleIdentifier</key><string>io.github.wszqkzqk.pvzportable</string>
	<key>CFBundleExecutable</key><string>PvZ Portable</string>
	<key>CFBundleIconFile</key><string>AppIcon</string>
	<key>CFBundlePackageType</key><string>APPL</string>
	<key>CFBundleShortVersionString</key><string>$VERSION</string>
	<key>CFBundleVersion</key><string>1</string>
	<key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
EOF

codesign --force --deep -s - "$APP" >/dev/null 2>&1
codesign --verify --deep "$APP"
echo "$APP"
