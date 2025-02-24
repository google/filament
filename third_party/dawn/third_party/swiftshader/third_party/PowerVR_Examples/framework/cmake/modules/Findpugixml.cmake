# Findpugixml.cmake
#
# Finds the pugixml library and its dependencies
#
# This will define the following imported targets
#     pugixml

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(pugixml_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../external/pugixml/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
	endif()
endif()

if(NOT TARGET pugixml)
	find_package(pugixml REQUIRED CONFIG)
endif()