# Community arm64-ios triplet with the deployment target pinned, so vcpkg
# ports inherit the main project's baseline instead of the build SDK's.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME iOS)
set(VCPKG_OSX_DEPLOYMENT_TARGET "16.4")
