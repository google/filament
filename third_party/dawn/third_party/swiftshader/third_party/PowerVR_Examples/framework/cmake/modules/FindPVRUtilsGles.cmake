# FindPVRUtilsGles.cmake
#
# Finds the PVRUtilsGles library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRAssets
#     PVRUtilsGles

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET PVRAssets)
	find_dependency(PVRAssets REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRUtilsGles_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRUtils/OpenGLES/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRUtilsGles")
	endif()
endif()

if(NOT TARGET PVRUtilsGles)
	find_package(PVRUtilsGles REQUIRED CONFIG)
endif()