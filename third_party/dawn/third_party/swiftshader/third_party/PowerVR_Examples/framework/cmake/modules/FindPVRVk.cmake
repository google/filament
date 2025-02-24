# FindPVRVk.cmake
#
# Finds the PVRVk library
#
# This will define the following imported targets
#     PVRVk

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRVk_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../framework/PVRVk/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRVk")
	endif()
endif()

if(NOT TARGET PVRVk)
	find_package(PVRVk REQUIRED CONFIG)
endif()