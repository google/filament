cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(GNUInstallDirs)
	
	add_library(tinygltf INTERFACE)
	target_include_directories(tinygltf INTERFACE
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

	install(TARGETS tinygltf EXPORT tinygltfTargets)
	install(EXPORT tinygltfTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinygltf OPTIONAL)

	# Export tinygltf from the build tree for use by other projects
	export(EXPORT tinygltfTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/tinygltfTargets.cmake")
	# build tree package config
	file(COPY tinygltfConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tinygltfConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/tinygltfConfig.cmake" COPYONLY)
	
	#set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS tinygltf) - TinyGLTF is an INTERFACE target only
endif()