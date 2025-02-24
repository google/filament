cmake_minimum_required(VERSION 3.3)
project(VulkanGnomeHorde)
if(IOS)
	message ("Skipping VulkanGnomeHorde : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanGnomeHorde)

set(SRC_FILES VulkanGnomeHorde.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanGnomeHorde)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanGnomeHorde ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanGnomeHorde
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeHorde
	FILE_LIST 
		bigMushroom0.pod
		bigMushroom_shadow0.pod
		fern0.pod
		fern_shadow0.pod
		gnome0.pod
		gnome1.pod
		gnome2.pod
		gnome3.pod
		gnome4.pod
		gnome5.pod
		gnome6.pod
		gnome_shadow0.pod
		mushroom0.pod
		mushroom1.pod
		mushroom_shadow0.pod
		rocks0.pod
		bigMushroom_shadow.pvr
		bigMushroom_texture.pvr
		fern.pvr
		fern_shadow.pvr
		gnome_shadow.pvr
		gnome_texture.pvr
		mushroom_shadow.pvr
		mushroom_texture.pvr
		rocks.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanGnomeHorde 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST
		Object.vsh
		Shadow.vsh
		Solid.fsh
		Shadow.fsh
		Plant.fsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanGnomeHorde)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanGnomeHorde)

target_link_libraries(VulkanGnomeHorde PUBLIC
	PVRShell
	PVRUtilsVk
)