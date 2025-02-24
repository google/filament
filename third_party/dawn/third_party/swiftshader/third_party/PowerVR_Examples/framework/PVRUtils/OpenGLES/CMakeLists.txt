cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRUtilsGles)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../../Common.cmake)

set(PVRUtils_HEADERS 
	../ArialBoldFont.h
	../MultiObject.h
	../PowerVRLogo.h
	../PBRUtils.h
	../PVRUtilsGles.h
	../PVRUtilsTypes.h
	../StructuredMemory.h)

# PVRUtilsGles include files
set(PVRUtilsGles_HEADERS 
	../../../include/DynamicEgl.h
	../../../include/DynamicGles.h
	BindingsGles.h
	ConvertToGlesTypes.h
	ErrorsGles.h
	HelperGles.h
	ModelGles.h
	PBRUtilsGles.h
	ShaderUtilsGles.h
	SpriteGles.h
	TextureUtilsGles.h
	UIRendererGles.h
	UIRendererShaders_ES.h)

# PVRUtilsGles sources
set(PVRUtilsGles_SRC 
	ConvertToGlesTypes.cpp
	ErrorsGles.cpp
	ModelGles.cpp
	PBRUtilsGles.cpp
	ShaderUtilsGles.cpp
	SpriteGles.cpp
	TextureUtilsGles.cpp
	UIRendererGles.cpp)

# Platform specific PVRUtilsGles sources
if (IOS)
	list(APPEND PVRUtilsGles_SRC
		../EAGL/EaglPlatformContext.mm)
	list(APPEND PVRUtilsGles_EAGL_HEADERS
		../EAGL/EaglPlatformHandles.h)
else()
	list(APPEND PVRUtilsGles_SRC
		../EGL/EglPlatformContext.cpp)
	list(APPEND PVRUtilsGles_EGL_HEADERS
		../EGL/EglPlatformContext.h
		../EGL/EglPlatformHandles.h)
endif()

set(PVRUtilsGles_PRIVATE_LINK_LIBS "")
set(PVRUtilsGles_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/../../../include
	${CMAKE_CURRENT_LIST_DIR}/../../)
set(PVRUtilsGles_PRIVATE_INCLUDE_DIRECTORIES "")

# Handle any platform specific api specific libraries
if(ANDROID)
elseif(APPLE)
	if (IOS)
		find_library(lib-gles OpenGLES)
		list(APPEND PVRUtilsGles_PRIVATE_LINK_LIBS ${lib-gles})
	endif()
elseif(UNIX)
	if(PVR_WINDOW_SYSTEM AND ("${PVR_WINDOW_SYSTEM}" STREQUAL Wayland))
		list(APPEND PVRUtilsGles_PRIVATE_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
		list(APPEND PVRUtilsGles_PRIVATE_INCLUDE_DIRECTORIES ${WAYLAND_EGL_INCLUDE_DIR})
	endif()
endif()

# Create the library
add_library(PVRUtilsGles STATIC 
	${PVRUtilsGles_SRC} 
	${PVRUtilsGles_HEADERS}
	${PVRUtils_HEADERS} 
	${PVRUtilsGles_EAGL_HEADERS}
	${PVRUtilsGles_EGL_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
	find_dependency(PVRAssets MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRUtilsGles)

target_link_libraries(PVRUtilsGles 
	PUBLIC
		PVRAssets 
		PVRCore
	PRIVATE
		${PVRUtilsGles_PRIVATE_LINK_LIBS})

if(WS OR PVR_WINDOW_SYSTEM)
	if(WS AND NOT PVR_WINDOW_SYSTEM)
		set(PVR_WINDOW_SYSTEM ${WS})
		message("The WS variable has been deprecated. Please move to using PVR_WINDOW_SYSTEM instead.")
	endif()
	
	set(PVR_WINDOW_SYSTEM_DEFINE "" CACHE INTERNAL "")
	# Validate the use of -DPVR_WINDOW_SYSTEM
	if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB OR ${PVR_WINDOW_SYSTEM} STREQUAL Wayland OR ${PVR_WINDOW_SYSTEM} STREQUAL NullWS OR ${PVR_WINDOW_SYSTEM} STREQUAL Screen)
		set(PVR_WINDOW_SYSTEM_DEFINE "${PVR_WINDOW_SYSTEM}" CACHE INTERNAL "")
	else()
		message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS, X11, XCB, Wayland, Screen.")
	endif()
	target_compile_definitions(PVRUtilsGles PUBLIC ${PVR_WINDOW_SYSTEM_DEFINE})
endif()

if(APPLE)
	target_compile_definitions(PVRUtilsGles PUBLIC GLES_SILENCE_DEPRECATION=1)
endif()

target_include_directories(PVRUtilsGles 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRUtilsGles_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
	
install(TARGETS PVRUtilsGles EXPORT PVRUtilsGlesTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	OPTIONAL)
install(FILES ${PVRUtils_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils OPTIONAL)
install(FILES ${PVRUtilsGles_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/OpenGLES OPTIONAL)
install(FILES ${PVRUtilsGles_EAGL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/EAGL OPTIONAL)
install(FILES ${PVRUtilsGles_EGL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRUtils/EGL OPTIONAL)
install(EXPORT PVRUtilsGlesTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsGles OPTIONAL)

export(EXPORT PVRUtilsGlesTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGles/PVRUtilsGlesTargets.cmake")
# build tree package config
file(COPY cmake/PVRUtilsGlesConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsGles/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRUtilsGles/PVRUtilsGlesConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGles/PVRUtilsGlesConfig.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRUtilsGles/PVRUtilsGlesTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRUtilsGles OPTIONAL)