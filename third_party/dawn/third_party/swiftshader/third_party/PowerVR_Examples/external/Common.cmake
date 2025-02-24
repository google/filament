# Copyright (c) Imagination Technologies Limited.

set(EXTERNAL_COMMON_CMAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

# Checks to see if the subdirectory exists, and has not already been included
# Sets a convenience variable for it
# Parameters: Target, folder to add
function(add_subdirectory_if_exists TARGET SUBDIR_FOLDER)
	get_filename_component(SUBDIR_ABS_PATH ${SUBDIR_FOLDER} ABSOLUTE)
	if(EXISTS ${SUBDIR_ABS_PATH}/CMakeLists.txt)
		if (NOT TARGET ${TARGET})
			add_subdirectory(${SUBDIR_FOLDER} EXCLUDE_FROM_ALL)
			set("${TARGET}_EXISTS" 1 CACHE INTERNAL "")
		endif()
	else()
		set("${TARGET}_EXISTS" 0 CACHE INTERNAL "")
	endif()
endfunction(add_subdirectory_if_exists)

# Download an external project. Used in the actual projects' CMakeLists.txt.
function(download_external_project external_project_name)
	cmake_parse_arguments("ARGUMENTS" "" "CMAKE_FILES_DIR;SRC_DIR;DOWNLOAD_DIR;DOWNLOAD_FILENAME;URL;HASH;BYPRODUCTS" "" ${ARGN})
	
	set(external_project_cmake_files_dir ${ARGUMENTS_CMAKE_FILES_DIR})
	set(external_project_src_dir ${ARGUMENTS_SRC_DIR})
	set(external_project_download_dir ${ARGUMENTS_DOWNLOAD_DIR})
	set(external_project_byproducts ${ARGUMENTS_BYPRODUCTS})
	set(external_project_download_name ${ARGUMENTS_DOWNLOAD_FILENAME})
	set(external_project_archive_hash ${ARGUMENTS_HASH})
	set(external_project_url ${ARGUMENTS_URL})
	
	message("* download_external_project: Processing external project [${external_project_name}]")

	option(${external_project_name}_FORCE_DOWNLOAD "Download ${external_project_name} from ${external_project_url} even if already present on disk" OFF)
	option(${external_project_name}_FORCE_UNPACK "Download ${external_project_name} from ${external_project_url} even if already present on disk" OFF)

	if(${external_project_name}_FORCE_UNPACK OR ${external_project_name}_FORCE_DOWNLOAD OR NOT EXISTS "${external_project_download_dir}/${external_project_download_name}")
		message("* download_external_project: [${external_project_name}] - FORCE unpack - Deleting [${external_project_src_dir}] and [${external_project_cmake_files_dir}]")
		file(REMOVE_RECURSE "${external_project_src_dir}")
		file(REMOVE_RECURSE "${external_project_cmake_files_dir}")
		set(CLEAN_TREE ON)
	endif()

	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(NOT ${external_project_name}_FORCE_DOWNLOAD AND EXISTS "${external_project_download_dir}/${external_project_download_name}")
		file(MD5 "${external_project_download_dir}/${external_project_download_name}" FILE_HASH)
		if (external_project_archive_hash AND ("${external_project_archive_hash}" STREQUAL "${FILE_HASH}"))
			set(external_project_url "${external_project_download_dir}/${external_project_download_name}")
			message("* download_external_project: ${external_project_download_name} was found and hash matches, using local file.")
		else()
			if (NOT CLEAN_TREE) #already deleted
				file(REMOVE_RECURSE "${external_project_src_dir}")
				file(REMOVE_RECURSE "${external_project_cmake_files_dir}")
			endif()
			message("* download_external_project: ${external_project_download_name} was found with hash mismatch, will download from original url.")
		endif()
	else()
		message("* download_external_project: ${external_project_download_name} was not found so will download it from provided URL")
	endif()

	# See here for details: https://crascit.com/2015/07/25/cmake-gtest/
	file(COPY ${EXTERNAL_COMMON_CMAKE_DIRECTORY}/external_project_download.cmake.in DESTINATION ${external_project_cmake_files_dir} NO_SOURCE_PERMISSIONS)
	configure_file(${external_project_cmake_files_dir}/external_project_download.cmake.in ${external_project_cmake_files_dir}/CMakeLists.txt)

	message("* external_project: [${external_project_name}] - Executing Generation.")

	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}" .
					WORKING_DIRECTORY "${external_project_cmake_files_dir}" 
					RESULT_VARIABLE download_configure_result 
					OUTPUT_VARIABLE download_configure_output
					ERROR_VARIABLE download_configure_output)
	if(download_configure_result)
		message(FATAL_ERROR "${external_project_name} download configure step failed (${download_configure_result}): ${download_configure_output}")
	endif()

	message("* external_project: [${external_project_name}] - Executing Build.")

	execute_process(COMMAND ${CMAKE_COMMAND} --build .
					WORKING_DIRECTORY "${external_project_cmake_files_dir}"
					RESULT_VARIABLE download_build_result 
					OUTPUT_VARIABLE download_build_output
					ERROR_VARIABLE download_build_output)
	if(download_build_result)
		message(FATAL_ERROR "${external_project_name} download build step failed (${download_build_result}): ${download_build_output}")
	endif()
endfunction(download_external_project)
