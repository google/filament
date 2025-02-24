cmake_minimum_required(VERSION 3.10)
project(PowerVR_SDK)

include(GNUInstallDirs)

option(PVR_BUILD_FRAMEWORK "Build the PowerVR Framework" ON)
option(PVR_PREBUILT_DEPENDENCIES "Indicates that the PowerVR Framework and its dependencies have been prebuilt. Libraries will not be built and will instead be imported. The Examples will still be built" OFF)

# Compile definitions, options that can be set  others
include("${CMAKE_CURRENT_LIST_DIR}/Configuration.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/framework/Common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/examples/ExampleFunctions.cmake")

# Avoids the SDK being added multiple times
if(TARGET PowerVR_SDK)
	return()
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" OFF)
else()
	option(PVR_BUILD_EXAMPLES "Build the PowerVR SDK Examples" ON)
endif()

# Make a top-level project file to build everything
add_library(PowerVR_SDK INTERFACE)

# Default to release if the user passes nothing.
if(NOT CMAKE_BUILD_TYPE) 
	message("-DCMAKE_BUILD_TYPE not defined. Assuming Release")
	set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "CMAKE_BUILD_TYPE - Specifies the build type on single-configuration generators")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install/${INSTALL_SUFFIX} CACHE INTERNAL "")
	# Ensure other projects don't set CMAKE_INSTALL_PREFIX
	set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()

set(GLSLANG_VALIDATOR_INSTALL_DIR "GLSLANG_VALIDATOR-NOTFOUND" CACHE PATH "Path to a prebuilt glslangValidator")

if(NOT GLSLANG_VALIDATOR_INSTALL_DIR)
	set(GLSLANG_VALIDATOR_INSTALL_DIR $ENV{GLSLANG_VALIDATOR_INSTALL_DIR})
endif()

if(APPLE AND NOT IOS)
	find_package(MoltenVK)
endif()

add_subdirectory(external EXCLUDE_FROM_ALL)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	add_subdirectory(framework EXCLUDE_FROM_ALL)
endif()

if(PVR_BUILD_EXAMPLES)
	option(PVR_BUILD_OPENGLES_EXAMPLES "Build the OpenGLES PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_VULKAN_EXAMPLES "Build the Vulkan PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_OPENCL_EXAMPLES "Build the OpenCL PowerVR SDK Examples - PVR_BUILD_EXAMPLES must also be enabled" OFF)
	option(PVR_BUILD_OPENGLES2_EXAMPLES "Only build OpenGL ES 2.0 examples - PVR_BUILD_EXAMPLES must also be enabled. Assumes PVR_BUILD_OPENGLES_EXAMPLES to ON and disables Non-OpenGL ES examples as well." OFF)

	if (PVR_BUILD_OPENGLES2_EXAMPLES)
		set(PVR_BUILD_OPENGLES_EXAMPLES ON) 
	endif()

	if(NOT PVR_BUILD_OPENGLES_EXAMPLES AND NOT PVR_BUILD_VULKAN_EXAMPLES AND NOT PVR_BUILD_OPENCL_EXAMPLES)
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES/CMakeLists.txt)
			set(PVR_BUILD_OPENGLES_EXAMPLES 1)
		endif()
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan/CMakeLists.txt)
			set(PVR_BUILD_VULKAN_EXAMPLES 1)
		endif()
		if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/examples/OpenCL/CMakeLists.txt)
			set(PVR_BUILD_OPENCL_EXAMPLES 1)
		endif()
	endif()

	add_subdirectory(examples)
endif()

get_property(ALL_EXTERNAL_TARGETS GLOBAL PROPERTY PVR_EXTERNAL_TARGETS)
get_property(ALL_FRAMEWORK_TARGETS GLOBAL PROPERTY PVR_FRAMEWORK_TARGETS)
get_property(ALL_EXAMPLE_TARGETS GLOBAL PROPERTY PVR_EXAMPLE_TARGETS)

foreach(TARGET_NAME ${ALL_EXTERNAL_TARGETS} ${ALL_FRAMEWORK_TARGETS} ${ALL_EXAMPLE_TARGETS})
	enable_sdk_options_for_target(${TARGET_NAME})
endforeach()