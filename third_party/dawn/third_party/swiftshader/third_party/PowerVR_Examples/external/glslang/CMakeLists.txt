cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	# User Option: The glslang version to download
	set(glslang_DOWNLOAD_VERSION "8.13.3559" CACHE INTERNAL "The exact version of GLSLANG to download from github")

	# User Option: The URL from where to download glslang.
	set(glslang_URL "https://github.com/KhronosGroup/glslang/archive/${glslang_DOWNLOAD_VERSION}.tar.gz" CACHE INTERNAL "The URL from which to downlad glslang.")
	
	# User option: The hash used for glm
	set(glslang_ARCHIVE_HASH "cb32322377cee2bc1cee5b60ebe46133" CACHE INTERNAL "The validation MD5 hash to use for the downloaded MD5 archive/")
endif()

if (ANDROID)
	# Set glslang download directory - we use a downloads directory to allow us to maintain cleanliness
	set(glslang_DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/downloads/glslang_downloads CACHE INTERNAL "")
else()
	# Set glslang download directory - we use a downloads directory to allow us to maintain cleanliness
	set(glslang_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/glslang_downloads CACHE INTERNAL "")
endif()

# Each file needs a different URL and HASH
if(CMAKE_HOST_WIN32)
	set(HOST_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL "")
	# WORKAROUND - Some unfortunate releases on github seem to be missing windows binaries. Thankfully, this code should probably NOT be used at all as
	set(glslangValidator_PACKAGE_NAME "glslang-master-windows-x64-Release")
	set(glslangValidator_ARCHIVE_HASH "f610b747634118672e941b5c0ec90430")
elseif(CMAKE_HOST_APPLE)
	set(glslangValidator_PACKAGE_NAME "glslang-master-osx-Release")
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
	set(glslangValidator_ARCHIVE_HASH "1d9a9ecf54729342161b8070e961814d")
elseif(CMAKE_HOST_UNIX)
	set(glslangValidator_PACKAGE_NAME "glslang-master-linux-Release")
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
	set(glslangValidator_ARCHIVE_HASH "9c052d0f7b52b0f690ba2a36a45d7384")
endif()

if(PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	set(glslangValidator_BINARY_LOCATION "${PVR_GLSLANG_VALIDATOR_INSTALL_DIR}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
else()
	set(glslangValidator_BINARY_LOCATION "${glslang_DOWNLOAD_DIR}/${glslangValidator_PACKAGE_NAME}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
endif()

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	if (ANDROID)
		# Set glslang download directory - we use a downloads directory to allow us to maintain cleanliness
		set(glslang_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/src CACHE INTERNAL "")
	else()
		# Set the glslang source directory
		set(glslang_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")
	endif()

	# glslang
	download_external_project("glslang"
		CMAKE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/download"
		SRC_DIR "${glslang_SRC_DIR}"
		DOWNLOAD_DIR "${glslang_DOWNLOAD_DIR}"
		DOWNLOAD_FILENAME "glslang-${glslang_DOWNLOAD_VERSION}.tar.gz"
		URL "${glslang_URL}"
		HASH "${glslang_ARCHIVE_HASH}"
		BYPRODUCTS "${glslang_SRC_DIR}/CMakeLists.txt"
	)
	
	# These options are used for both glslang and glslangValidator
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	set(ENABLE_HLSL OFF CACHE INTERNAL "")
	set(BUILD_EXTERNAL OFF CACHE INTERNAL "")

	# If CMAKE_CROSSCOMPILING then do not build glslangValidator using normal add_subdirectory logic
	if(CMAKE_CROSSCOMPILING OR APPLE OR PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
		# See glslangValidator/CMakeLists.txt for how glslangValidator is built when cross-compiling
		set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
	else()
		set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "")
	endif()

	add_subdirectory("${glslang_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")
	
	# If CMAKE_CROSSCOMPILING then we configure glslangValidator separately and import the executable
	if((CMAKE_CROSSCOMPILING OR APPLE) AND NOT PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
		# Set the URL from where to download glslangValidator. Notice, that just changing the version should affect this.
		set(glslangValidator_URL "https://github.com/KhronosGroup/glslang/releases/download/${glslang_DOWNLOAD_VERSION}/${glslangValidator_PACKAGE_NAME}.zip")

		download_external_project("glslangValidator"
			CMAKE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/glslangValidator/download"
			SRC_DIR "${glslang_DOWNLOAD_DIR}/${glslangValidator_PACKAGE_NAME}"
			DOWNLOAD_DIR "${glslang_DOWNLOAD_DIR}"
			DOWNLOAD_FILENAME "${glslangValidator_PACKAGE_NAME}-${glslang_DOWNLOAD_VERSION}.zip"
			URL "${glslangValidator_URL}"
			HASH "${glslangValidator_ARCHIVE_HASH}"
			BYPRODUCTS "${glslangValidator_BINARY_LOCATION}"
		)
	else()
		export(TARGETS glslangValidator FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangValidatorConfig.cmake")
		file(COPY glslangValidatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/glslang/" NO_SOURCE_PERMISSIONS)
		configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/glslang/glslangValidatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangValidatorConfig.cmake" COPYONLY)
	endif()
	
	# Switch the following when glslang properly export their targets.
	#export(EXPORT OGLCompilerTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OGLCompilerConfig.cmake")
	#export(EXPORT OSDependentTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OSDependentConfig.cmake")
	#export(EXPORT glslangTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangConfig.cmake")
	#export(EXPORT SPIRVTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/SPIRVConfig.cmake")
	export(TARGETS OGLCompiler FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OGLCompilerConfig.cmake")
	export(TARGETS OSDependent FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OSDependentConfig.cmake")
	export(TARGETS glslang FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangConfig.cmake")
	export(TARGETS SPIRV FILE "${CMAKE_CURRENT_BINARY_DIR}/build/SPIRVConfig.cmake")
	
	# build tree package config
	file(COPY OGLCompilerConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/OGLCompilerConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/OGLCompilerConfig.cmake" COPYONLY)
	file(COPY OSDependentConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/OSDependentConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/OSDependentConfig.cmake" COPYONLY)
	file(COPY glslangConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangConfig.cmake" COPYONLY)
	file(COPY SPIRVConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/SPIRVConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/SPIRVConfig.cmake" COPYONLY)
	
	set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS glslang)
	# set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS glslangValidator) Nope! This is not built with the SDK!
endif()

if(CMAKE_CROSSCOMPILING OR APPLE OR PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	# Create an import target for glslangValidator so we can use the target name directly
	add_executable(glslangValidator IMPORTED GLOBAL)
	
	set_target_properties(glslangValidator PROPERTIES IMPORTED_LOCATION "${glslangValidator_BINARY_LOCATION}")
endif()
