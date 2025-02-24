cmake_minimum_required(VERSION 3.3)
project(OpenGLESPVRScopeExample)
if(IOS)
	message("Skipping OpenGLESPVRScopeExample : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPVRScopeExample)

set(SRC_FILES OpenGLESPVRScopeExample.cpp PVRScopeGraph.cpp PVRScopeGraph.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPVRScopeExample)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESPVRScopeExample ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESPVRScopeExample
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Marble.pvr
		Satyr.pod)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESPVRScopeExample 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		FragShader_ES2.fsh
		FragShader_ES3.fsh
		VertShader_ES2.vsh
		VertShader_ES3.vsh
		GraphFragShader_ES2.fsh
		GraphFragShader_ES3.fsh
		GraphVertShader_ES2.vsh
		GraphVertShader_ES3.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESPVRScopeExample)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESPVRScopeExample)

target_link_libraries(OpenGLESPVRScopeExample PUBLIC
	PVRShell
	PVRUtilsGles
	PVRScopeDeveloper
)
if(WIN32)
	add_custom_command(TARGET OpenGLESPVRScopeExample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:PVRScopeDeveloperShared> $<TARGET_FILE_DIR:OpenGLESPVRScopeExample>)
endif()