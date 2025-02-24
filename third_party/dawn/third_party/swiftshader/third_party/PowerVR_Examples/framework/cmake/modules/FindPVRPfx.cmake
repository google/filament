# FindPVRPfx.cmake
#
# Finds the PVRPfx library and its dependencies
#
# This will define the following imported targets
#     PVRUtilsVk
#     PVRPfx

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRUtilsVk)
	find_dependency(PVRUtilsVk REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRPfx_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRPfx/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRPfx")
	endif()
endif()

if(NOT TARGET PVRPfx)
	find_package(PVRPfx REQUIRED CONFIG)
endif()