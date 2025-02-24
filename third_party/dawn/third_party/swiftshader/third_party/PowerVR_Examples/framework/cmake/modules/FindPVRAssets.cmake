# FindPVRAssets.cmake
#
# Finds the PVRAssets library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     tinygltf
#     PVRAssets

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET tinygltf)
	find_dependency(tinygltf REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRAssets_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRAssets/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRAssets")
	endif()
endif()

if(NOT TARGET PVRAssets)
	find_package(PVRAssets REQUIRED CONFIG)
endif()