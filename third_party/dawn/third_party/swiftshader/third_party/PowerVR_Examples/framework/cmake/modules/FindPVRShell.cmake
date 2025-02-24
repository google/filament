# FindPVRShell.cmake
#
# Finds the PVRShell library and its dependencies
#
# This will define the following imported targets
#	  PVRCore
#	  PVRShell

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRShell_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRShell/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRShell")
	endif()
endif()

if(NOT TARGET PVRShell)
	find_package(PVRShell REQUIRED CONFIG)
endif()