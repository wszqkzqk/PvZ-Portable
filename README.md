# PvZ-Portable

<div align="center">
  <img src="icon-readme.png" alt="PvZ-Portable" width="450">
  <br>
  <a href="https://deepwiki.com/wszqkzqk/PvZ-Portable"><img src="https://deepwiki.com/badge.svg" alt="Ask DeepWiki"></a>
</div>

**PvZ-Portable** is a cross-platform, community-driven C++20 reimplementation of *Plants vs. Zombies: Game of the Year Edition*, built to bring an authentic recreation of the classic game to modern desktop, mobile, console, and web platforms.

📦 **Native Builds:** Prebuilt packages for Windows, Linux, macOS, Android, iOS, and Nintendo Switch are available on the [Releases](https://github.com/wszqkzqk/PvZ-Portable/releases) page.

🌐 **Don't want to install?** [Try the WebAssembly build directly in your browser](https://wszqkzqk.github.io/PvZ-Portable/pvz-portable.html) (you still need your own game data files).

> **Important Notice**
> - **Game Assets Required:** This repository does not contain any copyrighted assets (graphics, audio, or fonts) owned by PopCap Games or Electronic Arts. To play, you must supply `main.pak` and the `properties/` folder from a legally purchased copy of *Plants vs. Zombies: GOTY Edition* ([Steam](https://store.steampowered.com/app/3590/Plants_vs_Zombies_GOTY_Edition/) / [EA](https://www.ea.com/games/plants-vs-zombies/plants-vs-zombies)).
> - **Clean-Room Implementation:** The codebase is a manual reimplementation based on publicly documented community research ([植物大战僵尸吧](https://tieba.baidu.com/f?ie=utf-8&kw=%E6%A4%8D%E7%89%A9%E5%A4%A7%E6%88%98%E5%83%B5%E5%B0%B8), [PvZ Wiki](https://wiki.pvz1.com/doku.php?id=home), and [PvZ Tools](https://pvz.tools/memory/)). The author (`wszqkzqk`) has never reverse-engineered the original binary, and code produced directly from reverse engineering is not accepted.
> - **Non-Commercial & Educational:** This project is developed for educational purposes focusing on cross-platform porting, engine modernization, and multi-architecture compatibility. It is not affiliated with, authorized, or endorsed by PopCap Games or Electronic Arts.

---

## Highlights & Features

### Graphics, Audio & Gameplay
- **SDL3 + OpenGL ES 2.0 Backend:** Uses OpenGL ES 2.0 with automatic fallback to desktop OpenGL 2.1+. Because GLES 2.0 is the common subset supported natively by desktop GPU drivers, mobile chips, WebGL, and consoles, the renderer runs out of the box everywhere without extra wrapper dependencies (though [ANGLE](https://chromium.googlesource.com/angle/angle) can still be used to translate calls to DirectX, Metal, or Vulkan if desired).
- **Resizable Window:** Supports free window resizing and fullscreen scaling, lifting the fixed-resolution limitation of the original release.
- **Authentic MO3 Music & Audio:** Powered by `SDL3_mixer` and `libopenmpt` for accurate playback of the original `.mo3` tracker module soundtrack and sound effects.
- **Unlockable Limbo Page:** Restores access to the hidden mini-games page from the original game. To unlock it, open the Mini-Games, Puzzle, or Survival selection screen and click or tap any blank area 5 times in quick succession.
- **Deterministic Replay System:** Record gameplay sessions into compact `.dmo` files and play them back deterministically for debugging or regression testing.

### Portability & Architecture Support
- **32-bit & 64-bit Multi-Architecture:** Runs natively on `i686`, `x86_64`, `aarch64`, `riscv64`, `loongarch64`, and more.
- **Endian-Independent Design:** Supports both little-endian and big-endian systems, including cross-endian save compatibility (big-endian support is implemented by design, though untested on physical hardware).
- **Modern C++20 Foundation:** Built with `std::thread`, `std::filesystem`, unified internal UTF-8 string handling, and full Unicode file path support across all operating systems.
- **Low-Memory Mode:** Optional `LOW_MEMORY` build mode disables resource pre-caching for RAM-constrained console targets.

### Save & Asset Compatibility
- **Original Profile Compatibility:** Directly reads and writes original PC GOTY global profile files (`users.dat`, `user*.dat` covering adventure progress, coins, Zen Garden, achievements, etc.), while fixing the Year 2038 timestamp overflow bug in a backward-compatible manner.
- **Cross-Platform Mid-Level Saves (`.v4`):** Replaces the original game's fragile raw-memory `.dat` dumps with a structured TLV save format that can be freely transferred across operating systems, compilers, and 32/64-bit CPU architectures, complete with a YAML export/import tool.
- **Multilingual Asset Support:** Works with official GOTY asset packages in English, Chinese, German, Spanish, French, and Italian.

---

## Supported Platforms

| Platform | Resource & Data Location | Status |
| :--- | :--- | :--- |
| **Linux** | Executable dir (resources); per-user app-data (saves/caches) | Fully supported |
| **Windows** | Executable dir (resources); per-user app-data (saves/caches) | Fully supported |
| **macOS** | Executable dir (resources); per-user app-data (saves/caches) | Fully supported |
| **BSD Family** | Executable dir (resources); per-user app-data (saves/caches) | Supported (verified on FreeBSD) |
| **Haiku** | Executable dir (resources); per-user app-data (saves/caches) | Partially works (no music) |
| **Android** | `Android/data/io.github.wszqkzqk.pvzportable/files/` | Fully supported |
| **iOS / iPadOS** | App Documents directory (via iOS Files app / Finder) | Supported (sideload via unsigned IPA) |
| **Web (WASM)** | Browser IndexedDB (saves); resources loaded locally at runtime | Fully supported (requires HTTP server) |
| **Nintendo Switch** | `sdmc:/switch/PvZPortable` | Works on real hardware (Kenji-NX crashes on boot) |

---

## Getting Started

### Game Assets & Version Compatibility

You need `main.pak` and the `properties/` folder from a legal copy of *Plants vs. Zombies: GOTY Edition* (unpacked resource files can also be used instead of `main.pak`).

PvZ-Portable is designed and tested primarily against **GOTY Edition 1.2.0.1073 (English, standalone PopCap release)**, which is recommended for the best out-of-the-box experience.

Other official releases, including **Steam GOTY 1.2.0.1096** and **non-English GOTY editions** (`1.2.0.1093` DE/ES/FR/IT, or `1.1.0.1056` ZH based on 1.2.0.1073), are also fully playable with identical gameplay mechanics. Because those versions renamed a few XML string keys, you may notice minor UI text differences unless you generate a compatibility override file:

```bash
python3 scripts/make-compat-overrides.py "/path/to/main.pak"
```

This script generates `pvz-portable.xml` inside the `properties/` directory next to `main.pak` (use `-o` to specify a custom output path, or `--force` to overwrite an existing file).

<details>
<summary>Known cosmetic text differences on non-1.2.0.1073 EN assets without overrides (click to expand)</summary>

| Issue (Non-1.2.0.1073 EN only) | Cause |
| :--- | :--- |
| **Almanac blue description text missing** | In 1.2.0.1096, the introductory paragraph was split from `[XXX_DESCRIPTION]` into `[XXX_DESCRIPTION_HEADER]`, which the 1.2.0.1073 engine does not read. |
| **"Restart" button label missing** | `[RESTART_LEVEL]` was renamed to `[RESTART_LEVEL_BUTTON]` in 1.2.0.1096. |
| **Unencountered zombie shows `???`** instead of `(not encountered yet)` | `[NOT_ENCOUNTERED_YET]` was changed to `???` in 1.2.0.1096, moving the old text to `[NOT_ENCOUNTERED_YET_DESCRIPTION]`. |
| **Crazy Dave's plant sell price shows 1/10 of actual value** | In 1.2.0.1073, `[CRAZY_DAVE_1700]` appends a literal `0` after `{SELL_PRICE}` (`${SELL_PRICE}0`) because the engine passes `price / 10`. In 1.2.0.1096 that trailing `0` was removed. |
</details>

### Desktop (Linux, Windows, macOS, BSD, etc.)

Place `main.pak` and the `properties/` folder in the same directory as the `pvz-portable` executable. By default, the game resolves read-only resources relative to the executable directory, so you can launch it from any working directory.

Per-user writable files are stored separately in your operating system's standard application data directory under `io.github.wszqkzqk/PvZPortable`:
- **Linux:** `~/.local/share/io.github.wszqkzqk/PvZPortable/`
- **Windows:** `%APPDATA%\io.github.wszqkzqk\PvZPortable\`
- **macOS:** `~/Library/Application Support/io.github.wszqkzqk/PvZPortable/`

Inside this directory, the game maintains:
- `userdata/`: Player profiles and mid-level save files.
- `cache64/` (64-bit builds) or `cache32/` (32-bit builds): Local startup caches for compiled reanimation and definition data. These use native memory layout and are not meant to be shared across machines; if a cache or schema check fails, the game automatically recompiles them from source assets.
- `registry.regemu`: Persisted game settings.

### Android

Download the prebuilt `arm64-v8a` APK from [Releases](https://github.com/wszqkzqk/PvZ-Portable/releases) (or build from source for other ABIs). Requires Android 7.0+.

1. **First Launch:** Install and open the APK. When no assets are detected, the resource import screen opens automatically.
2. **Import Assets:** Select either:
   - A ZIP archive containing `main.pak` and `properties/` (at the archive root or inside a single top-level folder such as `PvZ/main.pak`), or
   - A directory containing `main.pak` and `properties/`.
3. Tap **Start Game** once import completes.
4. **Managing Data Later:** Long-press the app icon in your launcher and select **Manage Data** to re-import resources or import/export save files.

*Notes:*
- All files are stored under `Android/data/io.github.wszqkzqk.pvzportable/files/`. Imports and exports use the Android Storage Access Framework (SAF), so no broad storage permissions are required.
- The Android build preserves the desktop game's 4:3 aspect ratio and mouse-driven interaction model via SDL3 touch-to-mouse mapping, without mobile-specific UI redesigns.

### iOS / iPadOS

Download the unsigned IPA from [Releases](https://github.com/wszqkzqk/PvZ-Portable/releases) or build it via `ios/build-ios.sh`. Requires iOS 16.4+ (`arm64`).

1. **Sideload the IPA:** Install using [AltStore](https://altstore.io/), [TrollStore](https://github.com/opa334/TrollStore), or Xcode with a free Apple ID (note that free Apple ID signatures expire after 7 days, whereas TrollStore installations are permanent).
2. **Copy Game Assets:** Open the iOS **Files** app (or Finder / iTunes file sharing on desktop) and copy `main.pak` and the `properties/` folder into the **PvZ Portable** Documents directory.

### Web (Browser / WebAssembly)

1. Open the [Online WebAssembly Player](https://wszqkzqk.github.io/PvZ-Portable/pvz-portable.html).
2. Load your game assets from either a ZIP archive or a folder containing `main.pak` and `properties/`, then click **Start Game**.
   - All assets are processed locally in your browser and are never uploaded to any server.
   - Saves are stored in browser IndexedDB and can be exported or imported as a ZIP archive using the on-screen controls.

To self-host the WebAssembly build, download the WASM archive from [Releases](https://github.com/wszqkzqk/PvZ-Portable/releases) and serve it over HTTP (for example, `python3 -m http.server`). Opening the `.html` file directly via `file://` will not work due to browser security restrictions.

---

## Save Data & Conversion Tools

PvZ-Portable uses two types of save files inside `userdata/`:

1. **Global User Data (`users.dat`, `user1.dat`, etc.)**
   - Stores player profiles, adventure progress, coins, Zen Garden plants, and shop purchases.
   - Uses explicit field serialization and is directly compatible with the original PC GOTY Edition across all platforms.

2. **Mid-Level Save States (`game1_0.v4`, etc.)**
   - Created when you choose "Save and Exit" during a level.
   - **`.v4` (Default, Portable):** Serializes game objects field-by-field using Type-Length-Value (TLV) tags. You can freely copy `.v4` files between Windows, Linux, macOS, Android, iOS, Web, and Switch across 32/64-bit and different CPU architectures.
   - **`.dat` (Legacy):** The original game's mid-level save format dumped raw C++ structs directly to disk, which breaks across different architectures, pointer sizes, or compilers. PvZ-Portable still loads legacy `.dat` saves as a fallback and automatically migrates them to `.v4`.

### Inspecting & Editing `.v4` Saves

The repository includes `scripts/pvzp-v4-converter.py` for inspecting mid-level saves or converting them to and from human-readable YAML:

```bash
# View basic level statistics (wave, sun, plant/zombie counts, etc.)
python3 scripts/pvzp-v4-converter.py info ~/.local/share/io.github.wszqkzqk/PvZPortable/userdata/game1_13.v4

# Export a binary .v4 save to YAML for inspection or editing
python3 scripts/pvzp-v4-converter.py export ~/.local/share/io.github.wszqkzqk/PvZPortable/userdata/game1_13.v4 level.yaml

# Re-import modified YAML back into a .v4 save (back up your original file first!)
cp ~/.local/share/io.github.wszqkzqk/PvZPortable/userdata/game1_13.v4 ~/.local/share/io.github.wszqkzqk/PvZPortable/userdata/game1_13.v4.bak
python3 scripts/pvzp-v4-converter.py import level.yaml ~/.local/share/io.github.wszqkzqk/PvZPortable/userdata/game1_13.v4
```

---

## Command-Line Arguments

Both `-flag <value>` and `-flag=<value>` syntaxes are supported:

### Path Overrides
- `-resdir <path>`: Set the directory where read-only game assets (`main.pak` and `properties/`) are located.
- `-savedir <path>`: Override the directory used for writable data (saves, caches, settings, and screenshots).

### Recording and Playing Back Gameplay
Demo files (`.dmo`) record deterministic input and RNG state for debugging and regression testing. Automatically named recordings are written to the current working directory:
- `-record [<path>]`: Record the session to `pvzp-YYYYMMDD-HHMMSS[-N].dmo`, or to `<path>` if specified.
- `-recnum <N>`: Record a new session and retain only the `N` most recent auto-named `pvzp-*.dmo` files (custom-named files are untouched).
- `-play [<path>]`: Replay the most recent `pvzp-*.dmo` file, or `<path>` if specified.
- `-playnum <N>`: Replay the `N`-th most recent auto-named `pvzp-*.dmo` recording (`1` is the latest).

---

## Building from Source

### Dependencies

- **Build System:** `CMake` (>= 3.12), `Ninja`
- **Compiler:** C++20-capable compiler and standard library (`GCC`, `Clang`, or `MSVC`)
- **Libraries:** `SDL3` (>= 3.2.12), `SDL3_mixer`, `libopenmpt`, `libpng`, `libjpeg-turbo`, `zlib`
- **Graphics API:** `OpenGL ES 2.0` or `OpenGL 2.1+` headers/drivers

Install dependencies for your platform:

- **Arch Linux:**
  ```bash
  sudo pacman -S --needed base-devel cmake libjpeg-turbo libopenmpt libpng ninja sdl3 sdl3_mixer
  ```
- **Debian / Ubuntu:**
  ```bash
  sudo apt install cmake ninja-build libjpeg-dev libopenmpt-dev libpng-dev libsdl3-dev libsdl3-mixer-dev
  ```
- **Windows (MSYS2 UCRT64):**
  ```bash
  pacman -S --needed base-devel mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libjpeg-turbo mingw-w64-ucrt-x86_64-libopenmpt mingw-w64-ucrt-x86_64-libpng mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL3 mingw-w64-ucrt-x86_64-SDL3_mixer
  ```
- **macOS (Homebrew):**
  ```bash
  brew install cmake dylibbundler jpeg-turbo libopenmpt libpng ninja sdl3 sdl3_mixer
  ```

### Compiling

Configure and build from the repository root:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For local native builds, you can also enable CPU-specific instruction sets via compiler flags:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-march=native" -DCMAKE_CXX_FLAGS="-march=native"
```

### CMake Configuration Options

Pass `-D<OPTION>=ON/OFF` during the `cmake -B build` step to customize the build:

| Option | Default | Description |
| :--- | :--- | :--- |
| `PVZ_DEBUG` | `OFF` (`ON` in `Debug` builds) | Enable in-game cheat keys, debug overlays, and diagnostic features. |
| `DO_FIX_BUGS` | `OFF` | Apply optional community fixes for original 1.2.0.1073 gameplay quirks and bugs.[^1] Disabled by default because many players consider original behaviors part of classic gameplay. |
| `LOW_MEMORY` | `OFF` | Reduce memory footprint by disabling resource pre-caching (intended for consoles with limited RAM). |
| `CONSOLE` | `OFF` (`ON` in `Debug` builds) | Attach a stdout/stderr console window on Windows. |
| `BUILD_STATIC` | `OFF` | Statically link dependencies on Windows (MinGW) and glibc-based Linux. For MSVC, use a vcpkg `-static` triplet instead. |

[^1]: `DO_FIX_BUGS` currently includes the following adjustments:
    - Fix Bungee Zombie duplicate sun/item drop in *I, Zombie* mode.
    - Make mind-controlled Gargantuars smash enemy zombies instead of plants, smash vases in *Scary Potter* mode, and throw mind-controlled Imps (with corrected scale, health, and direction).
    - Make mind-controlled Pea/Gatling Head zombies shoot forward toward enemy zombies instead of backward.
    - Make mind-controlled Jalapeno and Squash zombies target and damage enemy zombies instead of plants (including coordinate fixes for Squash tracking).
    - Allow mind-controlled Jalapeno zombies to clear Dr. Zomboss's ice/fireballs and ladder placements properly.
    - Synchronize Backup Dancer animations with the Dancing Zombie (fixes the "Maid" displacement glitch).
    - Fix visual glitch when a Ladder Zombie recovers its arm.
    - Fix Dr. Zomboss's attack (RV smash, fireball/iceball) and zombie summon row ranges on 6-lane Pool/Fog levels.

For example, to build an optimized Release binary with cheat keys enabled:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DPVZ_DEBUG=ON
```

---

## Contributing

Contributions are welcome! If you encounter build or gameplay issues, please open an issue with your platform and reproduction details.

### Developer Notes

- **Format v4 Save Compatibility:**
  - Always assign new TLV IDs when introducing new serialized fields; never reuse existing IDs.
  - Provide sensible default values when loading older `.v4` saves that lack newly added fields.
  - Never dump raw C++ structs for layout-dependent types; serialize fields explicitly using fixed-width primitives.

<details>
<summary>SexyAppFramework Coding Conventions (click to expand)</summary>

From the original SexyAppFramework documentation:

> The framework differs from many other APIs in that some class properties are not wrapped in accessor methods, but rather are made to be accessed directly through public member data. The window caption of your application, for example, is set by assigning a value to the `std::string mTitle` in the application object before the application's window is created. We felt that in many cases this reduced the code required to implement a class. Also of note is the prefix notation used on variables: `m` denotes a class member, `the` denotes a parameter passed to a method or function, and `a` denotes a local variable.
</details>

---

## License & Acknowledgments

**Copyright (C) 2026 Zhou Qiankang `<wszqkzqk@qq.com>`**

This project's code is licensed under the [**GNU Lesser General Public License v3.0 or later**](https://www.gnu.org/licenses/lgpl-3.0.html) (`LGPL-3.0-or-later`). Full license texts are provided in `LICENSE` (LGPL-3.0) and `COPYING` (GPL-3.0) at the repository root. The code is provided "as is", without warranty of any kind.

- **Intellectual Property:** *Plants vs. Zombies* and all original game assets are the property of PopCap Games and Electronic Arts. The LGPL-3.0-or-later license applies solely to the source code in this repository. Project icons and platform logos were created by `wszqkzqk` with the assistance of AI image generation tools and are not official PopCap/EA assets.
- **PopCap Games Framework:** Portions of `src/SexyAppFramework` are derived from the [PopCap Games Framework](src/SexyAppFramework/LICENSE) and have been refactored and modernized by the community under the LGPL-3.0-or-later license:
  > *"This product includes portions of the PopCap Games Framework, © 2005-2009 PopCap Games, Inc. All rights reserved. (http://popcapframework.sourceforge.net/)."*

### Acknowledgments
- **PopCap Games** for creating *Plants vs. Zombies* and releasing the SexyAppFramework under a permissive license.
- **The SDL Team** for SDL3 and SDL3_mixer, which power the cross-platform windowing, input, rendering, and audio backends.
- **The OpenMPT Team** for `libopenmpt`, enabling accurate MO3 tracker music playback.
- All community contributors and researchers, especially [@Headshotnoby](https://github.com/headshot2017) and [@Patoke](https://github.com/Patoke) for their foundational work.
