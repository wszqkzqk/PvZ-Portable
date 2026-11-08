cmake_minimum_required(VERSION 3.16)
project(pvz-portable-harmony LANGUAGES C CXX)

# Build SDL3, SDL3_mixer and the game into the HAP; run harmony/build-deps.sh first.

get_filename_component(PVZP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(PVZP_HARMONY_DEPS "${PVZP_ROOT}/harmony/deps" CACHE PATH "Output directory of harmony/build-deps.sh")
set(PVZP_DEPS_PREFIX "${PVZP_HARMONY_DEPS}/${OHOS_ARCH}")

if(NOT EXISTS "${PVZP_HARMONY_DEPS}/src/SDL/CMakeLists.txt"
		OR NOT EXISTS "${PVZP_HARMONY_DEPS}/src/SDL3_mixer/CMakeLists.txt"
		OR NOT EXISTS "${PVZP_DEPS_PREFIX}/include/libopenmpt/libopenmpt.h")
	message(FATAL_ERROR "HarmonyOS dependencies for ${OHOS_ARCH} are missing; run harmony/build-deps.sh ${OHOS_ARCH}")
endif()

# SDL3 must remain shared because ArkTS calls into libSDL3.so directly.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory("${PVZP_HARMONY_DEPS}/src/SDL" SDL3)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
set(SDLMIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
set(SDLMIXER_TESTS OFF CACHE BOOL "" FORCE)
set(SDLMIXER_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDLMIXER_AIFF OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VOC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDLMIXER_GME OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MIDI OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
set(SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
add_subdirectory("${PVZP_HARMONY_DEPS}/src/SDL3_mixer" SDL3_mixer)

# Prebuilt static libpng, libjpeg-turbo and libopenmpt; the toolchain re-roots searches into the sysroot.
list(APPEND CMAKE_PREFIX_PATH "${PVZP_DEPS_PREFIX}")
list(APPEND CMAKE_FIND_ROOT_PATH "${PVZP_DEPS_PREFIX}")

add_subdirectory("${PVZP_ROOT}" pvz-portable)
