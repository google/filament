cmake_minimum_required(VERSION 3.3)
project(VulkanExampleUI)
if(IOS)
	message ("Skipping VulkanExampleUI : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanExampleUI)

set(SRC_FILES VulkanExampleUI.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanExampleUI)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanExampleUI ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanExampleUI
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI
	FILE_LIST 
		container-corner.pvr
		container-filler.pvr
		container-vertical.pvr
		container-horizontal.pvr
		topbar.pvr
		background.pvr
		clock-face.pvr
		clock-face-small.pvr
		hand.pvr
		hand-small.pvr
		battery.pvr
		internet-web-browser.pvr
		mail-message-new.pvr
		network-wireless.pvr
		office-calendar.pvr
		weather-sun-cloud.pvr
		weather-storm.pvr
		weather-rain.pvr
		text1.pvr
		text2.pvr
		text-weather.pvr
		text-fri.pvr
		text-sat.pvr
		text-sun.pvr
		text-mon.pvr
		weather-sun-cloud-big.pvr
		window-bottom.pvr
		window-bottomcorner.pvr
		window-side.pvr
		window-top.pvr
		window-topleft.pvr
		window-topright.pvr
		loremipsum.pvr
		vertical-bar.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanExampleUI 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST
		ColShader.fsh
		ColShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanExampleUI)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanExampleUI)

target_link_libraries(VulkanExampleUI PUBLIC
	PVRShell
	PVRUtilsVk
)