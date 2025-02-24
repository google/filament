cmake_minimum_required(VERSION 3.3)
project(VulkanPVRScopeExample)
if(IOS)
	message("Skipping VulkanPVRScopeExample : Not supported on iOS")
	return()
endif()
if(APPLE AND NOT IOS)
	message("Skipping VulkanPVRScopeExample : Not supported on macOS")
	return()
endif()
if(IOS)
	message ("Skipping VulkanPVRScopeExample : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanPVRScopeExample)

set(SRC_FILES VulkanPVRScopeExample.cpp PVRScopeGraph.cpp PVRScopeGraph.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanPVRScopeExample)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanPVRScopeExample ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanPVRScopeExample
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Marble.pvr
		Satyr.pod)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanPVRScopeExample 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST
		FragShader.fsh
		VertShader.vsh
		GraphFragShader.fsh
		GraphVertShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanPVRScopeExample)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanPVRScopeExample)

target_link_libraries(VulkanPVRScopeExample PUBLIC
	PVRShell
	PVRUtilsVk
	PVRScopeDeveloper
)
if(WIN32)
	add_custom_command(TARGET VulkanPVRScopeExample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:PVRScopeDeveloperShared> $<TARGET_FILE_DIR:VulkanPVRScopeExample>)
endif()