cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRUtilsVk)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../../Common.cmake)

set(PVRUtilsVk_PRIVATE_LINK_LIBS glslang SPIRV)
set(PVRUtilsVk_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/../../)

set(PVRUtils_HEADERS 
	../ArialBoldFont.h
	../MultiObject.h
	../PowerVRLogo.h
	../PBRUtils.h
	../PVRUtilsTypes.h
	../StructuredMemory.h)

# PVRUtilsVk sources
set(PVRUtilsVk_HEADERS
	../ArialBoldFont.h
	../MultiObject.h
	../PowerVRLogo.h
	../PBRUtils.h
	../PVRUtilsTypes.h
	../PVRUtilsVk.h
	../StructuredMemory.h
	AsynchronousVk.h
	ConvertToPVRVkTypes.h
	HelperVk.h
	MemoryAllocator.h
	PBRUtilsVk.h
	PBRUtilsVertShader.h
	PBRUtilsIrradianceFragShader.h
	PBRUtilsPrefilteredFragShader.h
	ShaderUtilsVk.h
	SpriteVk.h
	UIRendererFragShader.h
	UIRendererVertShader.h
	UIRendererVk.h)
	
# PVRUtilsVk sources
set(PVRUtilsVk_SRC
	HelperVk.cpp
	MemoryAllocator.cpp
	PBRUtilsVk.cpp
	ShaderUtilsVk.cpp
	SpriteVk.cpp
	UIRendererVk.cpp)

if(APPLE AND NOT IOS)
	find_package(MoltenVK REQUIRED)
	if (${MOLTENVK_FOUND})
		list(APPEND PVRUtilsVk_PRIVATE_LINK_LIBS ${MVK_LIBRARIES})
		list(APPEND PVRUtilsVk_INCLUDE_DIRECTORIES ${MVK_INCLUDE_DIR})
	endif()
endif()
	
# Create the library
add_library(PVRUtilsVk STATIC 
	${PVRUtilsVk_SRC}
	${PVRUtils_HEADERS}
	${PVRUtilsVk_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
	find_dependency(PVRAssets MODULE REQUIRED)
	find_dependency(PVRVk MODULE REQUIRED)
	find_dependency(glslang MODULE REQUIRED)
	find_dependency(SPIRV MODULE REQUIRED)
	find_dependency(VulkanMemoryAllocator MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRUtilsVk)

target_link_libraries(PVRUtilsVk 
	PUBLIC
		PVRAssets 
		PVRCore
		PVRVk
		VulkanMemoryAllocator
	PRIVATE
		${PVRUtilsVk_PRIVATE_LINK_LIBS})

target_include_directories(PVRUtilsVk 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRUtilsVk_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
	
install(TARGETS PVRUtilsVk EXPORT PVRUtilsVkTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	OPTIONAL)
install(FILES ${PVRUtils_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils OPTIONAL)
install(FILES ${PVRUtilsVk_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/Vulkan OPTIONAL)
#install(EXPORT PVRUtilsVkTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsVk OPTIONAL) # Cannot install(EXPORT PVRUtilsVkTargets...) until glslang properly exports their targets

export(EXPORT PVRUtilsVkTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsVk/PVRUtilsVkTargets.cmake")
# build tree package config
file(COPY cmake/PVRUtilsVkConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsVk/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsVk/PVRUtilsVkConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsVk/PVRUtilsVkConfig.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsVk/PVRUtilsVkTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsVk OPTIONAL)