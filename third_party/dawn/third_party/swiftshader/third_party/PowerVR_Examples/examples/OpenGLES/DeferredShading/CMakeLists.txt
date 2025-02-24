cmake_minimum_required(VERSION 3.3)
project(OpenGLESDeferredShading)
if(IOS)
	message("Skipping OpenGLESDeferredShading : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESDeferredShading)

set(SRC_FILES OpenGLESDeferredShading.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESDeferredShading)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESDeferredShading ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESDeferredShading
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		SatyrAndTable.pod
		pointlight.pod
		Marble.pvr
		MarbleNormalMap.pvr
		Table.pvr)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESDeferredShading 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		AttributelessVertexShader.vsh
		DirectionalLightFragmentShader.fsh
		GBufferFloorFragmentShader.fsh
		GBufferFloorVertexShader.vsh
		GBufferFragmentShader.fsh
		GBufferVertexShader.vsh
		PointLightPass1FragmentShader.fsh
		PointLightPass1VertexShader.vsh
		PointLightPass2FragmentShader.fsh
		PointLightPass2VertexShader.vsh
		PointLightPass3FragmentShader.fsh
		PointLightPass3VertexShader.vsh
		WritePlsToFbo.fsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESDeferredShading)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESDeferredShading)

target_link_libraries(OpenGLESDeferredShading PUBLIC
	PVRShell
	PVRUtilsGles
)