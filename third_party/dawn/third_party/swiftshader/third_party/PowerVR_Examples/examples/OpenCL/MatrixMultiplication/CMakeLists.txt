cmake_minimum_required(VERSION 3.3)
project(OpenCLMatrixMultiplication)
if(IOS)	
	message("Skipping OpenCLMatrixMultiplication : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRCore REQUIRED MODULE)
	find_package(PVRUtilsCL REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenCLMatrixMultiplication)

set(SRC_FILES OpenCLMatrixMultiplication.cpp Matrix.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenCLMatrixMultiplication)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenCLMatrixMultiplication ${SRC_FILES} COMMAND_LINE)

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textual Kernel files (OpenCL) ###
add_assets_to_target(OpenCLMatrixMultiplication 
	SOURCE_GROUP "kernels" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		kernel.cl)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenCLMatrixMultiplication)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenCLMatrixMultiplication)

target_link_libraries(OpenCLMatrixMultiplication PUBLIC
	PVRCore
	PVRUtilsCL
)