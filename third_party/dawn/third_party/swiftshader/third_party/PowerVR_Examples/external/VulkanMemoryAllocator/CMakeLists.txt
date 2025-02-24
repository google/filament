cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	# User Option: Set current VulkanMemoryAllocator version.
	set(VulkanMemoryAllocator_DOWNLOAD_VERSION "v2.3.0" CACHE INTERNAL "The version of Vulkan Memory Allocator to use")
	# User Option: The URL from which to download the VulkanMemoryAllocator.
	set(VulkanMemoryAllocator_URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/${VulkanMemoryAllocator_DOWNLOAD_VERSION}.tar.gz" CACHE INTERNAL "The URL from wich VulkanMemoryAllocator will be downloaded")
	# User option: The hash used for VMA
	set(VulkanMemoryAllocator_ARCHIVE_HASH "1846f2b0f04c485356cd7ce31a02a2be" CACHE INTERNAL "The validation MD5 hash to use for the downloaded VulkanMemoryAllocator archive")


	if(ANDROID)
		# Set the glm source directory
		set(VulkanMemoryAllocator_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/src CACHE INTERNAL "")
		# Set glm download directory - we use a downloads directory to allow us to maintain cleanliness
		set(VulkanMemoryAllocator_DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/downloads/VulkanMemoryAllocator_downloads CACHE INTERNAL "")
	else()
		# Set the VulkanMemoryAllocator source directory
		set(VulkanMemoryAllocator_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")
		# Set VulkanMemoryAllocator download directory - we use a downloads directory to allow us to maintain cleanliness
		set(VulkanMemoryAllocator_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/VulkanMemoryAllocator_downloads CACHE INTERNAL "")
	endif()


	download_external_project("VulkanMemoryAllocator"
		CMAKE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/download"
		SRC_DIR "${VulkanMemoryAllocator_SRC_DIR}"
		DOWNLOAD_DIR "${VulkanMemoryAllocator_DOWNLOAD_DIR}"
		DOWNLOAD_FILENAME "VulkanMemoryAllocator-${VulkanMemoryAllocator_DOWNLOAD_VERSION}.tar.gz"
		URL "${VulkanMemoryAllocator_URL}"
		HASH ${VulkanMemoryAllocator_ARCHIVE_HASH}
		BYPRODUCTS "${VulkanMemoryAllocator_SRC_DIR}/src/vk_mem_alloc.h"
	)

	add_library(VulkanMemoryAllocator INTERFACE)

	target_include_directories(VulkanMemoryAllocator INTERFACE
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<BUILD_INTERFACE:${VulkanMemoryAllocator_SRC_DIR}/src>)

	install(TARGETS VulkanMemoryAllocator EXPORT VulkanMemoryAllocatorTargets)
	install(EXPORT VulkanMemoryAllocatorTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VulkanMemoryAllocator OPTIONAL)

	# Export VulkanMemoryAllocator from the build tree for use by other projects
	export(EXPORT VulkanMemoryAllocatorTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorTargets.cmake")
	# build tree package config
	file(COPY VulkanMemoryAllocatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/VulkanMemoryAllocatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocatorConfig.cmake" COPYONLY)
endif()