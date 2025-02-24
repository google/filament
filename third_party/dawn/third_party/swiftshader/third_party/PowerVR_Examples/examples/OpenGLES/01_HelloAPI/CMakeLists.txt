cmake_minimum_required(VERSION 3.3)
project(OpenGLESHelloAPI)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/framework/cmake/modules")  #for find_package

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESHelloAPI)

set(HelloAPI_INCLUDE_DIRECTORIES ${SDK_ROOT}/include/)

set(PLATFORM_LIBS ${CMAKE_DL_LIBS})

if(WIN32)
	set(SRC_FILES 
		 OpenGLESHelloAPI_Windows.cpp
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h")
	add_executable(OpenGLESHelloAPI WIN32 ${SRC_FILES})
	# Get rid of the "this function is unsafe" warning in Visual Studio
	target_compile_definitions(OpenGLESHelloAPI PUBLIC _CRT_SECURE_NO_WARNINGS)
elseif(ANDROID)
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND PLATFORM_LIBS ${lib-android} ${lib-log})
	set(SRC_FILES 
		OpenGLESHelloAPI_Android.cpp 
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
	add_library(OpenGLESHelloAPI SHARED ${SRC_FILES})
	# Force export ANativeActivity_onCreate(),
	# Refer to: https://github.com/android-ndk/ndk/issues/381
	set_property(TARGET OpenGLESHelloAPI APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
elseif(APPLE)
	if(IOS)
		set(SRC_FILES OpenGLESHelloAPI_iOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
		file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/SDK/res/iOS/* ${SDK_ROOT}/SDK/res/iOS/OpenGLES3/*)
		list(APPEND ASSET_FILES ${ICONS})
		find_library(lib-uikit UIKit)
		find_library(lib-gles OpenGLES)
		find_library(lib-foundation Foundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coregraphics CoreGraphics)
		list(APPEND PLATFORM_LIBS ${lib-uikit} ${lib-gles} ${lib-foundation} ${lib-quartz} ${lib-coregraphics})
	else()
		set(PLATFORM_FOLDER macOS_x86)
		set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
		set(SRC_FILES OpenGLESHelloAPI_macOS.mm)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
		list(APPEND FRAMEWORK_FILES "${BUILDS_LIB_FOLDER}/libEGL.dylib" "${BUILDS_LIB_FOLDER}/libGLESv2.dylib")
		set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
		source_group(Frameworks FILES ${FRAMEWORK_FILES})
		find_library(lib-AppKit AppKit)
		list(APPEND PLATFORM_LIBS ${lib-AppKit})
		list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
	endif()
	
	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	add_executable(OpenGLESHelloAPI MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESHelloAPI PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESHelloAPI PROPERTIES RESOURCE "${ASSET_FILES}")
	
	if(IOS)
		target_compile_options(OpenGLESHelloAPI INTERFACE -Wno-return-type)
		set_target_properties(OpenGLESHelloAPI PROPERTIES LINK_OPTIONS "-Wno-return-type")
	endif()
	# Prevent the linker from stripping out the functions that macOS calls reflectively
	set_target_properties(OpenGLESHelloAPI PROPERTIES LINK_OPTIONS "-all_load")
	
	if(IOS)
		if(CODE_SIGN_IDENTITY)
			set_target_properties(OpenGLESHelloAPI PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}")
		endif()
		if(DEVELOPMENT_TEAM_ID)
			set_target_properties(OpenGLESHelloAPI PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}")
		endif()
	endif()
	
elseif(UNIX)
	if(NOT WS AND NOT PVR_WINDOW_SYSTEM)
		message(FATAL_ERROR "PVR_WINDOW_SYSTEM (Window System) Variable not set. Supported windowing systems can be enabled by passing: -DPVR_WINDOW_SYSTEM=NullWS, -DPVR_WINDOW_SYSTEM=X11, -DPVR_WINDOW_SYSTEM=XCB, -DPVR_WINDOW_SYSTEM=Wayland, -DPVR_WINDOW_SYSTEM=Screen to CMake")
	endif()
	
	if(WS AND NOT PVR_WINDOW_SYSTEM)
		set(PVR_WINDOW_SYSTEM ${WS})
		message("The WS variable has been deprecated. Please move to using PVR_WINDOW_SYSTEM instead.")
	endif()
	
	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()
	
	# X11 OR XCB is to be used
	if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB)
		find_package(X11 REQUIRED)
		
		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list(APPEND PLATFORM_LIBS ${X11_LIBRARIES})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
		
		# X11 is to be used
		if(${PVR_WINDOW_SYSTEM} STREQUAL X11)
			set(SRC_FILES OpenGLESHelloAPI_LinuxX11.cpp)
		endif()
		
		# XCB is to be used. Xlib-xcb is also used for XCB
		if(${PVR_WINDOW_SYSTEM} STREQUAL XCB)
			find_package(XCB REQUIRED)
			
			if(NOT ${XCB_FOUND})
				message(FATAL_ERROR "XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
			endif()
			
			find_package(X11_XCB)
			if(NOT ${X11_XCB_FOUND})
				message(FATAL_ERROR "XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11-XCB libraries")
			endif()
			
			list(APPEND PLATFORM_LIBS ${XCB_LIBRARIES} ${X11_XCB_LIBRARIES})
			list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${XCB_INCLUDE_DIRS})
			
			set(SRC_FILES OpenGLESHelloAPI_LinuxXCB.cpp)
		endif()
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		list(APPEND PLATFORM_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${lib-ffi})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_EGL_INCLUDE_DIR})
		
		set(SRC_FILES OpenGLESHelloAPI_LinuxWayland.cpp)
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL NullWS)
		set(SRC_FILES OpenGLESHelloAPI_NullWS.cpp)
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list(APPEND PLATFORM_LIBS "screen")
			set(SRC_FILES OpenGLESHelloAPI_QNXScreen.cpp)
		endif()
	else()
		message( FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS(default), X11, XCB, Wayland, Screen." )
	endif()
	
	add_executable(OpenGLESHelloAPI ${SRC_FILES})
	
	if(CMAKE_SYSTEM_NAME MATCHES "QNX")
		target_compile_options(OpenGLESHelloAPI PUBLIC -Wno-ignored-attributes)
	endif()
endif()

set_target_properties(OpenGLESHelloAPI PROPERTIES
    CXX_STANDARD 14)

target_link_libraries(OpenGLESHelloAPI ${PLATFORM_LIBS})
target_include_directories(OpenGLESHelloAPI PUBLIC ${HelloAPI_INCLUDE_DIRECTORIES})
target_compile_definitions(OpenGLESHelloAPI PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1