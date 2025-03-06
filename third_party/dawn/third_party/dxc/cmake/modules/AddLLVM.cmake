include(LLVMProcessSources)
include(LLVM-Config)
find_package(Python3 REQUIRED)

function(llvm_update_compile_flags name)
  get_property(sources TARGET ${name} PROPERTY SOURCES)
  if("${sources}" MATCHES "\\.c(;|$)")
    set(update_src_props ON)
  endif()

  # LLVM_REQUIRES_EH is an internal flag that individual
  # targets can use to force EH
  if(LLVM_REQUIRES_EH OR LLVM_ENABLE_EH) # HLSL Change - allow CLANG_CL to use EH.
    if(NOT (LLVM_REQUIRES_RTTI OR LLVM_ENABLE_RTTI))
      message(AUTHOR_WARNING "Exception handling requires RTTI. Enabling RTTI for ${name}")
      set(LLVM_REQUIRES_RTTI ON)
    endif()
  else()
    if(LLVM_COMPILER_IS_GCC_COMPATIBLE)
      list(APPEND LLVM_COMPILE_FLAGS "-fno-exceptions")
    elseif(MSVC)
      list(APPEND LLVM_COMPILE_DEFINITIONS _HAS_EXCEPTIONS=0)
      list(APPEND LLVM_COMPILE_FLAGS "/EHs-c-")
    endif()
    if (CLANG_CL)
      # FIXME: Remove this once clang-cl supports SEH
      list(APPEND LLVM_COMPILE_DEFINITIONS "GTEST_HAS_SEH=0")
    endif()
  endif()

  # LLVM_REQUIRES_RTTI is an internal flag that individual
  # targets can use to force RTTI
  if(NOT (LLVM_REQUIRES_RTTI OR LLVM_ENABLE_RTTI))
    list(APPEND LLVM_COMPILE_DEFINITIONS GTEST_HAS_RTTI=0)
    if (LLVM_COMPILER_IS_GCC_COMPATIBLE)
      list(APPEND LLVM_COMPILE_FLAGS "-fno-rtti")
    elseif (MSVC)
      list(APPEND LLVM_COMPILE_FLAGS "/GR-")
    endif ()
  endif()

  # HLSL Changes Start
  if (LLVM_ENABLE_EH)
    if (MSVC)
      list(APPEND LLVM_COMPILE_FLAGS "/EHsc")
    else (MSVC)
      # This is just the default exception handling on Linux
    endif (MSVC)
  endif (LLVM_ENABLE_EH)
  if (NOT HLSL_ENABLE_DEBUG_ITERATORS)
    add_definitions(/D_ITERATOR_DEBUG_LEVEL=0)
  endif (NOT HLSL_ENABLE_DEBUG_ITERATORS)
  # HLSL Changes End

  # Assume that;
  #   - LLVM_COMPILE_FLAGS is list.
  #   - PROPERTY COMPILE_FLAGS is string.
  string(REPLACE ";" " " target_compile_flags "${LLVM_COMPILE_FLAGS}")

  if(update_src_props)
    foreach(fn ${sources})
      get_filename_component(suf ${fn} EXT)
      if("${suf}" STREQUAL ".cpp")
        set_property(SOURCE ${fn} APPEND_STRING PROPERTY
          COMPILE_FLAGS "${target_compile_flags}")
      endif()
    endforeach()
  else()
    # Update target props, since all sources are C++.
    set_property(TARGET ${name} APPEND_STRING PROPERTY
      COMPILE_FLAGS "${target_compile_flags}")
  endif()

  set_property(TARGET ${name} APPEND PROPERTY COMPILE_DEFINITIONS ${LLVM_COMPILE_DEFINITIONS})
endfunction()

function(add_llvm_symbol_exports target_name export_file)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(native_export_file "${target_name}.exports")
    add_custom_command(OUTPUT ${native_export_file}
      COMMAND sed -e "s/^/_/" < ${export_file} > ${native_export_file}
      DEPENDS ${export_file}
      VERBATIM
      COMMENT "Creating export file for ${target_name}")
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                 LINK_FLAGS " -Wl,-exported_symbols_list,${CMAKE_CURRENT_BINARY_DIR}/${native_export_file}")
  elseif(LLVM_HAVE_LINK_VERSION_SCRIPT)
    # Gold and BFD ld require a version script rather than a plain list.
    set(native_export_file "${target_name}.exports")
    # FIXME: Don't write the "local:" line on OpenBSD.
    add_custom_command(OUTPUT ${native_export_file}
      COMMAND echo "{" > ${native_export_file}
      COMMAND grep -q "[[:alnum:]]" ${export_file} && echo "  global:" >> ${native_export_file} || :
      COMMAND sed -e "s/$/;/" -e "s/^/    /" < ${export_file} >> ${native_export_file}
      COMMAND echo "  local: *;" >> ${native_export_file}
      COMMAND echo "};" >> ${native_export_file}
      DEPENDS ${export_file}
      VERBATIM
      COMMENT "Creating export file for ${target_name}")
    if (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
      set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                   LINK_FLAGS "  -Wl,-M,${CMAKE_CURRENT_BINARY_DIR}/${native_export_file}")
    else()
      set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                   LINK_FLAGS "  -Wl,--version-script,${CMAKE_CURRENT_BINARY_DIR}/${native_export_file}")
    endif()
  else()
    set(native_export_file "${target_name}.def")

    add_custom_command(OUTPUT ${native_export_file}
      COMMAND ${Python3_EXECUTABLE} -c "import sys;print(''.join(['EXPORTS\\n']+sys.stdin.readlines(),))"
        < ${export_file} > ${native_export_file}
      DEPENDS ${export_file}
      VERBATIM
      COMMENT "Creating export file for ${target_name}")
    set(export_file_linker_flag "${CMAKE_CURRENT_BINARY_DIR}/${native_export_file}")
    if(MSVC)
      set(export_file_linker_flag "/DEF:\"${export_file_linker_flag}\"")
    endif()
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                 LINK_FLAGS " ${export_file_linker_flag}")
  endif()

  add_custom_target(${target_name}_exports DEPENDS ${native_export_file})
  set_target_properties(${target_name}_exports PROPERTIES FOLDER "Misc")

  get_property(srcs TARGET ${target_name} PROPERTY SOURCES)
  foreach(src ${srcs})
    get_filename_component(extension ${src} EXT)
    if(extension STREQUAL ".cpp")
      set(first_source_file ${src})
      break()
    endif()
  endforeach()

  # Force re-linking when the exports file changes. Actually, it
  # forces recompilation of the source file. The LINK_DEPENDS target
  # property only works for makefile-based generators.
  # FIXME: This is not safe because this will create the same target
  # ${native_export_file} in several different file:
  # - One where we emitted ${target_name}_exports
  # - One where we emitted the build command for the following object.
  # set_property(SOURCE ${first_source_file} APPEND PROPERTY
  #   OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${native_export_file})

  set_property(DIRECTORY APPEND
    PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${native_export_file})

  add_dependencies(${target_name} ${target_name}_exports)

  # Add dependency to *_exports later -- CMake issue 14747
  list(APPEND LLVM_COMMON_DEPENDS ${target_name}_exports)
  set(LLVM_COMMON_DEPENDS ${LLVM_COMMON_DEPENDS} PARENT_SCOPE)
endfunction(add_llvm_symbol_exports)

if(NOT WIN32 AND NOT APPLE)
  execute_process(
    COMMAND ${CMAKE_C_COMPILER} -Wl,--version
    OUTPUT_VARIABLE stdout
    ERROR_QUIET
    )
  if("${stdout}" MATCHES "GNU gold")
    set(LLVM_LINKER_IS_GOLD ON)
  endif()
endif()

function(add_link_opts target_name)
  # Don't use linker optimizations in debug builds since it slows down the
  # linker in a context where the optimizations are not important.
  if (NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")

    # Pass -O3 to the linker. This enabled different optimizations on different
    # linkers.
    if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|SunOS" OR WIN32))
      set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                   LINK_FLAGS " -Wl,-O3")
    endif()

    if(LLVM_LINKER_IS_GOLD)
      # With gold gc-sections is always safe.
      set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                   LINK_FLAGS " -Wl,--gc-sections")
      # Note that there is a bug with -Wl,--icf=safe so it is not safe
      # to enable. See https://sourceware.org/bugzilla/show_bug.cgi?id=17704.
    endif()

    if(NOT LLVM_NO_DEAD_STRIP)
      if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        # ld64's implementation of -dead_strip breaks tools that use plugins.
        set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                     LINK_FLAGS " -Wl,-dead_strip")
      elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
        set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                     LINK_FLAGS " -Wl,-z -Wl,discard-unused=sections")
      elseif(NOT WIN32 AND NOT LLVM_LINKER_IS_GOLD)
        # Object files are compiled with -ffunction-data-sections.
        # Versions of bfd ld < 2.23.1 have a bug in --gc-sections that breaks
        # tools that use plugins. Always pass --gc-sections once we require
        # a newer linker.
        set_property(TARGET ${target_name} APPEND_STRING PROPERTY
                     LINK_FLAGS " -Wl,--gc-sections")
      endif()
    endif()
  endif()
endfunction(add_link_opts)

# Set each output directory according to ${CMAKE_CONFIGURATION_TYPES}.
# Note: Don't set variables CMAKE_*_OUTPUT_DIRECTORY any more,
# or a certain builder, for eaxample, msbuild.exe, would be confused.
function(set_output_directory target bindir libdir)
  # Do nothing if *_OUTPUT_INTDIR is empty.
  if("${bindir}" STREQUAL "")
    return()
  endif()

  # moddir -- corresponding to LIBRARY_OUTPUT_DIRECTORY.
  # It affects output of add_library(MODULE).
  if(WIN32 OR CYGWIN)
    # DLL platform
    set(moddir ${bindir})
  else()
    set(moddir ${libdir})
  endif()
  if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
    foreach(build_mode ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${build_mode}" CONFIG_SUFFIX)
      string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} bi ${bindir})
      string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} li ${libdir})
      string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} mi ${moddir})
      set_target_properties(${target} PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${bi})
      set_target_properties(${target} PROPERTIES "ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${li})
      set_target_properties(${target} PROPERTIES "LIBRARY_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${mi})
    endforeach()
  else()
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${bindir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${libdir})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${moddir})
  endif()
endfunction()

# If on Windows and building with MSVC, add the resource script containing the
# VERSIONINFO data to the project.  This embeds version resource information
# into the output .exe or .dll.
# TODO: Enable for MinGW Windows builds too.
#
function(add_windows_version_resource_file OUT_VAR)
  set(sources ${ARGN})
  if (WIN32)
    set(resource_file ${LLVM_SOURCE_DIR}/resources/windows_version_resource.rc)
    if(EXISTS ${resource_file})
      set(sources ${sources} ${resource_file})
      source_group("Resource Files" ${resource_file})
      set(windows_resource_file ${resource_file} PARENT_SCOPE)
    endif()
  endif(WIN32)

  set(${OUT_VAR} ${sources} PARENT_SCOPE)
endfunction(add_windows_version_resource_file)

# set_windows_version_resource_properties(name resource_file...
#   VERSION_MAJOR int
#     Optional major version number (defaults to LLVM_VERSION_MAJOR)
#   VERSION_MINOR int
#     Optional minor version number (defaults to LLVM_VERSION_MINOR)
#   VERSION_PATCHLEVEL int
#     Optional patchlevel version number (defaults to LLVM_VERSION_PATCH)
#   VERSION_STRING
#     Optional version string (defaults to PACKAGE_VERSION)
#   PRODUCT_NAME
#     Optional product name string (defaults to "LLVM")
#   )
function(set_windows_version_resource_properties name resource_file)
  cmake_parse_arguments(ARG
    ""
    "VERSION_MAJOR;VERSION_MINOR;VERSION_PATCHLEVEL;VERSION_STRING;PRODUCT_NAME"
    ""
    ${ARGN})

  if (NOT DEFINED ARG_VERSION_MAJOR)
    set(ARG_VERSION_MAJOR ${LLVM_VERSION_MAJOR})
  endif()

  if (NOT DEFINED ARG_VERSION_MINOR)
    set(ARG_VERSION_MINOR ${LLVM_VERSION_MINOR})
  endif()

  if (NOT DEFINED ARG_VERSION_PATCHLEVEL)
    set(ARG_VERSION_PATCHLEVEL ${LLVM_VERSION_PATCH})
  endif()

  if (NOT DEFINED ARG_VERSION_STRING)
    set(ARG_VERSION_STRING ${PACKAGE_VERSION})
  endif()

  if (NOT DEFINED ARG_PRODUCT_NAME)
    set(ARG_PRODUCT_NAME "LLVM")
  endif()

  if (MSVC)
  set_property(SOURCE ${resource_file}
               PROPERTY COMPILE_FLAGS /nologo)
  endif()
  set_property(SOURCE ${resource_file}
               PROPERTY COMPILE_DEFINITIONS
               "RC_VERSION_FIELD_1=${ARG_VERSION_MAJOR}"
               "RC_VERSION_FIELD_2=${ARG_VERSION_MINOR}"
               "RC_VERSION_FIELD_3=${ARG_VERSION_PATCHLEVEL}"
               "RC_VERSION_FIELD_4=0"
               "RC_FILE_VERSION=\"${ARG_VERSION_STRING}\""
               "RC_INTERNAL_NAME=\"${name}\""
               "RC_PRODUCT_NAME=\"${ARG_PRODUCT_NAME}\""
               "RC_PRODUCT_VERSION=\"${ARG_VERSION_STRING}\"")

  # HLSL change begin - set common version
  if(${HLSL_EMBED_VERSION})
    if (DEFINED resource_file)
      add_dependencies(${name} hlsl_version_autogen)
      set_property(SOURCE ${resource_file}
                  PROPERTY COMPILE_DEFINITIONS
                  "INCLUDE_HLSL_VERSION_FILE=1")
      set_property(SOURCE ${resource_file}
                  PROPERTY COMPILE_OPTIONS
                  "-I" "${HLSL_VERSION_LOCATION}")
    endif (DEFINED resource_file)
  endif(${HLSL_EMBED_VERSION})
  # HLSL change ends
endfunction(set_windows_version_resource_properties)

# llvm_add_library(name sources...
#   SHARED;STATIC
#     STATIC by default w/o BUILD_SHARED_LIBS.
#     SHARED by default w/  BUILD_SHARED_LIBS.
#   MODULE
#     Target ${name} might not be created on unsupported platforms.
#     Check with "if(TARGET ${name})".
#   OUTPUT_NAME name
#     Corresponds to OUTPUT_NAME in target properties.
#   DEPENDS targets...
#     Same semantics as add_dependencies().
#   LINK_COMPONENTS components...
#     Same as the variable LLVM_LINK_COMPONENTS.
#   LINK_LIBS lib_targets...
#     Same semantics as target_link_libraries().
#   ADDITIONAL_HEADERS
#     May specify header files for IDE generators.
#   )
function(llvm_add_library name)
  cmake_parse_arguments(ARG
    "MODULE;SHARED;STATIC"
    "OUTPUT_NAME"
    "ADDITIONAL_HEADERS;DEPENDS;LINK_COMPONENTS;LINK_LIBS;OBJLIBS"
    ${ARGN})
  list(APPEND LLVM_COMMON_DEPENDS ${ARG_DEPENDS})
  if(ARG_ADDITIONAL_HEADERS)
    # Pass through ADDITIONAL_HEADERS.
    set(ARG_ADDITIONAL_HEADERS ADDITIONAL_HEADERS ${ARG_ADDITIONAL_HEADERS})
  endif()
  if(ARG_OBJLIBS)
    set(ALL_FILES ${ARG_OBJLIBS})
  else()
    llvm_process_sources(ALL_FILES ${ARG_UNPARSED_ARGUMENTS} ${ARG_ADDITIONAL_HEADERS})
  endif()

  if(ARG_MODULE)
    if(ARG_SHARED OR ARG_STATIC)
      message(WARNING "MODULE with SHARED|STATIC doesn't make sense.")
    endif()
    if(NOT LLVM_ENABLE_PLUGINS)
      message(STATUS "${name} ignored -- Loadable modules not supported on this platform.")
      return()
    endif()
  else()
    if(BUILD_SHARED_LIBS AND NOT ARG_STATIC)
      set(ARG_SHARED TRUE)
    endif()
    if(NOT ARG_SHARED)
      set(ARG_STATIC TRUE)
    endif()
  endif()

  # Generate objlib
  if(ARG_SHARED AND ARG_STATIC)
    # Generate an obj library for both targets.
    set(obj_name "obj.${name}")
    add_library(${obj_name} OBJECT EXCLUDE_FROM_ALL
      ${ALL_FILES}
      )
    llvm_update_compile_flags(${obj_name})
    set(ALL_FILES "$<TARGET_OBJECTS:${obj_name}>")

    # Do add_dependencies(obj) later due to CMake issue 14747.
    list(APPEND objlibs ${obj_name})

    set_target_properties(${obj_name} PROPERTIES FOLDER "Object Libraries")
  endif()

  if(ARG_SHARED AND ARG_STATIC)
    # static
    set(name_static "${name}_static")
    if(ARG_OUTPUT_NAME)
      set(output_name OUTPUT_NAME "${ARG_OUTPUT_NAME}")
    endif()
    # DEPENDS has been appended to LLVM_COMMON_LIBS.
    llvm_add_library(${name_static} STATIC
      ${output_name}
      OBJLIBS ${ALL_FILES} # objlib
      LINK_LIBS ${ARG_LINK_LIBS}
      LINK_COMPONENTS ${ARG_LINK_COMPONENTS}
      )
    # FIXME: Add name_static to anywhere in TARGET ${name}'s PROPERTY.
    set(ARG_STATIC)
  endif()

  if(ARG_MODULE)
    add_library(${name} MODULE ${ALL_FILES})
  elseif(ARG_SHARED)
    add_windows_version_resource_file(ALL_FILES ${ALL_FILES})
    add_library(${name} SHARED ${ALL_FILES})
  else()
    add_library(${name} STATIC ${ALL_FILES})
  endif()

  if(DEFINED windows_resource_file)
    set_windows_version_resource_properties(${name} ${windows_resource_file})
    set(windows_resource_file ${windows_resource_file} PARENT_SCOPE)
  endif()

  set_output_directory(${name} ${LLVM_RUNTIME_OUTPUT_INTDIR} ${LLVM_LIBRARY_OUTPUT_INTDIR})
  llvm_update_compile_flags(${name})
  add_link_opts( ${name} )
  if(ARG_OUTPUT_NAME)
    set_target_properties(${name}
      PROPERTIES
      OUTPUT_NAME ${ARG_OUTPUT_NAME}
      )
  endif()

  if(ARG_MODULE)
    set_target_properties(${name} PROPERTIES
      PREFIX ""
      SUFFIX ${LLVM_PLUGIN_EXT}
      )
  endif()

  if(ARG_SHARED)
    if(WIN32)
      set_target_properties(${name} PROPERTIES
        PREFIX ""
        )
    endif()

    # HLSL Change Begin - Don't generate so versioned files.
    set_target_properties(${name}
      PROPERTIES
      SOVERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}
      VERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}${LLVM_VERSION_SUFFIX}
      NO_SONAME On)
    if (APPLE)
      set_property(TARGET ${name} APPEND_STRING PROPERTY
                   LINK_FLAGS " -Wl,-install_name,@rpath/${CMAKE_SHARED_LIBRARY_PREFIX}${name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    elseif(UNIX)
      set_property(TARGET ${name} APPEND_STRING PROPERTY
                   LINK_FLAGS " -Wl,-soname,${CMAKE_SHARED_LIBRARY_PREFIX}${name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
    # HLSL Change End - Don't generate so versioned files.
  endif()

  if(ARG_MODULE OR ARG_SHARED)
    # Do not add -Dname_EXPORTS to the command-line when building files in this
    # target. Doing so is actively harmful for the modules build because it
    # creates extra module variants, and not useful because we don't use these
    # macros.
    set_target_properties( ${name} PROPERTIES DEFINE_SYMBOL "" )

    if (LLVM_EXPORTED_SYMBOL_FILE)
      add_llvm_symbol_exports( ${name} ${LLVM_EXPORTED_SYMBOL_FILE} )
    endif()
  endif()

  # Add the explicit dependency information for this library.
  #
  # It would be nice to verify that we have the dependencies for this library
  # name, but using get_property(... SET) doesn't suffice to determine if a
  # property has been set to an empty value.
  get_property(lib_deps GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_${name})

  llvm_map_components_to_libnames(llvm_libs
    ${ARG_LINK_COMPONENTS}
    ${LLVM_LINK_COMPONENTS}
    )

  if(CMAKE_VERSION VERSION_LESS 2.8.12)
    # Link libs w/o keywords, assuming PUBLIC.
    target_link_libraries(${name}
      ${ARG_LINK_LIBS}
      ${lib_deps}
      ${llvm_libs}
      )
  elseif(ARG_STATIC)
    target_link_libraries(${name} INTERFACE
      ${ARG_LINK_LIBS}
      ${lib_deps}
      ${llvm_libs}
      )
  else()
    # We can use PRIVATE since SO knows its dependent libs.
    target_link_libraries(${name} PRIVATE
      ${ARG_LINK_LIBS}
      ${lib_deps}
      ${llvm_libs}
      )
  endif()

  if(LLVM_COMMON_DEPENDS)
    add_dependencies(${name} ${LLVM_COMMON_DEPENDS})
    # Add dependencies also to objlibs.
    # CMake issue 14747 --  add_dependencies() might be ignored to objlib's user.
    foreach(objlib ${objlibs})
      add_dependencies(${objlib} ${LLVM_COMMON_DEPENDS})
    endforeach()
  endif()
endfunction()

macro(add_llvm_library name)
  cmake_parse_arguments(ARG
    "SHARED"
    ""
    ""
    ${ARGN})
  if( BUILD_SHARED_LIBS )
    llvm_add_library(${name} SHARED ${ARGN})
  else()
    llvm_add_library(${name} ${ARGN})
  endif()
  # The gtest libraries should not be installed or exported as a target
  if ("${name}" STREQUAL gtest OR "${name}" STREQUAL gtest_main)
    set(_is_gtest TRUE)
  else()
    set(_is_gtest FALSE)
    set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${name} )
  endif()

  if( EXCLUDE_FROM_ALL )
    set_target_properties( ${name} PROPERTIES EXCLUDE_FROM_ALL ON)
  elseif(NOT _is_gtest)
    if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY OR ${name} STREQUAL "LTO")
      if(ARG_SHARED OR BUILD_SHARED_LIBS)
        if(WIN32 OR CYGWIN)
          set(install_type RUNTIME)
        else()
          set(install_type LIBRARY)
        endif()
      else()
        set(install_type ARCHIVE)
      endif()

      install(TARGETS ${name}
            EXPORT LLVMExports
            ${install_type} DESTINATION lib${LLVM_LIBDIR_SUFFIX}
            COMPONENT ${name})

      if (NOT CMAKE_CONFIGURATION_TYPES)
        add_custom_target(install-${name}
                          DEPENDS ${name}
                          COMMAND "${CMAKE_COMMAND}"
                                  -DCMAKE_INSTALL_COMPONENT=${name}
                                  -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
      endif()
    endif()
    set_property(GLOBAL APPEND PROPERTY LLVM_EXPORTS ${name})
  endif()
  set_target_properties(${name} PROPERTIES FOLDER "Libraries")
endmacro(add_llvm_library name)

macro(add_llvm_loadable_module name)
  llvm_add_library(${name} MODULE ${ARGN})
  if(NOT TARGET ${name})
    # Add empty "phony" target
    add_custom_target(${name})
  else()
    if( EXCLUDE_FROM_ALL )
      set_target_properties( ${name} PROPERTIES EXCLUDE_FROM_ALL ON)
    else()
      if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
        if(WIN32 OR CYGWIN)
          # DLL platform
          set(dlldir "bin")
        else()
          set(dlldir "lib${LLVM_LIBDIR_SUFFIX}")
        endif()
        install(TARGETS ${name}
          EXPORT LLVMExports
          LIBRARY DESTINATION ${dlldir}
          ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX})
      endif()
      set_property(GLOBAL APPEND PROPERTY LLVM_EXPORTS ${name})
    endif()
  endif()

  set_target_properties(${name} PROPERTIES FOLDER "Loadable modules")
endmacro(add_llvm_loadable_module name)


macro(add_llvm_executable name)
  llvm_process_sources( ALL_FILES ${ARGN} )
  add_windows_version_resource_file(ALL_FILES ${ALL_FILES})

  if( EXCLUDE_FROM_ALL )
    add_executable(${name} EXCLUDE_FROM_ALL ${ALL_FILES})
  else()
    add_executable(${name} ${ALL_FILES})
  endif()

  if(DEFINED windows_resource_file)
    set_windows_version_resource_properties(${name} ${windows_resource_file})
  endif()

  llvm_update_compile_flags(${name})
  add_link_opts( ${name} )

  # Do not add -Dname_EXPORTS to the command-line when building files in this
  # target. Doing so is actively harmful for the modules build because it
  # creates extra module variants, and not useful because we don't use these
  # macros.
  set_target_properties( ${name} PROPERTIES DEFINE_SYMBOL "" )

  if (LLVM_EXPORTED_SYMBOL_FILE)
    add_llvm_symbol_exports( ${name} ${LLVM_EXPORTED_SYMBOL_FILE} )
  endif(LLVM_EXPORTED_SYMBOL_FILE)

  set(EXCLUDE_FROM_ALL OFF)
  set_output_directory(${name} ${LLVM_RUNTIME_OUTPUT_INTDIR} ${LLVM_LIBRARY_OUTPUT_INTDIR})
  llvm_config( ${name} ${LLVM_LINK_COMPONENTS} )
  if( LLVM_COMMON_DEPENDS )
    add_dependencies( ${name} ${LLVM_COMMON_DEPENDS} )
  endif( LLVM_COMMON_DEPENDS )
endmacro(add_llvm_executable name)

function(export_executable_symbols target)
  if (NOT MSVC) # MSVC's linker doesn't support exporting all symbols.
    set_target_properties(${target} PROPERTIES ENABLE_EXPORTS 1)
  endif()
endfunction()


set (LLVM_TOOLCHAIN_TOOLS
  llvm-ar
  llvm-objdump
  )

macro(add_llvm_tool name)
  if( NOT LLVM_BUILD_TOOLS )
    set(EXCLUDE_FROM_ALL ON)
  endif()
  add_llvm_executable(${name} ${ARGN})

  list(FIND LLVM_TOOLCHAIN_TOOLS ${name} LLVM_IS_${name}_TOOLCHAIN_TOOL)
  if (LLVM_IS_${name}_TOOLCHAIN_TOOL GREATER -1 OR NOT LLVM_INSTALL_TOOLCHAIN_ONLY)
    if( LLVM_BUILD_TOOLS )
      install(TARGETS ${name}
              EXPORT LLVMExports
              RUNTIME DESTINATION bin
              COMPONENT ${name})

      if (NOT CMAKE_CONFIGURATION_TYPES)
        add_custom_target(install-${name}
                          DEPENDS ${name}
                          COMMAND "${CMAKE_COMMAND}"
                                  -DCMAKE_INSTALL_COMPONENT=${name}
                                  -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
      endif()
    endif()
  endif()
  if( LLVM_BUILD_TOOLS )
    set_property(GLOBAL APPEND PROPERTY LLVM_EXPORTS ${name})
  endif()
  set_target_properties(${name} PROPERTIES FOLDER "Tools")
endmacro(add_llvm_tool name)


macro(add_llvm_example name)
  if( NOT LLVM_BUILD_EXAMPLES )
    set(EXCLUDE_FROM_ALL ON)
  endif()
  add_llvm_executable(${name} ${ARGN})
  if( LLVM_BUILD_EXAMPLES )
    install(TARGETS ${name} RUNTIME DESTINATION examples)
  endif()
  set_target_properties(${name} PROPERTIES FOLDER "Examples")
endmacro(add_llvm_example name)


macro(add_llvm_utility name)
  add_llvm_executable(${name} ${ARGN})
  set_target_properties(${name} PROPERTIES FOLDER "Utils")
  if( LLVM_INSTALL_UTILS )
    install (TARGETS ${name}
      RUNTIME DESTINATION bin
      COMPONENT ${name})
    if (NOT CMAKE_CONFIGURATION_TYPES)
      add_custom_target(install-${name}
                        DEPENDS ${name}
                        COMMAND "${CMAKE_COMMAND}"
                                -DCMAKE_INSTALL_COMPONENT=${name}
                                -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif()
  endif()
endmacro(add_llvm_utility name)


macro(add_llvm_target target_name)
  include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})
  add_llvm_library(LLVM${target_name} ${ARGN})
  set( CURRENT_LLVM_TARGET LLVM${target_name} )
endmacro(add_llvm_target)

# Add external project that may want to be built as part of llvm such as Clang,
# lld, and Polly. This adds two options. One for the source directory of the
# project, which defaults to ${CMAKE_CURRENT_SOURCE_DIR}/${name}. Another to
# enable or disable building it with everything else.
# Additional parameter can be specified as the name of directory.
macro(add_llvm_external_project name)
  set(add_llvm_external_dir "${ARGN}")
  if("${add_llvm_external_dir}" STREQUAL "")
    set(add_llvm_external_dir ${name})
  endif()
  list(APPEND LLVM_IMPLICIT_PROJECT_IGNORE "${CMAKE_CURRENT_SOURCE_DIR}/${add_llvm_external_dir}")
  string(REPLACE "-" "_" nameUNDERSCORE ${name})
  string(TOUPPER ${nameUNDERSCORE} nameUPPER)
  #TODO: Remove this check in a few days once it has circulated through
  # buildbots and people's checkouts (cbieneman - July 14, 2015)
  if("${LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/${add_llvm_external_dir}")
    unset(LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR CACHE)
  endif()
  if(NOT LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR)
    set(LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${add_llvm_external_dir}")
  else()
    set(LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR
        CACHE PATH "Path to ${name} source directory")
  endif()
  if (EXISTS ${LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR}/CMakeLists.txt)
    option(LLVM_EXTERNAL_${nameUPPER}_BUILD
           "Whether to build ${name} as part of LLVM" ON)
    if (LLVM_EXTERNAL_${nameUPPER}_BUILD)
      add_subdirectory(${LLVM_EXTERNAL_${nameUPPER}_SOURCE_DIR} ${add_llvm_external_dir})
    endif()
  endif()
endmacro(add_llvm_external_project)

macro(add_llvm_tool_subdirectory name)
  list(APPEND LLVM_IMPLICIT_PROJECT_IGNORE "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
  add_subdirectory(${name})
endmacro(add_llvm_tool_subdirectory)

macro(ignore_llvm_tool_subdirectory name)
  list(APPEND LLVM_IMPLICIT_PROJECT_IGNORE "${CMAKE_CURRENT_SOURCE_DIR}/${name}")
endmacro(ignore_llvm_tool_subdirectory)

function(add_llvm_implicit_external_projects)
  set(list_of_implicit_subdirs "")
  file(GLOB sub-dirs "${CMAKE_CURRENT_SOURCE_DIR}/*")
  foreach(dir ${sub-dirs})
    if(IS_DIRECTORY "${dir}")
      list(FIND LLVM_IMPLICIT_PROJECT_IGNORE "${dir}" tool_subdir_ignore)
      if( tool_subdir_ignore EQUAL -1
          AND EXISTS "${dir}/CMakeLists.txt")
        get_filename_component(fn "${dir}" NAME)
        list(APPEND list_of_implicit_subdirs "${fn}")
      endif()
    endif()
  endforeach()

  foreach(external_proj ${list_of_implicit_subdirs})
    add_llvm_external_project("${external_proj}")
  endforeach()
endfunction(add_llvm_implicit_external_projects)

# Generic support for adding a unittest.
function(add_unittest test_suite test_name)
  if( NOT LLVM_BUILD_TESTS AND NOT SPIRV_BUILD_TESTS ) # SPIRV change
    set(EXCLUDE_FROM_ALL ON)
  endif()

  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googlemock/include) # HLSL Change - Pulled in from LLVM 4.0
  if (NOT LLVM_ENABLE_THREADS)
    list(APPEND LLVM_COMPILE_DEFINITIONS GTEST_HAS_PTHREAD=0)
  endif ()

  if (SUPPORTS_NO_VARIADIC_MACROS_FLAG)
    list(APPEND LLVM_COMPILE_FLAGS "-Wno-variadic-macros")
  endif ()

  set(LLVM_REQUIRES_RTTI OFF)

  add_llvm_executable(${test_name} ${ARGN})
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
  set_output_directory(${test_name} ${outdir} ${outdir})
  target_link_libraries(${test_name}
    gtest
    gtest_main
    LLVMSupport # gtest needs it for raw_ostream.
    )

  add_dependencies(${test_suite} ${test_name})
  get_target_property(test_suite_folder ${test_suite} FOLDER)
  if (NOT ${test_suite_folder} STREQUAL "NOTFOUND")
    set_property(TARGET ${test_name} PROPERTY FOLDER "${test_suite_folder}")
  endif ()
endfunction()

function(llvm_add_go_executable binary pkgpath)
  cmake_parse_arguments(ARG "ALL" "" "DEPENDS;GOFLAGS" ${ARGN})

  if(LLVM_BINDINGS MATCHES "go")
    # FIXME: This should depend only on the libraries Go needs.
    get_property(llvmlibs GLOBAL PROPERTY LLVM_LIBS)
    set(binpath ${CMAKE_BINARY_DIR}/bin/${binary}${CMAKE_EXECUTABLE_SUFFIX})
    set(cc "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}")
    set(cxx "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
    set(cppflags "")
    get_property(include_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
    foreach(d ${include_dirs})
      set(cppflags "${cppflags} -I${d}")
    endforeach(d)
    set(ldflags "${CMAKE_EXE_LINKER_FLAGS}")
    add_custom_command(OUTPUT ${binpath}
      COMMAND ${CMAKE_BINARY_DIR}/bin/llvm-go "cc=${cc}" "cxx=${cxx}" "cppflags=${cppflags}" "ldflags=${ldflags}"
              ${ARG_GOFLAGS} build -o ${binpath} ${pkgpath}
      DEPENDS llvm-config ${CMAKE_BINARY_DIR}/bin/llvm-go${CMAKE_EXECUTABLE_SUFFIX}
              ${llvmlibs} ${ARG_DEPENDS}
      COMMENT "Building Go executable ${binary}"
      VERBATIM)
    if (ARG_ALL)
      add_custom_target(${binary} ALL DEPENDS ${binpath})
    else()
      add_custom_target(${binary} DEPENDS ${binpath})
    endif()
  endif()
endfunction()

# This function provides an automatic way to 'configure'-like generate a file
# based on a set of common and custom variables, specifically targeting the
# variables needed for the 'lit.site.cfg' files. This function bundles the
# common variables that any Lit instance is likely to need, and custom
# variables can be passed in.
function(configure_lit_site_cfg input output)
  foreach(c ${LLVM_TARGETS_TO_BUILD})
    set(TARGETS_BUILT "${TARGETS_BUILT} ${c}")
  endforeach(c)
  set(TARGETS_TO_BUILD ${TARGETS_BUILT})

  set(SHLIBEXT "${LTDL_SHLIB_EXT}")

  # Configuration-time: See Unit/lit.site.cfg.in
  if (CMAKE_CFG_INTDIR STREQUAL ".")
    set(LLVM_BUILD_MODE ".")
  else ()
    set(LLVM_BUILD_MODE "%(build_mode)s")
  endif ()

  # They below might not be the build tree but provided binary tree.
  set(LLVM_SOURCE_DIR ${LLVM_MAIN_SRC_DIR})
  set(LLVM_BINARY_DIR ${LLVM_BINARY_DIR})
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} LLVM_TOOLS_DIR ${LLVM_TOOLS_BINARY_DIR})
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} LLVM_LIBS_DIR  ${LLVM_LIBRARY_DIR})

  # SHLIBDIR points the build tree.
  string(REPLACE ${CMAKE_CFG_INTDIR} ${LLVM_BUILD_MODE} SHLIBDIR "${LLVM_SHLIB_OUTPUT_INTDIR}")

  # FIXME: "ENABLE_SHARED" doesn't make sense, since it is used just for
  # plugins. We may rename it.
  if(LLVM_ENABLE_PLUGINS)
    set(ENABLE_SHARED "1")
  else()
    set(ENABLE_SHARED "0")
  endif()

  if(LLVM_ENABLE_ASSERTIONS AND NOT MSVC_IDE)
    set(ENABLE_ASSERTIONS "1")
  else()
    set(ENABLE_ASSERTIONS "0")
  endif()

  set(HOST_OS ${CMAKE_SYSTEM_NAME})
  set(HOST_ARCH ${CMAKE_SYSTEM_PROCESSOR})

  set(HOST_CC "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}")
  set(HOST_CXX "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
  set(HOST_LDFLAGS "${CMAKE_EXE_LINKER_FLAGS}")

  configure_file(${input} ${output} @ONLY)
endfunction()

# A raw function to create a lit target. This is used to implement the testuite
# management functions.
function(add_lit_target target comment)
  cmake_parse_arguments(ARG "" "" "PARAMS;DEPENDS;ARGS" ${ARGN})
  set(LIT_ARGS "${ARG_ARGS} ${LLVM_LIT_ARGS}")
  separate_arguments(LIT_ARGS)
  if (NOT CMAKE_CFG_INTDIR STREQUAL ".")
    list(APPEND LIT_ARGS --param build_mode=${CMAKE_CFG_INTDIR})
  endif ()
  if (LLVM_MAIN_SRC_DIR)
    set (LIT_COMMAND ${Python3_EXECUTABLE} ${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py)
  else()
    find_program(LIT_COMMAND llvm-lit)
  endif ()
  list(APPEND LIT_COMMAND ${LIT_ARGS})
  foreach(param ${ARG_PARAMS})
    list(APPEND LIT_COMMAND --param ${param})
  endforeach()
  if (ARG_UNPARSED_ARGUMENTS)
    add_custom_target(${target}
      COMMAND ${LIT_COMMAND} ${ARG_UNPARSED_ARGUMENTS}
      COMMENT "${comment}"
      ${cmake_3_2_USES_TERMINAL}
      )
  else()
    add_custom_target(${target}
      COMMAND ${CMAKE_COMMAND} -E echo "${target} does nothing, no tools built.")
    message(STATUS "${target} does nothing.")
  endif()
  if (ARG_DEPENDS)
    add_dependencies(${target} ${ARG_DEPENDS})
  endif()

  # Tests should be excluded from "Build Solution".
  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD ON)
endfunction()

# A function to add a set of lit test suites to be driven through 'check-*' targets.
function(add_lit_testsuite target comment)
  cmake_parse_arguments(ARG "" "" "PARAMS;DEPENDS;ARGS" ${ARGN})

  # EXCLUDE_FROM_ALL excludes the test ${target} out of check-all.
  if(NOT EXCLUDE_FROM_ALL)
    # Register the testsuites, params and depends for the global check rule.
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_TESTSUITES ${ARG_UNPARSED_ARGUMENTS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_PARAMS ${ARG_PARAMS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_DEPENDS ${ARG_DEPENDS})
    set_property(GLOBAL APPEND PROPERTY LLVM_LIT_EXTRA_ARGS ${ARG_ARGS})
  endif()

  # Produce a specific suffixed check rule.
  add_lit_target(${target} ${comment}
    ${ARG_UNPARSED_ARGUMENTS}
    PARAMS ${ARG_PARAMS}
    DEPENDS ${ARG_DEPENDS}
    ARGS ${ARG_ARGS}
    )
endfunction()

function(add_lit_testsuites project directory)
  if (NOT CMAKE_CONFIGURATION_TYPES)
    cmake_parse_arguments(ARG "EXCLUDE_FROM_CHECK_ALL" "FOLDER" "PARAMS;DEPENDS;ARGS" ${ARGN})

    if (NOT ARG_FOLDER)
      set(ARG_FOLDER "Test Subdirectories")
    endif()

    # Search recursively for test directories by assuming anything not
    # in a directory called Inputs contains tests.
    file(GLOB_RECURSE to_process LIST_DIRECTORIES true ${directory}/*)
    foreach(lit_suite ${to_process})
      if(NOT IS_DIRECTORY ${lit_suite})
        continue()
      endif()
      string(FIND ${lit_suite} Inputs is_inputs)
      string(FIND ${lit_suite} Output is_output)
      if (NOT (is_inputs EQUAL -1 AND is_output EQUAL -1))
        continue()
      endif()

      # Create a check- target for the directory.
      string(REPLACE ${directory} "" name_slash ${lit_suite})
      if (name_slash)
        string(REPLACE "/" "-" name_slash ${name_slash})
        string(REPLACE "\\" "-" name_dashes ${name_slash})
        string(TOLOWER "${project}${name_dashes}" name_var)
        add_lit_target("check-${name_var}" "Running lit suite ${lit_suite}"
          ${lit_suite}
          ${EXCLUDE_FROM_CHECK_ALL}
          PARAMS ${ARG_PARAMS}
          DEPENDS ${ARG_DEPENDS}
          ARGS ${ARG_ARGS}
        )
        set_target_properties(check-${name_var} PROPERTIES FOLDER ${ARG_FOLDER})
      endif()
    endforeach()
  endif()
endfunction()
