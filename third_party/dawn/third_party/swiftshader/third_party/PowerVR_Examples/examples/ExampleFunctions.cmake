# Copyright (c) Imagination Technologies Limited.
include(CheckCXXCompilerFlag)

set(SDK_ROOT_INTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR}/.. CACHE INTERNAL "")

option(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS "If enabled, pass /W4 to the compiler and disable specific unreasonable warnings." ON)
option(PVR_ENABLE_EXAMPLE_FAST_MATH "If enabled, attempt to enable fast-math." ON)

# Ensure the examples can find PVRVFrame OpenGL ES Emulation libraries.
if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/Windows_x86_64" CACHE INTERNAL "")
	else()
		set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/Windows_x86_32" CACHE INTERNAL "")
	endif()
elseif(APPLE AND NOT IOS)
	set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/macOS_x86" CACHE INTERNAL "")
elseif(UNIX)
	set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}" CACHE INTERNAL "")
endif()


if(WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		option(PVR_MSVC_ENABLE_EXAMPLE_JUST_MY_CODE "If enabled, enable 'Just My Code' feature." ON)
	endif()
endif()

# Adds the correct executable file (Windows MSVS/MinGW, Linux, MacOS, iOS or libary(Android)
# Optional parameters:
# COMMAND_LINE: Create a command line application instead of a windowed application. Will
#    not function for iOS. Android command line applications are binaries that should be run
#    through a shell (i.e. are not usable through an APK)
# SKIP_ICON_RESOURCES: Do not add the resource files for icons of the app (on non Windows, no effect)
function(add_platform_specific_executable EXECUTABLE_NAME)
	cmake_parse_arguments("ARGUMENTS" "COMMAND_LINE;SKIP_ICON_RESOURCES" "" "" ${ARGN})
	if(WIN32)
		if(ARGUMENTS_COMMAND_LINE)
			add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
		else()
			add_executable(${EXECUTABLE_NAME} WIN32 ${ARGUMENTS_UNPARSED_ARGUMENTS})
		endif()
		if(MINGW)
			# The following fixes "crt0_c.c:(.text.startup+0x2e): undefined reference to `WinMain'" seen when linking with mingw.
			# Without the following mingw fails to find WinMain as it is defined in PVRShell but the symbol has not been marked as an unresolved symbol. The fix is to mark WinMain as unresolved which ensures that WinMain is not stripped. 
			set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS " -u WinMain")
		endif()
	elseif(ANDROID)
		if(NOT ARGUMENTS_COMMAND_LINE)
			add_library(${EXECUTABLE_NAME} SHARED ${ARGUMENTS_UNPARSED_ARGUMENTS})
			# Force export ANativeActivity_onCreate(),
			# Refer to: https://github.com/android-ndk/ndk/issues/381
			set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS " -u ANativeActivity_onCreate")
		else()
			#Remember to also ensure Gradle is configured and called in a way suitable for creating a command line example
			add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
		endif()
	elseif(APPLE)
		if(IOS) 
			if (ARGUMENTS_COMMAND_LINE)
				message(SEND_ERROR "${EXECUTABLE_NAME}: Command line executables not supported on iOS platforms")
			else()
				configure_file(${SDK_ROOT_INTERNAL_DIR}/res/iOS/Entitlements.plist ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Entitlements.plist COPYONLY)
				set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
				file(GLOB EXTRA_RESOURCES LIST_DIRECTORIES false ${SDK_ROOT_INTERNAL_DIR}/res/iOS/* ${SDK_ROOT_INTERNAL_DIR}/res/iOS/OpenGLES/*)
			
				add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${EXTRA_RESOURCES} ${ARGUMENTS_UNPARSED_ARGUMENTS})
				if(CODE_SIGN_IDENTITY)
					set_target_properties (${EXECUTABLE_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}")
				endif()
				if(DEVELOPMENT_TEAM_ID)
					set_target_properties (${EXECUTABLE_NAME} PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}")
				endif()
			endif()
		else()
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/macOS/MainMenu.xib.in ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/MainMenu.xib)
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/macOS/macOS_Info.plist ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/macOS_Info.plist COPYONLY)
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/macOS_Info.plist")
			list(APPEND EXTRA_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/MainMenu.xib")
			
			list(APPEND FRAMEWORK_FILES "${PVR_VFRAME_LIB_FOLDER}/libEGL.dylib" "${PVR_VFRAME_LIB_FOLDER}/libGLESv2.dylib")
			find_package(MoltenVK)
			if(MOLTENVK_FOUND) 
				list(APPEND FRAMEWORK_FILES "${MVK_LIBRARIES}") 
			endif()
			
			set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
			source_group(Frameworks FILES ${FRAMEWORK_FILES})
			add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${EXTRA_RESOURCES} ${ARGUMENTS_UNPARSED_ARGUMENTS} ${FRAMEWORK_FILES})
		endif()
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_FILE}")
		source_group(Resources FILES ${EXTRA_RESOURCES})
		set_source_files_properties(${EXTRA_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	elseif(UNIX OR QNX)
		add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
	endif()
	if(WIN32 AND NOT ARGUMENTS_SKIP_ICON_RESOURCES)
		target_sources(${EXECUTABLE_NAME} PRIVATE
			"${SDK_ROOT_INTERNAL_DIR}/res/Windows/shared.rc"
			"${SDK_ROOT_INTERNAL_DIR}/res/Windows/resource.h")
	endif()
	if (ARGUMENTS_COMMAND_LINE) #Just to use in other places
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES COMMAND_LINE 1)
	endif()
	
	#Add to the global list of targets
	set_property(GLOBAL APPEND PROPERTY PVR_EXAMPLE_TARGETS ${EXECUTABLE_NAME})
endfunction()

function(add_trailing_slash MYPATH)
	if (NOT ${${MYPATH}} MATCHES "/$")
		set(${MYPATH} "${${MYPATH}}/" PARENT_SCOPE)
	endif()
endfunction()

function(mandatory_args FUNCTION_NAME ARG_NAMES)
	foreach(ARG_NAME ${ARG_NAMES})
		if(NOT ${ARG_NAME} AND NOT ARGUMENTS_${ARG_NAME})
			message(FATAL_ERROR "Function ${FUNCTION_NAME}: Mandatory parameter ${ARG_NAME} not defined")
		endif()
	endforeach()
endfunction()

function(unknown_args FUNCTION_NAME ARG_NAMES)
	if(ARG_NAMES)
		message(FATAL_ERROR "Function ${FUNCTION_NAME}: Unknown parameters passed: ${ARG_NAMES}")
	endif()
endfunction()

# Adds an asset to the resource list of TARGET_NAME (windows), and/or add rules to copy it to the asset folder of that target (unix etc.)
# "Assets" in this context are files that we will make available for runtime use to the target. For example, for Windows we can embed them
# into the executable or/end copy them to an asset folder, for UNIX we are copying them to an asset folder, for iOS/MacOS add them to the
# application bundle, for android they are copied into the assets/ folder in the .apk, etc.
# The assets will be copied from BASE_PATH/RELATIVE_PATH, and will be added as resources to the RELATIVE_PATH id for windows, or copied into
# ASSET_FOLDER/RELATIVE_PATH for linux.
#  Usage: add_assets_to_target(<TARGET_NAME> 
#    SOURCE_GROUP <SOURCE_GROUP> 
#    BASE_PATH <BASE_PATH> 
#    ASSET_FOLDER <ASSET_FOLDER> 
#    FILE_LIST <LIST OF RELATIVE_PATH>...)
#After adding all assets, remember to also call add_assets_resource_file(TARGET_NAME)
function(add_assets_to_target TARGET_NAME)
	cmake_parse_arguments(ARGUMENTS "NO_PACKAGE" "SOURCE_GROUP;BASE_PATH;ASSET_FOLDER" "FILE_LIST" ${ARGN})
	mandatory_args(add_assets_to_target BASE_PATH ASSET_FOLDER FILE_LIST)
	unknown_args(add_assets_to_target "${ARGUMENTS_UNPARSED_ARGUMENTS}")

	if(NOT SOURCE_GROUP)
		set(SOURCE_GROUP "assets`")
	endif()
	
	#Add list of all resource files to our target for later use
	add_trailing_slash(ARGUMENTS_ASSET_FOLDER)
	add_trailing_slash(ARGUMENTS_BASE_PATH)
	
	foreach(RELATIVE_PATH ${ARGUMENTS_FILE_LIST})
		set(SOURCE_PATH ${ARGUMENTS_BASE_PATH}${RELATIVE_PATH})
		set(TARGET_PATH ${ARGUMENTS_ASSET_FOLDER}${RELATIVE_PATH})

		source_group(${ARGUMENTS_SOURCE_GROUP} FILES ${SOURCE_PATH})
		target_sources(${TARGET_NAME} PUBLIC ${SOURCE_PATH})

		if (NOT ARGUMENTS_NO_PACKAGE)
			set_property(TARGET ${TARGET_NAME} APPEND PROPERTY SDK_RESOURCE_FILES "${RELATIVE_PATH}|${SOURCE_PATH}")
			if (APPLE)
				source_group(Resources FILES ${SOURCE_PATH})
				set_source_files_properties(${SOURCE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
				set_target_properties(${TARGET_NAME} PROPERTIES RESOURCE ${SOURCE_PATH})
			endif()
			get_target_property(IS_COMMAND_LINE ${TARGET_NAME} COMMAND_LINE)
			if((UNIX OR QNX) AND NOT APPLE AND (IS_COMMAND_LINE OR NOT ANDROID)) # Only copy assets for Unix or Android Command line
				#add a rule to copy the asset
				add_custom_command(
					OUTPUT ${TARGET_PATH} 
					PRE_BUILD 
					MAIN_DEPENDENCY ${SOURCE_PATH}
					COMMAND ${CMAKE_COMMAND} -E remove -f ${TARGET_PATH}
					COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_PATH} ${TARGET_PATH}
					COMMENT "${CMAKE_PROJECT_NAME}: Copying ${SOURCE_PATH} to ${TARGET_PATH}"
				)
			endif()
		endif()
	endforeach()
endfunction()

# Adds a shader that should be compiled to spirv.
# Will create a rule that, at build time, will compile the provided shaders from at BASE_PATH/RELATIVE_PATH into spirv at ASSET_FOLDER/RELATIVE_PATH.
# source group for the generated spirv will be SOURCE_GROUP_generated
# Will also add the spirv generated to the resource list of TARGET_NAME (windows)
#  Usage: add_spirv_shaders_to_target(<TARGET_NAME> <SOURCE_GROUP> <ASSET_FOLDER> <BASE_PATH> <RELATIVE_PATH> [GLSLANG_ARGUMENTS <glslang_arguments>])
#  If the optional GLSLANG_ARGUMENTS argument is provided, it will be passed as the argument to glslangValidator when compiling spir-v
# After adding all assets, remember to also call add_assets_resource_file(TARGET_NAME)
function(add_spirv_shaders_to_target TARGET_NAME)
	cmake_parse_arguments(ARGUMENTS "" "GLSLANG_ARGUMENTS;SOURCE_GROUP;SPIRV_SOURCE_GROUP;BASE_PATH;ASSET_FOLDER;SPIRV_OUTPUT_FOLDER" "FILE_LIST" ${ARGN})
	if(NOT ARGUMENTS_SPIRV_SOURCE_GROUP)
		set(ARGUMENTS_SPIRV_SOURCE_GROUP shaders_generated)
	endif()
	if(NOT ARGUMENTS_SOURCE_GROUP)
		set(ARGUMENTS_SOURCE_GROUP shaders_source)
	endif()
	if(NOT ARGUMENTS_GLSLANG_ARGUMENTS)
		set(ARGUMENTS_GLSLANG_ARGUMENTS -V)
	endif()
	if(NOT ARGUMENTS_SPIRV_OUTPUT_FOLDER)
		if(ANDROID)
			set(ARGUMENTS_SPIRV_OUTPUT_FOLDER ${ARGUMENTS_BASE_PATH})
		else()
			set(ARGUMENTS_SPIRV_OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR})
		endif()
	endif()
	
	mandatory_args(add_spirv_shaders_to_target BASE_PATH ASSET_FOLDER FILE_LIST)
	unknown_args(add_spirv_shaders_to_target "${ARGUMENTS_UNPARSED_ARGUMENTS}")

	#add the source shader files to source groups. Don't package with the app
	add_assets_to_target(${TARGET_NAME}
		SOURCE_GROUP ${ARGUMENTS_SOURCE_GROUP}
		ASSET_FOLDER ${ARGUMENTS_ASSET_FOLDER}
		BASE_PATH ${ARGUMENTS_BASE_PATH}
		FILE_LIST ${ARGUMENTS_FILE_LIST}
		NO_PACKAGE)

	add_trailing_slash(ARGUMENTS_ASSET_FOLDER)
	add_trailing_slash(ARGUMENTS_BASE_PATH)
	add_trailing_slash(ARGUMENTS_SPIRV_OUTPUT_FOLDER)

	unset(TMP_SHADER_LIST)

	foreach(RELATIVE_PATH ${ARGUMENTS_FILE_LIST})
		LIST(APPEND TMP_SHADER_LIST "${RELATIVE_PATH}.spv")
		get_filename_component(SHADER_NAME ${RELATIVE_PATH} NAME)
		get_glslang_validator_type(SHADER_TYPE SHADER_NAME)
		set (GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND glslangValidator ${ARGUMENTS_GLSLANG_ARGUMENTS} ${ARGUMENTS_BASE_PATH}${RELATIVE_PATH} -o ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}${RELATIVE_PATH}.spv -S ${SHADER_TYPE})
		add_custom_command(
			DEPENDS glslangValidator
			OUTPUT ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}/${RELATIVE_PATH}.spv 
			PRE_BUILD 
			MAIN_DEPENDENCY ${ARGUMENTS_BASE_PATH}/${RELATIVE_PATH}
			COMMAND echo ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMAND ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMENT "${PROJECT_NAME}: Compiling ${RELATIVE_PATH} to ${RELATIVE_PATH}.spv"
		)
	endforeach()
	
	#add the final spirv-list of files as resources and package them with the app
	add_assets_to_target(${TARGET_NAME}
		SOURCE_GROUP ${ARGUMENTS_SPIRV_SOURCE_GROUP}
		ASSET_FOLDER ${ARGUMENTS_ASSET_FOLDER}
		BASE_PATH ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}
		FILE_LIST ${TMP_SHADER_LIST}
		)
endfunction()

# Adds a (windows or other resource-based platform) resource file with the resources that were previously added to the target with "add_assets_to_target"
# or "add_spirv_shaders_to_target". It reads a special target property added by those functions: SDK_RESOURCE_FILES, and populates Resources.rc with those files
# This property, if you wish to set it manually, is a list of pipe - separated resource name/resource path pairs, for example: 
# "CarAsset/car.gltf|e:/myfolder/CarAsset/car.gltf;custom_resource_id|e:/anotherfolder/myresource.png..."
function(add_assets_resource_file TARGET_NAME)
	get_property(SDK_RES_LIST TARGET ${TARGET_NAME} PROPERTY SDK_RESOURCE_FILES) #Shorten typing. Global defined/reset by add_platform_specific_executable and populated by add_asset.

	if(WIN32)
		unset(RESOURCE_LIST)
		foreach(TEMP ${SDK_RES_LIST}) #prepare a list in the format we need for Windows resource list
			# Split up the '|' separated name/path pairs, set them in the variable expected by resources.rc.in and populate it
			string(REPLACE "|" ";" PAIR ${TEMP})
			list(GET PAIR 0 RESOURCE_NAME)
			list(GET PAIR 1 RESOURCE_PATH)
			file(RELATIVE_PATH RESOURCE_PATH ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources ${RESOURCE_PATH})
			file(TO_NATIVE_PATH "${RESOURCE_PATH}" RESOURCE_PATH)
			string(REPLACE "\\" "\\\\" RESOURCE_PATH "${RESOURCE_PATH}")
			set(RESOURCE_LIST ${RESOURCE_LIST} "${RESOURCE_NAME} RCDATA \"${RESOURCE_PATH}\"\n")
		endforeach()

		if (NOT(RESOURCE_LIST STREQUAL ""))
			#flatten the list into a string that will be the whole resource block of the file
			string(REPLACE ";" "" RESOURCE_LIST "${RESOURCE_LIST}")
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/Windows/Resources.rc.in ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Resources.rc)
			#Add the resource files needed for windows (icons, asset files etc).
			target_sources(${TARGET_NAME} PUBLIC ${SOURCE_PATH} "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Resources.rc")
		endif()
	endif()
endfunction()

# Helper
function(get_glslang_validator_type out_glslang_validator_type INPUT_SHADER_NAME)
	if(${INPUT_SHADER_NAME} MATCHES ".fsh$")
		set(${out_glslang_validator_type} frag PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".vsh$")
		set(${out_glslang_validator_type} vert PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".csh$")
		set(${out_glslang_validator_type} comp PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".gsh$")
		set(${out_glslang_validator_type} geom PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tcsh$")
		set(${out_glslang_validator_type} tesc PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tesh$")
		set(${out_glslang_validator_type} tese PARENT_SCOPE)
	endif()
endfunction(get_glslang_validator_type)

# Apply various compile/link time options to the specified example target
function(apply_example_compile_options_to_target THETARGET)
	if(WIN32)
		if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS)
				target_compile_options(${THETARGET} PRIVATE
					/W4 
					/wd4127 # Conditional expression is constant.
					/wd4201 # nameless struct/union
					/wd4245 # assignment: signed/unsigned mismatch
					/wd4365 # assignment: signed/unsigned mismatch
					/wd4389 # equality/inequality: signed/unsigned mismatch
					/wd4018 # equality/inequality: signed/unsigned mismatch
					/wd4706 # Assignment within conditional
				)
			endif()
		
			target_compile_options(${THETARGET} PRIVATE "/MP")
			
			if(PVR_MSVC_ENABLE_EXAMPLE_FAST_LINK)
				set_target_properties(${THETARGET} PROPERTIES 
					LINK_OPTIONS "$<$<CONFIG:DEBUG>:/DEBUG:FASTLINK>")
			endif()

			if(PVR_MSVC_ENABLE_EXAMPLE_JUST_MY_CODE)
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
			if(PVR_ENABLE_EXAMPLE_FAST_MATH)
				CHECK_CXX_COMPILER_FLAG(/fp:fast COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "/fp:fast")
				endif()
			endif()
		endif()
	elseif(UNIX)	
		if(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS)
			target_compile_options(${THETARGET} BEFORE PRIVATE -Wall)
		endif()
		
		if(PVR_ENABLE_EXAMPLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "-ffast-math")
				endif()
			endif()
		endif()
	endif()
	
	# Use c++14
	set_target_properties(${THETARGET} PROPERTIES CXX_STANDARD 14)
	
	# Enable Debug and Release flags as appropriate
	target_compile_definitions(${THETARGET} PRIVATE $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1 RELEASE=1>)
endfunction()