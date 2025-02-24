cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	# User Option: The glm version to use.
	set(glm_DOWNLOAD_VERSION "0.9.9.4" CACHE INTERNAL "The version of GLM to download and use")
	# User Option: The URL from where to download glm.
	set(glm_URL "https://github.com/g-truc/glm/archive/${glm_DOWNLOAD_VERSION}.tar.gz" CACHE INTERNAL "URL from which to download GLM")
	
	# User option: The hash used for glm
	set(glm_ARCHIVE_HASH "e60bcfb57c53b03e55cb4fc2f2cc58e6" CACHE INTERNAL "The validation MD5 hash to use for the downloaded MD5 archive/")

	if(ANDROID)
		# Set the glm source directory
		set(glm_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/src CACHE INTERNAL "")
		# Set glm download directory - we use a downloads directory to allow us to maintain cleanliness
		set(glm_DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/downloads/glm_downloads CACHE INTERNAL "")
	else()
		# Set the glm source directory
		set(glm_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")
		# Set glm download directory - we use a downloads directory to allow us to maintain cleanliness
		set(glm_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/glm_downloads CACHE INTERNAL "")
	endif()

	download_external_project("glm"
		CMAKE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/download"
		SRC_DIR "${glm_SRC_DIR}"
		DOWNLOAD_DIR "${glm_DOWNLOAD_DIR}"
		DOWNLOAD_FILENAME "glm-${glm_DOWNLOAD_VERSION}.tar.gz"
		URL "${glm_URL}"
		HASH ${glm_ARCHIVE_HASH}
		BYPRODUCTS "${glm_SRC_DIR}/CMakeLists.txt"
	)

	set(GLM_TEST_ENABLE_FAST_MATH ON CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
	set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
	set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")

	# GLM provides a module GNUInstallDirs which overrides default CMake behaviour so ensure GNUInstallDirs is included prior to the add_subdirectory call for glm so that the "default" behaviour is observed
	include(GNUInstallDirs)
	add_subdirectory("${glm_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")

	# Add this target to the list of targets for easier use
	# set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS glm) --DISABLED: GLM is an INTERFACE target
endif()