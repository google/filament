# Findglm.cmake
#
# Finds the glm target
#
# This will define the following imported targets
#     glm

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(glm_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../external/glm/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
	endif()
endif()

if(NOT TARGET glm)
	find_package(glm REQUIRED CONFIG)
endif()