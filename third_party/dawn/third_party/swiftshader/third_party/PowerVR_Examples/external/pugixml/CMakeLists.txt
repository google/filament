cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	# User Option: Set current pugixml version. If set, will affect the PugiXML URL
	set(pugixml_DOWNLOAD_VERSION "1.10" CACHE INTERNAL "The version of PugiXML to use")
	# User Option: Set the URL from where to download PugiXML. Notice, that just changing the version should affect this.
	set(pugixml_URL "https://github.com/zeux/pugixml/releases/download/v${pugixml_DOWNLOAD_VERSION}/pugixml-${pugixml_DOWNLOAD_VERSION}.tar.gz" CACHE INTERNAL "PugiXML package Download URL")
	# User option: The hash used for VMA
	set(pugixml_ARCHIVE_HASH "f97237e9908201c6d8536210747b66af" CACHE INTERNAL "The validation MD5 hash for the downloaded PugiXML archive.")

	if(ANDROID)
		# Set the glm source directory
		set(pugixml_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/build-pugixml/.cxx/src CACHE INTERNAL "")
		# Set glm download directory - we use a downloads directory to allow us to maintain cleanliness
		set(pugixml_DOWNLOAD_DIR ${CMAKE_CURRENT_LIST_DIR}/build-android/.cxx/downloads/pugixml_downloads CACHE INTERNAL "")
	else()
		# Set the pugixml source directory
		set(pugixml_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")
		# Set pugixml download directory - we use a downloads directory to allow us to maintain cleanliness
		set(pugixml_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/pugixml_downloads CACHE INTERNAL "")
	endif()

	download_external_project("pugixml"
		CMAKE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/download"
		SRC_DIR "${pugixml_SRC_DIR}"
		DOWNLOAD_DIR "${pugixml_DOWNLOAD_DIR}"
		DOWNLOAD_FILENAME "pugixml-${pugixml_DOWNLOAD_VERSION}.tar.gz"
		URL "${pugixml_URL}"
		HASH ${pugixml_ARCHIVE_HASH}
		BYPRODUCTS "${pugixml_SRC_DIR}/CMakeLists.txt"
	)

	if(EXISTS ${pugixml_SRC_DIR}/tests/)
		# Remove the tests/ folder from pugixml. The use of filenames with unicode characters cause issues in various places. As we are not currently using pugixml tests we'll remove the tests/ directory.
		file(REMOVE_RECURSE ${pugixml_SRC_DIR}/tests/)
	endif()

	add_subdirectory("${pugixml_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")

	# Use c++14 with no extensions
	set_target_properties(pugixml PROPERTIES CXX_STANDARD 14)

	# Export pugixml from the build tree for use by other projects
	export(EXPORT pugixml-config FILE "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake")
	# build tree package config
	file(COPY pugixmlConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/pugixmlConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake" COPYONLY)

	set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS pugixml)
endif()