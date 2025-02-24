# FindPVRCore.cmake
#
# Finds the PVRCore library and its dependencies
#
# This will define the following imported targets
#     glm
#     pugixml
#	  PVRCore

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET glm)
	find_dependency(glm REQUIRED MODULE)
endif()

if(NOT TARGET pugixml)
	find_dependency(pugixml REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRCore_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRCore/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRCore")
	endif()
endif()

if(NOT TARGET PVRCore)
	find_package(PVRCore REQUIRED CONFIG)
endif()