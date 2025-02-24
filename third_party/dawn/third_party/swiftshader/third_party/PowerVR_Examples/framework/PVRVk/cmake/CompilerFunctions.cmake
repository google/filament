# This file sets up optimal compiler/linker flags for the PowerVR SDK
# This file contains a set of convenient options to set for PVRVk and/or its targets.
include(CheckCXXCompilerFlag)

option(PVR_ENABLE_RECOMMENDED_WARNINGS "If enabled, pass /W4 to the compiler and disable some specific warnings." ON)

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
			
			target_compile_definitions(${THETARGET} 
				PUBLIC
					_STL_EXTRA_DISABLED_WARNINGS=4774\ 
					_CRT_SECURE_NO_WARNINGS)

		endif()

		if(MINGW)
			set(_WIN32_WINNT 0x0600 CACHE INTERNAL "Setting _WIN32_WINNT to 0x0600 for Windows Vista APIs")
			set(WINVER 0x0600 CACHE INTERNAL "Setting WINVER to 0x0600 for Windows Vista APIs")
		
			target_compile_definitions(${THETARGET} PUBLIC _WIN32_WINNT=${_WIN32_WINNT})
			target_compile_definitions(${THETARGET} PUBLIC WINVER=${WINVER})
		endif()

		string(TOUPPER ${THETARGET} THETARGET_UPPERCASE)

		target_compile_definitions(${THETARGET}
			PUBLIC 
				WIN32_LEAN_AND_MEAN=1 
				VC_EXTRALEAN=1 
				NOMINMAX=1
				${THETARGET_UPPERCASE}_PRESENT=1
				)
	elseif(UNIX)
		if(NOT APPLE)
			# Use Gold Linker by default when it is supported
			execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
			if("${ld_version}" MATCHES "GNU gold")
				target_link_libraries(${THETARGET} PUBLIC "-fuse-ld=gold -Wl,--disable-new-dtags")
			endif()
		endif()
		
		if(PVR_ENABLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall)
			target_compile_options(${THETARGET} PUBLIC -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-sign-compare -Wno-reorder)
		endif()
	
		# Make sure we are not getting the "reorder constructor parameters" warning
		target_compile_options(${THETARGET} PRIVATE -Wno-reorder -Wno-strict-aliasing -Wno-unknown-pragmas -Wno-sign-compare)
		
		if(PVR_ENABLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:RELEASE>:-ffast-math>")
				endif()
			endif()
		endif()

		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			target_compile_options(${THETARGET} PRIVATE -Wno-ignored-attributes)
		endif()
	endif()

	# Set the C++ standard to at least CPP14
	get_target_property(PVRVK_CXX_STANDARD ${THETARGET} CXX_STANDARD)
	
	if ((NOT PVRVK_CXX_STANDARD) OR (PVRVK_CXX_STANDARD LESS 14))
	# Use c++14
		set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD 14)
	endif()

	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()