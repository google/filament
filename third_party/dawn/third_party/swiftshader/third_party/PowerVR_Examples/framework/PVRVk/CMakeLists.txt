cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRVk)

include(GNUInstallDirs)
include(cmake/CompilerFunctions.cmake)

set(PVRVK_USE_PLATFORM "${PVRVK_USE_PLATFORM}" CACHE STRING "A list of Vulkan platforms of the form VK_USE_PLATFORM_* used as compile-time definitions in PVRVk")

# PVRVk include files
set(PVRVk_HEADERS
	ApiObjectsVk.h
	BufferVk.h
	CommandBufferVk.h
	CommandPoolVk.h
	ComputePipelineVk.h
	DebugReportCallbackVk.h
	DebugUtilsMessengerVk.h
	DebugUtilsVk.h
	DescriptorSetVk.h
	DeviceMemoryVk.h
	DeviceVk.h
	DisplayModeVk.h
	DisplayVk.h
	EventVk.h
	ExtensionsVk.h
	FenceVk.h
	ForwardDecObjectsVk.h
	FramebufferVk.h
	GraphicsPipelineVk.h
	HeadersVk.h
	ImageVk.cpp
	ImageVk.h
	InstanceVk.h
	LayersVk.h
	MemoryBarrierVk.h
	PhysicalDeviceVk.h
	PipelineCacheVk.h
	PipelineConfigVk.h
	PipelineLayoutVk.h
	PipelineVk.h
	PopulateCreateInfoVk.h
	PVRVk.h
	pvrvk_vulkan_wrapper.h
	PVRVkObjectBaseVk.h
	QueryPoolVk.h
	QueueVk.h
	RenderPassVk.h
	SamplerVk.h
	SemaphoreVk.h
	ShaderModuleVk.h
	SurfaceVk.h
	SwapchainVk.h
	TypesVk.h)
	  
set(PVRVk_INCLUDES 
	../../include/pvr_openlib.h
	../../include/vk_bindings.h
	../../include/vk_bindings_helper.h)

# PVRVk sources
set(PVRVk_SRC
	BufferVk.cpp
	CommandBufferVk.cpp
	CommandPoolVk.cpp
	DebugReportCallbackVk.cpp
	DebugUtilsMessengerVk.cpp
	DebugUtilsVk.cpp
	DescriptorSetVk.cpp
	DeviceVk.cpp
	DisplayModeVk.cpp
	DisplayVk.cpp
	EventVk.cpp
	ExtensionsVk.cpp
	FenceVk.cpp
	FramebufferVk.cpp
	ImageVk.cpp
	InstanceVk.cpp
	LayersVk.cpp
	PhysicalDeviceVk.cpp
	PipelineLayoutVk.cpp
	QueryPoolVk.cpp
	QueueVk.cpp
	RenderPassVk.cpp
	SamplerVk.cpp
	SemaphoreVk.cpp
	ShaderModuleVk.cpp
	SurfaceVk.cpp
	SwapchainVk.cpp)

set(PVRVk_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../include
	${CMAKE_CURRENT_LIST_DIR}/../
	${MVK_INCLUDE_DIR})

# Create the library
add_library(PVRVk STATIC
	${PVRVk_SRC}
	${PVRVk_HEADERS}
	${PVRVk_INCLUDES}
	)

# Set the include directories for PVRVk
target_include_directories(PVRVk 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRVk_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Determine which platform PVRVk should be built to support
if(PVRVK_USE_PLATFORM)
	# Setup compile definitions
	target_compile_definitions(PVRVk 
		PUBLIC 
			"${PVRVK_USE_PLATFORM}")
endif()

apply_framework_compile_options_to_target(PVRVk)

install(TARGETS PVRVk EXPORT PVRVkTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	OPTIONAL)
# Setup the files to be installed for PVRVk
install(FILES ${PVRVk_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRVk OPTIONAL)
install(FILES ${PVRVk_INCLUDES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include OPTIONAL)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../include/vulkan DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include OPTIONAL)
install(EXPORT PVRVkTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRVk OPTIONAL)

export(EXPORT PVRVkTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRVk/PVRVkTargets.cmake")
# build tree package config
file(COPY cmake/PVRVkConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRVk/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRVk/PVRVkConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRVk/PVRVkConfig.cmake" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRVk/PVRVkTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRVk OPTIONAL)