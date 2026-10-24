# find_package(SDL2) shim for the HarmonyOS build: sdl2-compat is built in the same CMake tree
# (see ../CMakeLists.txt), and both the game and SDL-Mixer-X look SDL2 up with find_package().
if(NOT TARGET SDL2::SDL2)
	message(FATAL_ERROR "add sdl2-compat with add_subdirectory() before find_package(SDL2)")
endif()

set(SDL2_FOUND TRUE)
set(SDL2_INCLUDE_DIRS "")
set(SDL2_LIBRARIES SDL2::SDL2)
