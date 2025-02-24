# FindPVRCamera.cmake
#
# Finds the PVRCamera library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRCamera

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRCamera_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRCamera/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRCamera")
	endif()
endif()

if(NOT TARGET PVRCamera)
	find_package(PVRCamera REQUIRED CONFIG)
endif()