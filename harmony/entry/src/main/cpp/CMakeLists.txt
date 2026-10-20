cmake_minimum_required(VERSION 3.16)
project(pvz-portable-harmony LANGUAGES C CXX)

# Build SDL3, sdl2-compat and the game into the HAP; run harmony/build-deps.sh first.

get_filename_component(PVZP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(PVZP_HARMONY_DEPS "${PVZP_ROOT}/harmony/deps" CACHE PATH "Output directory of harmony/build-deps.sh")
set(PVZP_DEPS_PREFIX "${PVZP_HARMONY_DEPS}/${OHOS_ARCH}")

if(NOT EXISTS "${PVZP_HARMONY_DEPS}/src/SDL/CMakeLists.txt"
		OR NOT EXISTS "${PVZP_HARMONY_DEPS}/src/sdl2-compat/CMakeLists.txt"
		OR NOT EXISTS "${PVZP_DEPS_PREFIX}/include/libopenmpt/libopenmpt.h")
	message(FATAL_ERROR "HarmonyOS dependencies for ${OHOS_ARCH} are missing; run harmony/build-deps.sh ${OHOS_ARCH}")
endif()

# SDL3 provides the HarmonyOS backend and loads libmain.so.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory("${PVZP_HARMONY_DEPS}/src/SDL" SDL3)

# sdl2-compat implements the SDL2 API the game is written against on top of libSDL3.so.
set(SDL2COMPAT_TESTS OFF CACHE BOOL "" FORCE)
# SDL-Mixer-X exports its install targets unconditionally, so SDL2 has to be in an export set too.
set(SDL2COMPAT_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2COMPAT_X11 OFF CACHE BOOL "" FORCE)
add_subdirectory("${PVZP_HARMONY_DEPS}/src/sdl2-compat" sdl2-compat)
set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "" FORCE)

# Prebuilt static libpng, libjpeg-turbo and libopenmpt; the toolchain re-roots searches into the sysroot.
list(APPEND CMAKE_PREFIX_PATH "${PVZP_DEPS_PREFIX}")
list(APPEND CMAKE_FIND_ROOT_PATH "${PVZP_DEPS_PREFIX}")

add_subdirectory("${PVZP_ROOT}" pvz-portable)
