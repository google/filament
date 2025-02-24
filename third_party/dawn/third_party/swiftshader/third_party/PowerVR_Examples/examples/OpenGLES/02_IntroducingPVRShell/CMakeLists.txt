cmake_minimum_required(VERSION 3.3)
project(OpenGLESIntroducingPVRShell)

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESIntroducingPVRShell)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESIntroducingPVRShell)

set(SRC_FILES 
	${CMAKE_CURRENT_LIST_DIR}/../../../include/DynamicEgl.h
	${CMAKE_CURRENT_LIST_DIR}/../../../include/DynamicGles.h
	${CMAKE_CURRENT_LIST_DIR}/EglContext.h 
	${CMAKE_CURRENT_LIST_DIR}/OpenGLESIntroducingPVRShell.cpp)

if(IOS)
	list(APPEND SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/EaglContext.mm)
else()
	list(APPEND SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/EglContext.cpp)
endif()

add_platform_specific_executable(OpenGLESIntroducingPVRShell ${SRC_FILES})

if(ANDROID)
elseif(APPLE)
	if (IOS)
		find_library(lib-gles OpenGLES)
		list(APPEND OpenGLESIntroducingPVRShell_LINK_LIBS ${lib-gles})
	endif()
elseif(UNIX)
	if(PVR_WINDOW_SYSTEM AND ("${PVR_WINDOW_SYSTEM}" STREQUAL Wayland))
		list(APPEND OpenGLESIntroducingPVRShell_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
		list(APPEND OpenGLESIntroducingPVRShell_INCLUDE_DIRECTORIES ${WAYLAND_EGL_INCLUDE_DIR})
	endif()
endif()

target_include_directories(OpenGLESIntroducingPVRShell 
	PUBLIC 
		${CMAKE_CURRENT_LIST_DIR}/../../../include/
		${OpenGLESIntroducingPVRShell_INCLUDE_DIRECTORIES})

# Apply SDK specific compile and linker options
apply_example_compile_options_to_target(OpenGLESIntroducingPVRShell)

target_link_libraries(OpenGLESIntroducingPVRShell PUBLIC
	PVRShell
	${OpenGLESIntroducingPVRShell_LINK_LIBS}
)
