cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRUtilsCL)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../../Common.cmake)

set(PVRUtilsCL_HEADERS
	../../../include/DynamicOCL.h
	OpenCLUtils.h)

add_library(PVRUtilsCL INTERFACE)

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
endif()

set(PVRUtilsCL_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${CMAKE_CURRENT_LIST_DIR}/../../)

target_include_directories(PVRUtilsCL 
	INTERFACE 
		"$<BUILD_INTERFACE:${PVRUtilsCL_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

list(APPEND PVRUtilsCL_LINK_LIBS PVRCore)

if(APPLE AND NOT IOS)
	find_library(lib-opencl OpenCL)
	list(APPEND PVRUtilsCL_LINK_LIBS ${lib-opencl})
endif()

target_link_libraries(PVRUtilsCL 
	INTERFACE
		${PVRUtilsCL_LINK_LIBS})

install(TARGETS PVRUtilsCL EXPORT PVRUtilsCLTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	OPTIONAL)
install(FILES ${PVRUtilsCL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/OpenCL OPTIONAL)
install(EXPORT PVRUtilsCLTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsCL OPTIONAL)

export(EXPORT PVRUtilsCLTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsCL/PVRUtilsCLTargets.cmake")
# build tree package config
file(COPY cmake/PVRUtilsCLConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsCL/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsCL/PVRUtilsCLConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsCL/PVRUtilsCLConfig.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsCL/PVRUtilsCLTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsCL OPTIONAL)