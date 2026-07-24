# Derives the project version from git tags, the same source of truth that
# PKGBUILD and android/app/build.gradle read. Sets:
#   PVZP_VERSION       full git describe
#   PVZP_VERSION_PLAIN leading tag only (Apple/console version fields)
#   PVZP_BUILD_NUMBER  commit count, grows monotonically on main
#   PVZP_COMMIT_DATE   HEAD commit date
# and generates ${PROJECT_BINARY_DIR}/ProjectVersion.h.
set(PVZP_VERSION "0.1")
set(PVZP_VERSION_PLAIN "0.1")
set(PVZP_BUILD_NUMBER "1")
set(PVZP_COMMIT_DATE "unknown")

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

	execute_process(
		COMMAND "${GIT_EXECUTABLE}" show -s --format=%cs HEAD
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		OUTPUT_VARIABLE _date
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(_date)
		set(PVZP_COMMIT_DATE "${_date}")
	endif()

	# Watch HEAD and its reflog so a checkout or commit re-runs configure;
	# reflogs are per-worktree and never packed, unlike branch refs.
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse --absolute-git-dir
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		OUTPUT_VARIABLE _git_dir
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	if(EXISTS "${_git_dir}/HEAD")
		set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
			"${_git_dir}/HEAD" "${_git_dir}/logs/HEAD")
	endif()
	unset(_describe)
	unset(_count)
	unset(_date)
	unset(_git_dir)
endif()

configure_file("${PROJECT_SOURCE_DIR}/CMake/ProjectVersion.h.in" "${PROJECT_BINARY_DIR}/ProjectVersion.h" @ONLY)
