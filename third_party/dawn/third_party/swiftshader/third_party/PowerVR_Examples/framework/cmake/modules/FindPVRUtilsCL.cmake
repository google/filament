# FindPVRUtilsCL.cmake
#
# Finds the PVRUtilsCL library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#	  PVRUtilsCL

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRUtilsCL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRUtils/OpenCL/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRUtilsCL")
	endif()
endif()

if(NOT TARGET PVRUtilsCL)
	find_package(PVRUtilsCL REQUIRED CONFIG)
endif()