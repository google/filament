cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(external)

include(Common.cmake)

# Includes external modules which are dependencies for the PowerVR SDK.
add_subdirectory_if_exists(glm glm)
add_subdirectory_if_exists(pugixml pugixml)
add_subdirectory_if_exists(tinygltf tinygltf)
add_subdirectory_if_exists(PVRScope PVRScope)


if(((${MOLTENVK_FOUND}) OR (NOT APPLE)) AND NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
	add_subdirectory_if_exists(glslang glslang)
	add_subdirectory_if_exists(VulkanMemoryAllocator VulkanMemoryAllocator)
endif()
