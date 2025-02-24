# Findtinygltf.cmake
#
# Finds the tinygltf target
#
# This will define the following imported targets
#     tinygltf

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(tinygltf_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../external/tinygltf/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}")
	endif()
endif()

if(NOT TARGET tinygltf)
	find_package(tinygltf REQUIRED CONFIG)
endif()