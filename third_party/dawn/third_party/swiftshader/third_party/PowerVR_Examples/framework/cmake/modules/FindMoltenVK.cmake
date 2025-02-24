# Try to find Wayland on a Unix system
#
# This will define:
#
#   MOLTENVK_FOUND       - True if MoltenVK is found
#   MVK_LIBRARIES   - Link these to use MoltenVK
#   MVK_INCLUDE_DIR - Include directory for MoltenVK
#   MVK_DEFINITIONS - Compiler flags for using MoltenVK
#
# In addition the following more fine grained variables will be defined:
#
#   MVK_PORT_INCLUDE_DIR		MVK_INCLUDE_DIR				MVK_PORT_LIBRARIES		
#   MVK_DATATYPE_INCLUDE_DIR	MVK_DATATYPE_INCLUDE_DIR	MVK_DATATYPE_LIBRARIES	
#   MVK_MOLTEN_INCLUDE_DIR		MVK_MOLTEN_INCLUDE_DIR		MVK_MOLTEN_LIBRARIES	
#   MVK_VK_INCLUDE_DIR			MVK_VK_INCLUDE_DIR			MVK_VK_LIBRARIES		
#
# Copyright (c) 2013 Martin Gr��lin <mgraesslin@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (APPLE AND NOT IOS)
	if (MVK_INCLUDE_DIR AND MVK_LIBRARIES)
		# In the cache already
		set(MVK_FIND_QUIETLY TRUE)
	endif ()

	# Use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)

	pkg_check_modules(PKG_MVK QUIET MoltenVK)

	set(MVK_DEFINITIONS ${PKG_MVK_CFLAGS})

	if(NOT DEFINED(VULKAN_SDK))
		if(DEFINED ENV{VULKAN_SDK})
			set(VULKAN_SDK "$ENV{VULKAN_SDK}")
			set(VULKAN_SDK_LIB "${VULKAN_SDK}/lib/")
		else()
			message("Warning: VULKAN_SDK ernvironment variable not set.")
		endif()
	endif()
	
	if(DEFINED VULKAN_SDK)
		if(NOT DEFINED MOLTENVK_INCLUDE_DIR)
		set(MOLTENVK_INCLUDE_DIR "${VULKAN_SDK}/../MoltenVK/include")
		endif()
	endif()

	find_path(MVK_PORT_INCLUDE_DIR NAMES vulkan-portability/vk_extx_portability_subset.h HINTS ${PKG_MVK_INCLUDE_DIRS} PATHS ${MVK_INSTALL_PATH} ${MOLTENVK_INCLUDE_DIR})
	find_path(MVK_DATATYPE_INCLUDE_DIR NAMES MoltenVK/mvk_datatypes.h HINTS	${PKG_MVK_INCLUDE_DIRS} PATHS ${MVK_INSTALL_PATH} ${MOLTENVK_INCLUDE_DIR})
	find_path(MVK_MOLTEN_INCLUDE_DIR NAMES MoltenVK/vk_mvk_moltenvk.h HINTS	${PKG_MVK_INCLUDE_DIRS} PATHS ${MVK_INSTALL_PATH} ${MOLTENVK_INCLUDE_DIR})
	find_path(MVK_VK_INCLUDE_DIR NAMES MoltenVK/mvk_vulkan.h HINTS ${PKG_MVK_INCLUDE_DIRS} PATHS ${MVK_INSTALL_PATH} ${MOLTENVK_INCLUDE_DIR})

	find_library(MVK_LIBRARIES NAMES MoltenVK HINTS ${VULKAN_SDK_LIB} ${PKG_MVK_LIBRARY_DIRS} PATHS ${MVK_INSTALL_PATH})

	set(MVK_INCLUDE_DIR ${MVK_PORT_INCLUDE_DIR} ${MVK_DATATYPE_INCLUDE_DIR} ${MVK_MOLTEN_INCLUDE_DIR} ${MVK_VK_INCLUDE_DIR})

	list(REMOVE_DUPLICATES MVK_INCLUDE_DIR)

	include(FindPackageHandleStandardArgs)

	find_package_handle_standard_args(MOLTENVK	DEFAULT_MSG	MVK_LIBRARIES MVK_VK_INCLUDE_DIR MVK_MOLTEN_INCLUDE_DIR MVK_DATATYPE_INCLUDE_DIR MVK_PORT_INCLUDE_DIR)

	message("Vulkan SDK Location: ${VULKAN_SDK}")
	message("Vulkan SDK Lib Location: ${VULKAN_SDK_LIB}")
	message("Vulkan SDK Include Location: ${MOLTENVK_INCLUDE_DIR}")

	mark_as_advanced(
		MVK_LIBRARIES
		MVK_INCLUDE_DIR
		MVK_PORT_INCLUDE_DIR
		MVK_DATATYPE_INCLUDE_DIR
		MVK_MOLTEN_INCLUDE_DIR
		MVK_VK_INCLUDE_DIR
	)

endif()