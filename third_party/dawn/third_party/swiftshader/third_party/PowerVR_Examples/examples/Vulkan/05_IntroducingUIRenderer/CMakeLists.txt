cmake_minimum_required(VERSION 3.3)
project(VulkanIntroducingUIRenderer)
if(IOS)
	message ("Skipping VulkanIntroducingUIRenderer : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanIntroducingUIRenderer)

set(SRC_FILES VulkanIntroducingUIRenderer.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanIntroducingUIRenderer)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanIntroducingUIRenderer ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanIntroducingUIRenderer
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer
	FILE_LIST 
		Text.txt
		titletext.txt
		arial_36.pvr
		starjout_60.pvr
		title_36.pvr
		title_46.pvr
		title_56.pvr)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanIntroducingUIRenderer)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanIntroducingUIRenderer)

target_link_libraries(VulkanIntroducingUIRenderer PUBLIC
	PVRShell
	PVRUtilsVk
)