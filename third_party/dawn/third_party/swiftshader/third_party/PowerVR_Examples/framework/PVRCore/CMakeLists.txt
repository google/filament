cmake_minimum_required(VERSION 3.3)
# Copyright (c) Imagination Technologies Limited.

project(PVRCore)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../Common.cmake)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/glm")
	message("GLM not present in ../../external/glm. Skipping parts of PVRCore requiring geometry and math.")
	set(PVRCore_NO_GLM 1)
endif()


if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/pugixml")
	message("PugiXML not present in ../../external/pugixml. Skipping parts of PVRCore that require XML parsing.")
	set(PVRCore_NO_PUGIXML 1)
endif()

# PVRCore include files
set(PVRCore_HEADERS
	Errors.h
	IAssetProvider.h
	Log.h
	PVRCore.h
	RefCounted.h
	Time.cpp
	Time_.h
	Threading.h
	Utils.h
	commandline/CommandLine.h
	stream/BufferStream.h
	stream/FilePath.h
	stream/FileStream.h
	stream/Stream.h
	strings/CompileTimeHash.h
	strings/StringFunctions.h
	strings/StringHash.h
	strings/UnicodeConverter.h
	texture/MetaData.h
	texture/PixelFormat.h
	texture/PVRTDecompress.h
	texture/Texture.h
	texture/TextureDefines.h
	texture/TextureHeader.h
	texture/TextureLoad.h
	texture/TextureLoadAsync.h
	textureio/FileDefinesBMP.h
	textureio/FileDefinesDDS.h
	textureio/FileDefinesKTX.h
	textureio/FileDefinesPVR.h
	textureio/FileDefinesTGA.h
	textureio/FileDefinesXNB.h
	textureio/PaletteExpander.h
	textureio/TextureReaderBMP.h
	textureio/TextureReaderDDS.h
	textureio/TextureReaderKTX.h
	textureio/TextureReaderPVR.h
	textureio/TextureReaderTGA.h
	textureio/TextureReaderXNB.h
	textureio/TextureWriterPVR.h
	textureio/TGAWriter.h
	types/FreeValue.h
	types/GpuDataTypes.h
	types/Types.h)
	
# PVRCore source files
set(PVRCore_SRC
	strings/UnicodeConverter.cpp
	texture/PVRTDecompress.cpp
	texture/Texture.cpp
	texture/TextureHeader.cpp
	textureio/PaletteExpander.cpp
	textureio/TextureReaderBMP.cpp
	textureio/TextureReaderDDS.cpp
	textureio/TextureReaderKTX.cpp
	textureio/TextureReaderPVR.cpp
	textureio/TextureReaderTGA.cpp
	textureio/TextureReaderXNB.cpp
	textureio/TextureWriterPVR.cpp
	Time.cpp)

#Only include math in GLM is present
if (NOT PVRCore_NO_GLM)
	list(APPEND PVRCore_HEADERS
		glm.h
		cameras/CameraKeyFrame.h
		cameras/FPSCamera.h
		cameras/TPSCamera.h
		math/AxisAlignedBox.h
		math/MathUtils.h
		math/Plane.h
		math/Rectangle.h
		math/Rectangle3D.h
		pfx/Effect.h
	)
endif()

#Only include PFXParser if both GLM and PugiXML are present
if (NOT PVRCore_NO_GLM AND NOT PVRCore_NO_PUGIXML)
	list(APPEND PVRCore_HEADERS
		pfx/PFXParser.h
	)
	list(APPEND PVRCore_SRC
		pfx/PFXParser.cpp
	)
endif()

# Add platform specific PVRCore sources
if(WIN32)
	list(APPEND PVRCore_HEADERS
	Windows/WindowsResourceStream.h)
elseif(ANDROID)
	list(APPEND PVRCore_HEADERS
		Android/AndroidAssetStream.h)
		
	list(APPEND PVRCore_SRC
		Android/AndroidAssetStream.cpp)
endif()


# Create the library
add_library(PVRCore STATIC ${PVRCore_SRC} ${PVRCore_HEADERS})

target_link_libraries(PVRCore PRIVATE ${CMAKE_DL_LIBS})


if (WIN32)
# No extra libraries for windows. We need this if though as if it is an unknown platform we error out
elseif (ANDROID)
	find_library(lib-log log)
	target_link_libraries(PVRCore PRIVATE ${lib-log})
elseif (APPLE)
	if (IOS)
		find_library(lib-foundation Foundation)
		find_library(lib-avfoundation AVFoundation)
		target_link_libraries(PVRCore PRIVATE ${lib-foundation} ${lib-avfoundation})
	endif()
elseif (UNIX)
	if(NOT CMAKE_SYSTEM_NAME MATCHES "QNX")
		find_package(Threads)
		target_link_libraries(PVRCore PRIVATE ${CMAKE_THREAD_LIBS_INIT})
		target_link_libraries(PVRCore PRIVATE "rt")
	endif()
else()
	message(FATAL_ERROR "Unknown Platform - Please set this up with platform-specific dependencies")
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake/modules")
	if (NOT PVRCore_NO_GLM)
		find_dependency(glm MODULE REQUIRED)
	endif()
	if (NOT PVRCore_NO_PUGIXML)
		find_dependency(pugixml MODULE REQUIRED)
	endif()
endif()

apply_framework_compile_options_to_target(PVRCore)

if(PVRCore_NO_GLM)
	target_compile_definitions(PVRCore PUBLIC PVRCORE_NO_GLM=1)
else()
	target_link_libraries(PVRCore PUBLIC glm)
endif()

if(PVRCore_NO_PUGIXML)
	target_compile_definitions(PVRCore PUBLIC PVRCORE_NO_PUGIXML=1)
else()
	target_link_libraries(PVRCore PRIVATE pugixml)
endif()

# Set the include directories for PVRCore
target_include_directories(PVRCore 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS PVRCore EXPORT PVRCoreTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	OPTIONAL)

foreach(file ${PVRCore_HEADERS})
	get_filename_component(dir ${file} DIRECTORY)
	install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRCore/${dir})
endforeach()
install(EXPORT PVRCoreTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRCore OPTIONAL)

export(EXPORT PVRCoreTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRCore/PVRCoreTargets.cmake")
# build tree package config
file(COPY cmake/PVRCoreConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRCore/" NO_SOURCE_PERMISSIONS)
configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRCore/PVRCoreConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRCore/PVRCoreConfig.cmake" COPYONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRCore/PVRCoreTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRCore OPTIONAL)