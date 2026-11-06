include(FetchContent)

function(pvz_latest_stable_release repository out_version fallback_version)
	execute_process(
		COMMAND git ls-remote --tags "${repository}" "refs/tags/release-*"
		OUTPUT_VARIABLE PVZP_SDL_REFS
		RESULT_VARIABLE PVZP_SDL_REFS_RESULT
		ERROR_QUIET
	)
	if(NOT PVZP_SDL_REFS_RESULT EQUAL 0)
		set(${out_version} "${fallback_version}" PARENT_SCOPE)
		return()
	endif()

	string(REPLACE "\r" "" PVZP_SDL_REFS "${PVZP_SDL_REFS}")
	string(REPLACE "\n" ";" PVZP_SDL_REF_LINES "${PVZP_SDL_REFS}")
	set(PVZP_SDL_LATEST "")
	foreach(PVZP_SDL_REF IN LISTS PVZP_SDL_REF_LINES)
		if(PVZP_SDL_REF MATCHES "refs/tags/release-([0-9]+\\.[0-9]+\\.[0-9]+)$")
			if(PVZP_SDL_LATEST STREQUAL "" OR CMAKE_MATCH_1 VERSION_GREATER PVZP_SDL_LATEST)
				set(PVZP_SDL_LATEST "${CMAKE_MATCH_1}")
			endif()
		endif()
	endforeach()
	if(PVZP_SDL_LATEST STREQUAL "")
		set(PVZP_SDL_LATEST "${fallback_version}")
	endif()
	set(${out_version} "${PVZP_SDL_LATEST}" PARENT_SCOPE)
endfunction()

find_package(SDL3 3.4.0 CONFIG QUIET)
if(NOT TARGET SDL3::SDL3)
	if(BUILD_STATIC OR EMSCRIPTEN OR NINTENDO_SWITCH OR IOS)
		set(SDL_SHARED OFF CACHE BOOL "" FORCE)
		set(SDL_STATIC ON CACHE BOOL "" FORCE)
	else()
		set(SDL_SHARED ON CACHE BOOL "" FORCE)
		set(SDL_STATIC OFF CACHE BOOL "" FORCE)
	endif()
	set(SDL_TESTS OFF CACHE BOOL "" FORCE)
	set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
	if(NINTENDO_SWITCH)
		set(SDL_VIDEO OFF CACHE BOOL "" FORCE)
		set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
		file(DOWNLOAD
			https://raw.githubusercontent.com/neomody77/sdl3-switch/main/sdl3-switch.patch
			"${CMAKE_BINARY_DIR}/sdl3-switch.patch"
		)
		set(PVZP_SDL3_PATCH_COMMAND PATCH_COMMAND git apply "${CMAKE_BINARY_DIR}/sdl3-switch.patch")
	endif()
	pvz_latest_stable_release(https://github.com/libsdl-org/SDL.git PVZP_SDL3_VERSION "3.4.20")
	FetchContent_Declare(SDL3
		URL "https://github.com/libsdl-org/SDL/releases/download/release-${PVZP_SDL3_VERSION}/SDL3-${PVZP_SDL3_VERSION}.tar.gz"
		${PVZP_SDL3_PATCH_COMMAND}
	)
	FetchContent_MakeAvailable(SDL3)
endif()

find_package(SDL3_mixer CONFIG QUIET)
if(NOT TARGET SDL3_mixer::SDL3_mixer)
	set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_INSTALL OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_TESTS OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_AIFF OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_WAVE ON CACHE BOOL "" FORCE)
	set(SDLMIXER_VOC OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_AU ON CACHE BOOL "" FORCE)
	set(SDLMIXER_FLAC OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_GME OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_MOD OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_MIDI OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_OPUS OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)
	set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_VORBIS_STB ON CACHE BOOL "" FORCE)
	set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
	set(SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
	pvz_latest_stable_release(https://github.com/libsdl-org/SDL_mixer.git PVZP_SDL3_MIXER_VERSION "3.2.4")
	FetchContent_Declare(SDL3_mixer
		URL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${PVZP_SDL3_MIXER_VERSION}/SDL3_mixer-${PVZP_SDL3_MIXER_VERSION}.tar.gz"
	)
	FetchContent_MakeAvailable(SDL3_mixer)
endif()
