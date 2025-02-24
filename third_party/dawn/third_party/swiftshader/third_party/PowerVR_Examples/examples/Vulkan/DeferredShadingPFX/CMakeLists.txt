cmake_minimum_required(VERSION 3.3)
project(VulkanDeferredShadingPFX)
if(IOS)
	message ("Skipping VulkanDeferredShadingPFX : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
	find_package(PVRPfx REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanDeferredShadingPFX)

set(SRC_FILES VulkanDeferredShadingPFX.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanDeferredShadingPFX)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanDeferredShadingPFX ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanDeferredShadingPFX
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		effect_MRT_PFX3.pfx)

add_assets_to_target(VulkanDeferredShadingPFX
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		SatyrAndTable.pod
		pointlight.pod
		Marble.pvr
		MarbleNormalMap.pvr
		Table.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanDeferredShadingPFX 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST
		AttributelessVertexShader.vsh
		GBufferFragmentShader.fsh
		GBufferVertexShader.vsh
		GBufferFloorFragmentShader.fsh
		GBufferFloorVertexShader.vsh
		DirectionalLightFragmentShader.fsh
		PointLightPass1FragmentShader.fsh
		PointLightPass1VertexShader.vsh
		PointLightPass2FragmentShader.fsh
		PointLightPass2VertexShader.vsh
		PointLightPass3FragmentShader.fsh
		PointLightPass3VertexShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanDeferredShadingPFX)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanDeferredShadingPFX)

target_link_libraries(VulkanDeferredShadingPFX PUBLIC
	PVRShell
	PVRUtilsVk
	PVRPfx
)