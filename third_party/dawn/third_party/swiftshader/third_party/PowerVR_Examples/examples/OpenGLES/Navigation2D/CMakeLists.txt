cmake_minimum_required(VERSION 3.3)
project(OpenGLESNavigation2D)

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(pugixml REQUIRED MODULE)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESNavigation2D)

set(SRC_FILES OpenGLESNavigation2D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess2D.cpp ../../common/NavDataProcess.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESNavigation2D)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESNavigation2D ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESNavigation2D
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D
	FILE_LIST 
		map.osm
		ATM.pvr
		bank.pvr
		bar.pvr
		butcher.pvr
		cafe.pvr
		casino.pvr
		cinema.pvr
		college.pvr
		courthouse.pvr
		dentist.pvr
		doctors.pvr
		embassy.pvr
		fastfood.pvr
		fire.pvr
		florist.pvr
		font.pvr
		hairdresser.pvr
		hospital.pvr
		library.pvr
		optician.pvr
		other.pvr
		parking.pvr
		petrol.pvr
		pharmacy.pvr
		police.pvr
		postbox.pvr
		postoffice.pvr
		pub.pvr
		restaurant.pvr
		Road.pvr
		shop.pvr
		theatre.pvr
		toilets.pvr
		university.pvr
		vets.pvr
		worship.pvr)

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESNavigation2D 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		AA_VertShader.vsh
		AA_FragShader.fsh
		VertShader.vsh
		FragShader.fsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESNavigation2D)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESNavigation2D)

target_link_libraries(OpenGLESNavigation2D PUBLIC
	pugixml
	PVRShell
	PVRUtilsGles
)