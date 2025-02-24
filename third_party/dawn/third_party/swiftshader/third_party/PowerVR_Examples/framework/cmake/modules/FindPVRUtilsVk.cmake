# FindPVRUtilsVk.cmake
#
# Finds the PVRUtilsVk library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRAssets
#     PVRVk
#     VulkanMemoryAllocator
#     glslang
#     SPIRV

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET PVRAssets)
	find_dependency(PVRAssets REQUIRED MODULE)
endif()

if(NOT TARGET PVRVk)
	find_dependency(PVRVk REQUIRED MODULE)
endif()

if(NOT TARGET VulkanMemoryAllocator)
	find_dependency(VulkanMemoryAllocator REQUIRED MODULE)
endif()

if(NOT TARGET glslang)
	find_dependency(glslang REQUIRED MODULE)
endif()

if(NOT TARGET SPIRV)
	find_dependency(SPIRV REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		set(PVRUtilsVk_DIR "${CMAKE_CURRENT_LIST_DIR}/../../PVRUtils/Vulkan/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/PVRUtilsVk")
	endif()
endif()

if(NOT TARGET PVRUtilsVk)
	find_package(PVRUtilsVk REQUIRED CONFIG)
endif()