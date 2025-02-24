cmake_minimum_required(VERSION 3.10)

# EVERYTHING IN THIS FILE IS BASICALLY OPTIONAL. It is our recommended compiler configuration and some tools. This is applied to all targets via the "enable_sdk_options_for_target"

# Options that can be set
option(PVR_ENABLE_FAST_MATH "If enabled, attempt to enable fast-math." ON)

if(WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		option(PVR_MSVC_ENABLE_LTCG "If enabled, use Link Time Code Generation for non-debug builds." ON)
		option(PVR_MSVC_ENABLE_JUST_MY_CODE "If enabled, enable 'Just My Code' feature." ON)
		option(PVR_MSVC_USE_STATIC_RUNTIME "If enabled, build against the static, rather than the dynamic, runtime." OFF)
	endif()
elseif(UNIX)
	option(PVR_UNIX_USE_GOLD_LINKER "If enabled, use the Gold linker instead of ld if available." ON)
endif()

function(enable_sdk_options_for_target THETARGET)
	if (NOT TARGET ${THETARGET})
		return()
	endif()
	if (WIN32)
		if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_MSVC_ENABLE_LTCG)
				target_compile_options(${THETARGET} PUBLIC "$<$<NOT:$<CONFIG:DEBUG>>:/GL>")

				# Enable Link Time Code Generation/Whole program optimization. Not using a billion better ways to do this
				# because android only uses 3.10 and there is no way to do it correctly with target_link_options and generator expressions.
				# From CMake 3.13 onward this should be : target_link_options(${THETARGET} PUBLIC "$<$<NOT:$<CONFIG:DEBUG>>:/LTCG:INCREMENTAL>")
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_RELEASE "/LTCG:INCREMENTAL")
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_MINSIZEREL "/LTCG:INCREMENTAL")
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO "/LTCG:INCREMENTAL")
			endif()

			if(PVR_MSVC_ENABLE_JUST_MY_CODE)
				# Enable "Just My Code" feature introduced in MSVC 15.8 (Visual Studio 2017)
				# See https://blogs.msdn.microsoft.com/vcblog/2018/06/29/announcing-jmc-stepping-in-visual-studio/
				#
				# For MSVC version numbering used here, see:
				# https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
				if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
					target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:DEBUG>:/JMC>")
				endif()
			endif()

			# Add the all-important fast-math flag
			if(PVR_ENABLE_FAST_MATH)
				CHECK_CXX_COMPILER_FLAG(/fp:fast COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "/fp:fast")
				endif()
			endif()
		endif()
	elseif(UNIX)
		if(NOT APPLE AND PVR_UNIX_USE_GOLD_LINKER)
			# Use Gold Linker by default when it is supported
			execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
			if("${ld_version}" MATCHES "GNU gold")
				#again, no target link options ... CMake 3.13 onward:
				#target_link_options(${THETARGET} PUBLIC "-fuse-ld=gold -Wl,--disable-new-dtags")
				set_property(TARGET ${THETARGET} APPEND_STRING PROPERTY LINK_FLAGS "-fuse-ld=gold -Wl,--disable-new-dtags")
			endif()
		endif()

		if(PVR_ENABLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "-ffast-math")
				endif()
			endif()
		endif()

	endif()
endfunction()