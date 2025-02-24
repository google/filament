cmake_minimum_required(VERSION 3.3)
project(OpenGLESPVRScopeRemote)
if(IOS)
	message("Skipping OpenGLESPVRScopeRemote : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPVRScopeRemote)

set(SRC_FILES OpenGLESPVRScopeRemote.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPVRScopeRemote)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESPVRScopeRemote ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESPVRScopeRemote
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Marble.pvr
		Satyr.pod)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESPVRScopeRemote 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		FragShader_ES2.fsh
		FragShader_ES3.fsh
		VertShader_ES2.vsh
		VertShader_ES3.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESPVRScopeRemote)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESPVRScopeRemote)

target_link_libraries(OpenGLESPVRScopeRemote PUBLIC
	PVRShell
	PVRUtilsGles
	PVRScopeDeveloper
)
if(WIN32)
	add_custom_command(TARGET OpenGLESPVRScopeRemote POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:PVRScopeDeveloperShared> $<TARGET_FILE_DIR:OpenGLESPVRScopeRemote>)
endif()