cmake_minimum_required(VERSION 3.3)
project(OpenGLESNavigation3D)

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(pugixml REQUIRED MODULE)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESNavigation3D)

set(SRC_FILES OpenGLESNavigation3D.cpp ../../common/NavDataProcess3D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESNavigation3D)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESNavigation3D ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESNavigation3D
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D
	FILE_LIST 
		map.osm
		font.pvr
		Road.pvr)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESNavigation3D 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		AA_VertShader_ES2.vsh
		AA_FragShader_ES2.fsh
		VertShader_ES2.vsh
		FragShader_ES2.fsh
		PlanarShadow_VertShader_ES2.vsh
		PlanarShadow_FragShader_ES2.fsh
		PerVertexLight_VertShader_ES2.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESNavigation3D)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESNavigation3D)

target_link_libraries(OpenGLESNavigation3D PUBLIC
	pugixml
	PVRShell
	PVRUtilsGles
)