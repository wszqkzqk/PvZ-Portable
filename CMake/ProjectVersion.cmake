# Derives the project version from git tags, the same source of truth that
# PKGBUILD and android/app/build.gradle read. Sets:
#   PVZP_VERSION       full git describe, e.g. 0.1.27-15-gcbcd2ef3ffcc
#   PVZP_VERSION_PLAIN leading tag only, e.g. 0.1.27 (Apple/console version fields)
#   PVZP_BUILD_NUMBER  commit count, grows monotonically on main
# and generates ${PROJECT_BINARY_DIR}/PvzpVersion.h.
set(PVZP_VERSION "0.1")
set(PVZP_VERSION_PLAIN "0.1")
set(PVZP_BUILD_NUMBER "1")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" describe --tags --abbrev=12
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		OUTPUT_VARIABLE _describe
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(_describe)
		string(REGEX REPLACE "^v" "" PVZP_VERSION "${_describe}")
		# CFBundleShortVersionString & friends accept only dotted numbers
		string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" PVZP_VERSION_PLAIN "${PVZP_VERSION}")
		if(NOT PVZP_VERSION_PLAIN)
			set(PVZP_VERSION_PLAIN "0.1")
		endif()
	endif()

	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		OUTPUT_VARIABLE _count
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(_count)
		set(PVZP_BUILD_NUMBER "${_count}")
	endif()

	# Re-run configure when a commit or checkout moves HEAD. HEAD itself only
	# changes on checkout; the ref it points to changes on commit, so watch
	# both. configure_file leaves unchanged outputs untouched, making these
	# re-configures free unless the version actually changed.
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse --absolute-git-dir
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		OUTPUT_VARIABLE _git_dir
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_git_dir}/HEAD")
	file(READ "${_git_dir}/HEAD" _head)
	if(_head MATCHES "^ref: (.+)\n")
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_git_dir}/${CMAKE_MATCH_1}")
	endif()
	unset(_describe)
	unset(_count)
	unset(_git_dir)
	unset(_head)
endif()

configure_file("${PROJECT_SOURCE_DIR}/CMake/PvzpVersion.h.in" "${PROJECT_BINARY_DIR}/PvzpVersion.h" @ONLY)
