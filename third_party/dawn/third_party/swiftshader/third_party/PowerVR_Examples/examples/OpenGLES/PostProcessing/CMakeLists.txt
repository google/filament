cmake_minimum_required(VERSION 3.3)
project(OpenGLESPostProcessing)
if(IOS)
	message("Skipping OpenGLESPostProcessing : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPostProcessing)

set(SRC_FILES OpenGLESPostProcessing.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPostProcessing)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESPostProcessing ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESPostProcessing
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		GaussianBlurFragmentShader.fsh.in
		LinearGaussianBlurVertexShader.vsh.in
		LinearGaussianBlurFragmentShader.fsh.in
		ComputeGaussianBlurHorizontalShader.csh.in
		ComputeGaussianBlurVerticalShader.csh.in)

add_assets_to_target(OpenGLESPostProcessing
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Satyr.pod
		MarbleNormalMap.pvr
		Marble.pvr)

add_assets_to_target(OpenGLESPostProcessing
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes
	FILE_LIST 
		satara_night_scale_0.305_rgb9e5.pvr
		satara_night_scale_0.305_rgb9e5_Irradiance.pvr
		pink_sunrise_rgb9e5.pvr
		pink_sunrise_rgb9e5_Irradiance.pvr
		signal_hill_sunrise_scale_0.312_rgb9e5.pvr
		signal_hill_sunrise_scale_0.312_rgb9e5_Irradiance.pvr)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESPostProcessing 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		Downsample2x2FragShader.fsh
		Downsample2x2VertShader.vsh
		Downsample4x4FragShader.fsh
		Downsample4x4VertShader.vsh
		DualFilterDownSampleFragShader.fsh
		DualFilterDownVertShader.vsh
		DualFilterUpSampleFragShader.fsh
		DualFilterUpSampleMergedFinalPassFragShader.fsh
		DualFilterUpVertShader.vsh
		TentFilterFirstUpSampleFragShader.fsh
		TentFilterUpSampleFragShader.fsh
		TentFilterUpSampleMergedFinalPassFragShader.fsh
		TentFilterUpSampleVertShader.vsh
		FragShader.fsh
		KawaseFragShader.fsh
		KawaseVertShader.vsh
		PostBloomFragShader.fsh
		PostBloomVertShader.vsh
		VertShader.vsh
		SkyboxFragShader.fsh
		SkyboxVertShader.vsh
		GaussianVertShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESPostProcessing)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESPostProcessing)

target_link_libraries(OpenGLESPostProcessing PUBLIC
	PVRShell
	PVRUtilsGles
)