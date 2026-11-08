cmake_minimum_required(VERSION 3.12...4.0)

project(pvz-portable LANGUAGES C CXX)

include(CMake/ProjectVersion.cmake)

option(BUILD_STATIC "Statically link dependencies for a standalone exe (Windows MinGW, Linux)" OFF)
if(BUILD_STATIC)
	if(WIN32 AND NOT MSVC)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
		set(BUILD_SHARED_LIBS OFF)
		add_link_options(-static)
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
		set(BUILD_SHARED_LIBS OFF)
	elseif(MSVC)
		message(WARNING "BUILD_STATIC is not supported for MSVC. Use a vcpkg -static triplet instead.")
		set(BUILD_STATIC OFF)
	else()
		message(WARNING "BUILD_STATIC is only supported on Windows with MinGW-based toolchains and on Linux.")
		set(BUILD_STATIC OFF)
	endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_GENERATOR MATCHES "Ninja")
	set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

if(MSVC)
	if(DEFINED VCPKG_TARGET_TRIPLET AND VCPKG_TARGET_TRIPLET MATCHES "-static")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	elseif(DEFINED VCPKG_TARGET_TRIPLET)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	endif()
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS
	src/*.cpp
	src/SexyAppFramework/*.cpp
	src/SexyAppFramework/graphics/*.cpp
	src/SexyAppFramework/imagelib/*.cpp
	src/SexyAppFramework/misc/*.cpp
	src/SexyAppFramework/paklib/*.cpp
	src/SexyAppFramework/sound/*.cpp
	src/SexyAppFramework/widget/*.cpp
	src/SexyAppFramework/fcaseopen/*.c
	src/PvzpLib/*.cpp
	src/Lawn/*.cpp
	src/Lawn/Widget/*.cpp
	src/Lawn/System/*.cpp
)

option(LOW_MEMORY "Enable low memory mode" OFF)
if(VCPKG_TOOLCHAIN)
	find_package(libopenmpt CONFIG REQUIRED)
	set(OPENMPT_LIB libopenmpt::libopenmpt)
else()
	find_library(OPENMPT_LIBRARY NAMES openmpt libopenmpt REQUIRED)
	find_path(OPENMPT_INCLUDE_DIR libopenmpt/libopenmpt.h REQUIRED)
	set(OPENMPT_LIB ${OPENMPT_LIBRARY})
	if(NOT EMSCRIPTEN AND NOT NINTENDO_SWITCH AND OPENMPT_LIBRARY MATCHES "\\${CMAKE_STATIC_LIBRARY_SUFFIX}$")
		include(CheckCXXSourceCompiles)
		include(CMakePushCheckState)
		cmake_push_check_state()
		set(CMAKE_REQUIRED_INCLUDES ${OPENMPT_INCLUDE_DIR})
		set(CMAKE_REQUIRED_LIBRARIES ${OPENMPT_LIBRARY})
		check_cxx_source_compiles("#include <libopenmpt/libopenmpt.h>
int main() {
	return openmpt_module_create_from_memory2(0, 0, 0, 0, 0, 0, 0, 0, 0) != 0;
}" OPENMPT_LINKS_STANDALONE)
		cmake_pop_check_state()
		if(NOT OPENMPT_LINKS_STANDALONE)
			find_library(MPG123_LIBRARY NAMES mpg123 libmpg123)
			find_library(VORBISFILE_LIBRARY NAMES vorbisfile libvorbisfile)
			find_library(VORBIS_LIBRARY NAMES vorbis libvorbis)
			find_library(OGG_LIBRARY NAMES ogg libogg)
			if(NOT MPG123_LIBRARY OR NOT VORBISFILE_LIBRARY OR NOT VORBIS_LIBRARY OR NOT OGG_LIBRARY)
				message(FATAL_ERROR "Static libopenmpt does not link standalone and some of its dependencies were not found (see the configure log for details)")
			endif()
			list(APPEND OPENMPT_LIB ${MPG123_LIBRARY} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
		endif()
	endif()
endif()

if(NINTENDO_SWITCH)
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/switch/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/switch/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/switch
	)
elseif(EMSCRIPTEN)
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/emscripten/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/default/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/emscripten
	)
else()
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/default/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/default/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/default
	)
endif()

if(CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
	set(WIN_CONSOLE_DEFAULT ON)
	set(PVZ_DEBUG_DEFAULT ON)
else()
	set(WIN_CONSOLE_DEFAULT OFF)
	set(PVZ_DEBUG_DEFAULT OFF)
endif()

option(PVZ_DEBUG "Enable PVZ_DEBUG macro to enable cheats, hidden features, etc." ${PVZ_DEBUG_DEFAULT})
option(CONSOLE "Show console on Windows" ${WIN_CONSOLE_DEFAULT})
option(DO_FIX_BUGS "Define DO_FIX_BUGS macro (Community fixes for original game bugs of 1.2.0.1073 GOTY Edition)" OFF)

if(EMSCRIPTEN)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_VORBIS=1 -sUSE_MPG123=1")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_VORBIS=1 -sUSE_MPG123=1 -fexceptions")
else()
	find_package(Threads REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(PNG REQUIRED)
endif()
if(EMSCRIPTEN AND PVZP_SDL3_MIXER_SOURCE_DIR)
	include(CMake/EmscriptenSDL3.cmake)
	add_subdirectory("${PVZP_SDL3_MIXER_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/SDL3_mixer")
else()
	find_package(SDL3 3.2.12 CONFIG REQUIRED)
	find_package(SDL3_mixer CONFIG REQUIRED)
endif()

set(PVZP_SDL3_TARGET SDL3::SDL3)
set(PVZP_SDL3_MIXER_TARGET SDL3_mixer::SDL3_mixer)
if(BUILD_STATIC)
	if(TARGET SDL3::SDL3-static)
		set(PVZP_SDL3_TARGET SDL3::SDL3-static)
	endif()
	if(TARGET SDL3_mixer::SDL3_mixer-static)
		set(PVZP_SDL3_MIXER_TARGET SDL3_mixer::SDL3_mixer-static)
	endif()
endif()

# Android requires a shared library loaded by SDLActivity; other platforms use an executable.
if(ANDROID)
	add_library(pvz-portable SHARED ${SOURCES})
elseif(IOS)
	add_executable(pvz-portable MACOSX_BUNDLE ${SOURCES})
else()
	add_executable(pvz-portable ${SOURCES})
endif()

target_include_directories(pvz-portable PRIVATE
	${PLAT_INCLUDES}
	${OPENMPT_INCLUDE_DIR}
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/SexyAppFramework
)
target_compile_definitions(pvz-portable PRIVATE
	$<$<BOOL:${PVZ_DEBUG}>:PVZ_DEBUG>
	$<$<BOOL:${DO_FIX_BUGS}>:DO_FIX_BUGS>
)
target_compile_features(pvz-portable PRIVATE cxx_std_20)
set_target_properties(pvz-portable PROPERTIES CXX_EXTENSIONS OFF)

if (LOW_MEMORY)
	target_compile_definitions(pvz-portable PRIVATE LOW_MEMORY)
endif()

target_link_libraries(pvz-portable PRIVATE
	${PVZP_SDL3_TARGET}
	${PVZP_SDL3_MIXER_TARGET}
	${OPENMPT_LIB}
)

if(EMSCRIPTEN)
	set_property(TARGET pvz-portable APPEND PROPERTY LINK_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/wasm/shell.html"
	)
	target_link_options(pvz-portable PRIVATE
		"SHELL:-sUSE_ZLIB=1"
		"SHELL:-sUSE_LIBPNG=1"
		"SHELL:-sUSE_LIBJPEG=1"
		"SHELL:-sUSE_VORBIS=1 -sUSE_MPG123=1"
		"SHELL:-fexceptions"
		"SHELL:-sASYNCIFY"
		"SHELL:-sALLOW_MEMORY_GROWTH=1"
		"SHELL:-sMAXIMUM_MEMORY=1073741824"
		"SHELL:-sGROWABLE_ARRAYBUFFERS=0"
		"SHELL:-sFORCE_FILESYSTEM=1"
		"SHELL:-sEXPORTED_RUNTIME_METHODS=[\"FS\",\"IDBFS\",\"callMain\"]"
		"SHELL:-sINVOKE_RUN=0"
		"SHELL:-lidbfs.js"
		"SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/wasm/shell.html"
	)
	set_target_properties(pvz-portable PROPERTIES SUFFIX ".html")
else()
	target_link_libraries(pvz-portable PRIVATE
		PNG::PNG
		JPEG::JPEG
		ZLIB::ZLIB
		Threads::Threads
	)
endif()

if (WIN32)
	if(MSVC)
		target_compile_options(pvz-portable PRIVATE /utf-8)
	endif()

	target_sources(pvz-portable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/LawnProject.rc)

	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/LawnProject.rc
		PROPERTIES
			COMPILE_DEFINITIONS "APP_ICON_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/icon.ico\""
			OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico"
	)

	target_link_libraries(pvz-portable PRIVATE ws2_32 user32 gdi32 winmm imm32 shlwapi)
	target_compile_definitions(pvz-portable PRIVATE WINDOWS)

	if(NOT CONSOLE)
		if(MSVC)
			set_target_properties(pvz-portable PROPERTIES WIN32_EXECUTABLE TRUE)
		else()
			target_link_options(pvz-portable PRIVATE -mwindows)
		endif()
	endif()
elseif (NINTENDO_SWITCH)
	target_link_libraries(pvz-portable PRIVATE GLESv2 EGL glapi drm_nouveau)

	nx_generate_nacp(pvz-portable.nacp
		NAME "PvZ Portable"
		AUTHOR "wszqkzqk, etc., a community-driven port"
		VERSION "${PVZP_VERSION_PLAIN}"
	)

	nx_create_nro(pvz-portable
		NACP pvz-portable.nacp
		ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon-switch.jpg
	)
elseif (ANDROID)
	target_link_libraries(pvz-portable PRIVATE GLESv2 EGL android log)
	set_target_properties(pvz-portable PROPERTIES OUTPUT_NAME "main")
elseif (IOS)
	target_link_libraries(pvz-portable PRIVATE
		"-framework Foundation"
		"-framework UIKit"
		"-framework OpenGLES"
		"-framework QuartzCore"
		"-framework CoreGraphics"
		"-framework CoreMotion"
		"-framework GameController"
		"-framework AVFoundation"
		"-framework AudioToolbox"
		"-framework Metal"
	)

	configure_file(ios/Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
	set_target_properties(pvz-portable PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.wszqkzqk.pvz-portable"
		MACOSX_BUNDLE_BUNDLE_NAME "PvZ Portable"
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
		XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
		XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "16.4"
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "io.github.wszqkzqk.pvz-portable"
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
		XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
	)

	# Add launch storyboard to the bundle
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard")
		target_sources(pvz-portable PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard")
		set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard"
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()

	# Add asset catalog (app icon)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ios/Assets.xcassets")
		target_sources(pvz-portable PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ios/Assets.xcassets")
		set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/ios/Assets.xcassets"
			PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()
endif()
