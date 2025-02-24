cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRShell)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../Common.cmake)

set(PVRShell_PRIVATE_LINK_LIBS "")

set(PVRShell_HEADERS
	../../include/sdkver.h
	PVRShell.h
	Shell.h
	ShellData.h
	StateMachine.h
	OS/ShellOS.h)
	
set(PVRShell_SRC
	Shell.cpp
	StateMachine.cpp)

# Each platform has its own implementation of PVRShell
if(WIN32)
	list(APPEND PVRShell_HEADERS
		OS/Windows/WindowsOSData.h)
	list(APPEND PVRShell_SRC
		OS/Windows/ShellOS.cpp
		EntryPoint/WinMain/main.cpp)
elseif(APPLE)
	find_library(lib-quartz QuartzCore)
	list(APPEND PVRShell_PRIVATE_LINK_LIBS ${lib-quartz})
	if(IOS)
		list(APPEND PVRShell_HEADERS
			EntryPoint/UIApplicationMain/AppController.h)
		list(APPEND PVRShell_SRC
			EntryPoint/UIApplicationMain/AppController.mm
			EntryPoint/UIApplicationMain/main.m
			OS/UIKit/ShellOS.mm)
			
		find_library(lib-uikit UIKit)
		list(APPEND PVRShell_PRIVATE_LINK_LIBS ${lib-uikit})
	else()
		list(APPEND PVRShell_HEADERS
			EntryPoint/NSApplicationMain/AppController.h
			OS/AppKit/ViewMTL.h)
		list(APPEND PVRShell_SRC
			EntryPoint/NSApplicationMain/AppController.mm
			EntryPoint/NSApplicationMain/main.m
			OS/AppKit/ShellOS.mm
			OS/AppKit/ViewMTL.mm)
			
		find_library(lib-Metal Metal)
		find_library(lib-appkit AppKit)
		list(APPEND PVRShell_PRIVATE_LINK_LIBS ${lib-appkit} ${lib-Metal})
	endif()
elseif(ANDROID)
	find_library(lib-android android)
	list(APPEND PVRShell_PRIVATE_LINK_LIBS ${lib-android})
	list(APPEND PVRShell_SRC
		EntryPoint/android_main/main.cpp
		OS/Android/ShellOS.cpp
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	
	list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
elseif(UNIX)
	if(NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
		list(APPEND PVRShell_OS_HEADERS
			OS/Linux/InternalOS.h)
		list(APPEND PVRShell_SRC
			OS/Linux/InternalOS.cpp)
	endif()

	# We support building for several Windowing Systems. Typical desktop systems support X11 and Wayland is catching on. NullWS is used by some development platforms/testchip.
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

		# X11 OR XCB is to be used
		if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB)
			find_package(X11 REQUIRED)

			if(${X11_FOUND})
				list(APPEND PVRShell_PRIVATE_LINK_LIBS ${X11_LIBRARIES})
				list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
			else()
				message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
			endif()
			
			# XCB is to be used
			if(${PVR_WINDOW_SYSTEM} STREQUAL XCB)
				find_package(XCB REQUIRED)
				
				if(${XCB_FOUND})
					# Xlib-xcb is also used for XCB specifically for OpenGLES
					find_package(X11_XCB)
				
					if(NOT ${X11_XCB_FOUND})
						message(FATAL_ERROR "X11_XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
					endif()

					# Add the XCB and Xlib-xcb libraries and include directories
					list(APPEND PVRShell_PRIVATE_LINK_LIBS ${XCB_LIBRARIES} ${X11_XCB_LIBRARIES})
					list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${XCB_INCLUDE_DIRS})
				else()
					message(FATAL_ERROR "XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
				endif()
			endif()
		elseif(${PVR_WINDOW_SYSTEM} STREQUAL Wayland) # The user has requested the Wayland libraries
			find_package(Wayland REQUIRED)
			
			if(NOT ${WAYLAND_FOUND})
				message(FATAL_ERROR "Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
			endif()
			
			find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
			
			list(APPEND PVRShell_PRIVATE_LINK_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
			list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${WAYLAND_CLIENT_INCLUDE_DIR})
		elseif(${PVR_WINDOW_SYSTEM} STREQUAL Screen)
			if(CMAKE_SYSTEM_NAME MATCHES "QNX")
				list(APPEND PVRShell_PRIVATE_LINK_LIBS "screen")
			endif()
		elseif(${PVR_WINDOW_SYSTEM} STREQUAL NullWS) # The user has requested no windowing system(direct-to-framebuffer)
		else()
			message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS(default), X11, Wayland, Screen." )
		endif()

		if("${PVR_WINDOW_SYSTEM}" STREQUAL "NullWS")
			message("CMake: Generating PVRShell for NullWS")
			if(CMAKE_SYSTEM_NAME MATCHES "QNX")
				list(APPEND PVRShell_SRC
					OS/QNXNullWS/ShellOS.cpp
					EntryPoint/main/main.cpp)
			else()
				list(APPEND PVRShell_SRC
					OS/Linux/NullWS/ShellOS.cpp
					EntryPoint/main/main.cpp)
			endif()
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "X11")
			message("CMake: Generating PVRShell for X11")
			list(APPEND PVRShell_SRC
				OS/Linux/X11/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "XCB")
			message("CMake: Generating PVRShell for XCB")
			list(APPEND PVRShell_SRC
				OS/Linux/XCB/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "Wayland")
			message("CMake: Generating PVRShell for Wayland")
			list(APPEND PVRShell_SRC
				OS/Linux/Wayland/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "Screen")
			if(CMAKE_SYSTEM_NAME MATCHES "QNX")
				message("CMake: Generating PVRShell for Screen")
				list(APPEND PVRShell_SRC
					OS/QNXScreen/ShellOS.cpp
					EntryPoint/main/main.cpp)
			else()
				message(FATAL_ERROR "[PVR_WINDOW_SYSTEM] variable set to [${PVR_WINDOW_SYSTEM}] which is only supported on QNX platforms.")
			endif()
		endif()	
	else()
		message(WARNING "PVR_WINDOW_SYSTEM (Window System) Variable has been not set for PVRShell. Supported windowing systems can be enabled by passing: -DPVR_WINDOW_SYSTEM=NullWS, -DPVR_WINDOW_SYSTEM=X11, -DPVR_WINDOW_SYSTEM=XCB, -DPVR_WINDOW_SYSTEM=Wayland, -DPVR_WINDOW_SYSTEM=Screen to CMake")
	endif()
endif()

# Create the library
add_library(PVRShell STATIC ${PVRShell_SRC} ${PVRShell_HEADERS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake/modules")
	find_dependency(PVRCore MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRShell)

target_link_libraries(PVRShell 
	PUBLIC
		PVRCore
	PRIVATE
		${PVRShell_PRIVATE_LINK_LIBS})

if(APPLE)
	target_link_libraries(PVRShell PUBLIC "-ObjC")
endif()

set(PVRShell_INCLUDE_DIRECTORIES 
	${CMAKE_CURRENT_LIST_DIR}/..
	${CMAKE_CURRENT_LIST_DIR}/../../include)

target_include_directories(PVRShell 
	PUBLIC 
		"$<BUILD_INTERFACE:${PVRShell_INCLUDE_DIRECTORIES}>"
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${PVRShell_PRIVATE_INCLUDE_DIRECTORIES})

target_compile_definitions(PVRShell 
	PUBLIC ${PVR_WINDOW_SYSTEM_DEFINE})

install(TARGETS PVRShell EXPORT PVRShellTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	OPTIONAL)
foreach(file ${PVRShell_HEADERS})
	get_filename_component(dir ${file} DIRECTORY)
	install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRShell/${dir})
endforeach()
install(EXPORT PVRShellTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRShell OPTIONAL)

export(EXPORT PVRShellTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRShell/PVRShellTargets.cmake")
# build tree package config
file(COPY cmake/PVRShellConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRShell/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRShell/PVRShellConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRShell/PVRShellConfig.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRShell/PVRShellTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRShell OPTIONAL)