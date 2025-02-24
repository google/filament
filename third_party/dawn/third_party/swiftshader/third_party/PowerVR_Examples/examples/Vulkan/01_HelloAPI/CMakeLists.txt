cmake_minimum_required(VERSION 3.3)
project(VulkanHelloAPI)

if(IOS)
	message ("Skipping VulkanHelloAPI : Not supported on iOS")
	return()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/framework/cmake/modules")  #for find_package

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanHelloAPI)

set(HelloAPI_INCLUDE_DIRECTORIES ${SDK_ROOT}/include/)
set(PLATFORM_LIBS ${CMAKE_DL_LIBS})
set(SRC_FILES vk_getProcAddrs.cpp VulkanHelloAPI.cpp vk_getProcAddrs.h VulkanHelloAPI.h VertShader.h FragShader.h VertShader.vsh FragShader.fsh)

if(WIN32)
	list(APPEND SRC_FILES 
		 MainWindows.cpp
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h")
	add_executable(VulkanHelloAPI WIN32 ${SRC_FILES})
	
	# Get rid of the "this function is unsafe" warning in Visual Studio
	target_compile_definitions(VulkanHelloAPI PUBLIC _CRT_SECURE_NO_WARNINGS)
elseif(ANDROID)
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND PLATFORM_LIBS ${lib-android} ${lib-log})
	list(APPEND SRC_FILES 
		MainAndroid.cpp 
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
	add_library(VulkanHelloAPI SHARED ${SRC_FILES})
	# Force export ANativeActivity_onCreate(),
	# Refer to: https://github.com/android-ndk/ndk/issues/381
	set_property(TARGET VulkanHelloAPI APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
elseif(APPLE)
	set(PLATFORM_FOLDER macOS_x86)
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
	set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
	find_library(lib-AppKit AppKit)
	list(APPEND PLATFORM_LIBS ${lib-AppKit})
	find_library(lib-QuartzCore QuartzCore)
	list(APPEND PLATFORM_LIBS ${lib-QuartzCore})
	find_library(lib-Metal Metal)
	list(APPEND PLATFORM_LIBS ${lib-Metal})
	list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")

	if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load") # PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY

	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	find_package(MoltenVK)
	if (${MOLTENVK_FOUND})
		list(APPEND FRAMEWORK_FILES ${MVK_LIBRARIES})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${MVK_INCLUDE_DIR})
	endif()	

	set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
	source_group(Frameworks FILES ${FRAMEWORK_FILES})

	list(APPEND SRC_FILES MainMacOS.mm)

	add_executable(VulkanHelloAPI MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(VulkanHelloAPI PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_FILE}")
	set_target_properties(VulkanHelloAPI PROPERTIES RESOURCE "${ASSET_FILES}")
	
elseif(UNIX)
	if(NOT WS AND NOT PVR_WINDOW_SYSTEM)
		message(FATAL_ERROR "PVR_WINDOW_SYSTEM (Window System) Variable not set. Supported windowing systems can be enabled by passing: -DPVR_WINDOW_SYSTEM=NullWS, -DPVR_WINDOW_SYSTEM=X11, -DPVR_WINDOW_SYSTEM=XCB, -DPVR_WINDOW_SYSTEM=Wayland to CMake")
	endif()
	
	if(WS AND NOT PVR_WINDOW_SYSTEM)
		set(PVR_WINDOW_SYSTEM ${WS})
		message("The WS variable has been deprecated. Please move to using PVR_WINDOW_SYSTEM instead.")
	endif()
	
	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()
	set(BUILD_DEFINITIONS "")
	if(${PVR_WINDOW_SYSTEM} STREQUAL X11) #The following lines add the specified windowing system libraries
		find_package(X11 REQUIRED)
		
		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list(APPEND PLATFORM_LIBS ${X11_LIBRARIES})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
		
		list(APPEND SRC_FILES MainLinuxX11.cpp)
		set(BUILD_DEFINITIONS BUILD_XLIB)
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL XCB) #The following lines add the specified windowing system libraries
		find_package(XCB REQUIRED)
		
		if(NOT ${XCB_FOUND})
			message(FATAL_ERROR "XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
		endif()
		
		list(APPEND PLATFORM_LIBS ${XCB_LIBRARIES})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${XCB_INCLUDE_DIRS})
		
		list(APPEND SRC_FILES MainLinuxXCB.cpp)
		set(BUILD_DEFINITIONS BUILD_XCB)
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		list(APPEND PLATFORM_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
		list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${WAYLAND_CLIENT_INCLUDE_DIR})
		
		list(APPEND SRC_FILES MainLinuxWayland.cpp)
		set(BUILD_DEFINITIONS BUILD_WAYLAND)
	elseif(${PVR_WINDOW_SYSTEM} STREQUAL NullWS)
		list(APPEND SRC_FILES MainNullWS.cpp)
	else()
		message( FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS(default), X11, XCB, Wayland." )
	endif()
	add_executable(VulkanHelloAPI ${SRC_FILES})
	target_compile_definitions(VulkanHelloAPI PUBLIC "${BUILD_DEFINITIONS}")
endif()

set_target_properties(VulkanHelloAPI PROPERTIES
    CXX_STANDARD 14)

target_link_libraries(VulkanHelloAPI ${PLATFORM_LIBS})
target_include_directories(VulkanHelloAPI PRIVATE ${HelloAPI_INCLUDE_DIRECTORIES})
target_compile_definitions(VulkanHelloAPI PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1