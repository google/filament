# This file sets up optimal compiler/linker flags for the PowerVR SDK
# This file contains a set of convenient options to set to the SDK and/or its targets.
include(CheckCXXCompilerFlag)

option(PVR_ENABLE_RECOMMENDED_WARNINGS "If enabled, pass /W4 to the compiler and disable some specific warnings." ON)
set(PVR_CXX_STANDARD "14" CACHE STRING "The CXX standard to build the PowerVR framework. At least 14 is supported.")

if (NOT ("${CMAKE_CURRENT_LIST_DIR}/cmake/modules" IN_LIST CMAKE_MODULE_PATH))
	# CMAKE_MODULE_PATH is used by find_package
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules" CACHE STRING "" FORCE)
endif()

# Checks to see if the subdirectory exists, and has not already been included
# Parameters: Target, folder to add, binary folder (optional)
function(add_subdirectory_if_exists TARGET SUBDIR_FOLDER)
	get_filename_component(SUBDIR_ABS_PATH ${SUBDIR_FOLDER} ABSOLUTE)
	if(EXISTS ${SUBDIR_ABS_PATH}/CMakeLists.txt)
		if (NOT TARGET ${TARGET})
			add_subdirectory(${SUBDIR_FOLDER})
			set(${TARGET}_EXISTS 1 CACHE "" BOOL INTERNAL)
		endif()
	else()
		set(${TARGET}_NOT_EXISTS 1 CACHE "" BOOL INTERNAL)
	endif()
endfunction(add_subdirectory_if_exists)

# Apply various compile/link time options to the specified target
function(apply_framework_compile_options_to_target THETARGET)
	if(WIN32)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_ENABLE_RECOMMENDED_WARNINGS)
				target_compile_options(${THETARGET} 
					PRIVATE /W4 
					PUBLIC 
						/wd4127 # Conditional expression is constant.
						/wd4201 # nameless struct/union
						/wd4245 # assignment: signed/unsigned mismatch
						/wd4365 # assignment: signed/unsigned mismatch
						/wd4389 # equality/inequality: signed/unsigned mismatch
						/wd4018 # equality/inequality: signed/unsigned mismatch
						/wd4706 # Assignment within conditional
				)
			endif()

			string(TOUPPER ${THETARGET} THETARGET_UPPERCASE)

			target_compile_definitions(${THETARGET} 
				PUBLIC
					_STL_EXTRA_DISABLED_WARNINGS=4774\ 
					_CRT_SECURE_NO_WARNINGS
					${THETARGET_UPPERCASE}_PRESENT=1
					)
				
			target_compile_options(${THETARGET} PRIVATE "/MP")
			target_link_libraries(${THETARGET} PUBLIC "$<$<CONFIG:DEBUG>:-incremental>")

		endif()

		if(MINGW)
			set(_WIN32_WINNT 0x0600 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0600 for Windows Vista APIs")
			set(WINVER 0x0600 CACHE INTERNAL "Setting WINVER to 0x0600 for Windows Vista APIs")
		
			target_compile_definitions(${THETARGET} PUBLIC _WIN32_WINNT=${_WIN32_WINNT})
			target_compile_definitions(${THETARGET} PUBLIC WINVER=${WINVER})
		endif()
		
		target_compile_definitions(${THETARGET}
			PUBLIC 
				WIN32_LEAN_AND_MEAN=1 
				VC_EXTRALEAN=1 
				NOMINMAX=1)
	elseif(UNIX)
		if(PVR_ENABLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall -Wno-unused-function)
			target_compile_options(${THETARGET} PUBLIC -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-sign-compare -Wno-reorder)
		endif()
	
		# Make sure we are not getting the "reorder constructor parameters" warning
		target_compile_options(${THETARGET} PRIVATE -Wno-reorder -Wno-strict-aliasing -Wno-unknown-pragmas -Wno-sign-compare)

		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			target_compile_options(${THETARGET} PRIVATE -Wno-ignored-attributes)
		endif()
	endif()
	
	# Use c++14
	set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD ${PVR_CXX_STANDARD})
	
	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()