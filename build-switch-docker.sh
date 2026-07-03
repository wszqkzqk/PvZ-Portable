#!/bin/bash
set -ex
source $DEVKITPRO/switchvars.sh
dkp-pacman -S --needed --noconfirm switch-sdl2 switch-libogg switch-libvorbis switch-mpg123 switch-libpng switch-libjpeg-turbo switch-zlib switch-cmake

if [ ! -f "$PORTLIBS_PREFIX/lib/libopenmpt.a" ]; then
  git clone --depth 1 https://github.com/OpenMPT/openmpt /tmp/openmpt
  cd /tmp/openmpt
  make -j$(nproc) CONFIG=gcc SHARED_LIB=0 DYNLINK=0 EXAMPLES=0 OPENMPT123=0 TEST=0 PREFIX=$PORTLIBS_PREFIX CC=aarch64-none-elf-gcc CXX=aarch64-none-elf-g++ install
fi

cd /work
cmake -B build-switch -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -DPVZ_DEBUG=ON
cmake --build build-switch -j$(nproc)
ls -la build-switch/pvz-portable.nro
