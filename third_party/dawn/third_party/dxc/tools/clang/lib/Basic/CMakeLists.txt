set(LLVM_LINK_COMPONENTS
  Core
#  MC # HLSL Change
  DxcSupport # HLSL Change
  Support
  )

# Figure out if we can track VC revisions.
function(find_first_existing_file out_var)
  foreach(file ${ARGN})
    if(EXISTS "${file}")
      set(${out_var} "${file}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

macro(find_first_existing_vc_file out_var path)
  find_first_existing_file(${out_var}
    "${path}/.git/logs/HEAD" # Git
    "${path}/.svn/wc.db"     # SVN 1.7
    "${path}/.svn/entries"   # SVN 1.6
    )
endmacro()

find_first_existing_vc_file(llvm_vc "${LLVM_MAIN_SRC_DIR}")
find_first_existing_vc_file(clang_vc "${CLANG_SOURCE_DIR}")

# The VC revision include that we want to generate.
set(version_inc "${CMAKE_CURRENT_BINARY_DIR}/SVNVersion.inc")

set(get_svn_script "${LLVM_MAIN_SRC_DIR}/cmake/modules/GetSVN.cmake")

# HLSL Change Starts
if (HLSL_ENABLE_FIXED_VER)
  # Actual version string is no longer specified here, but instead derived from
  # the defines in the generated or copied dxcversion.inc
  add_definitions(-DHLSL_FIXED_VER)
endif (HLSL_ENABLE_FIXED_VER)
# HLSL Change Ends

if(DEFINED llvm_vc AND DEFINED clang_vc)
  # Create custom target to generate the VC revision include.
  add_custom_command(OUTPUT "${version_inc}"
    DEPENDS "${llvm_vc}" "${clang_vc}" "${get_svn_script}"
    COMMAND
    ${CMAKE_COMMAND} "-DFIRST_SOURCE_DIR=${LLVM_MAIN_SRC_DIR}"
                     "-DFIRST_NAME=LLVM"
                     "-DSECOND_SOURCE_DIR=${CLANG_SOURCE_DIR}"
                     "-DSECOND_NAME=SVN"
                     "-DHEADER_FILE=${version_inc}"
                     -P "${get_svn_script}")

  # Mark the generated header as being generated.
  set_source_files_properties("${version_inc}"
    PROPERTIES GENERATED TRUE
               HEADER_FILE_ONLY TRUE)

  # Tell Version.cpp that it needs to build with -DHAVE_SVN_VERSION_INC.
  set_source_files_properties(Version.cpp
    PROPERTIES COMPILE_DEFINITIONS "HAVE_SVN_VERSION_INC")
else()
  # Not producing a VC revision include.
  set(version_inc)
endif()

add_clang_library(clangBasic
  Attributes.cpp
  Builtins.cpp
  CharInfo.cpp
  Diagnostic.cpp
  DiagnosticIDs.cpp
  DiagnosticOptions.cpp
  FileManager.cpp
  FileSystemStatCache.cpp
  IdentifierTable.cpp
  LangOptions.cpp
  Module.cpp
  ObjCRuntime.cpp
  OpenMPKinds.cpp
  OperatorPrecedence.cpp
  SanitizerBlacklist.cpp
  Sanitizers.cpp
  SourceLocation.cpp
  SourceManager.cpp
  TargetInfo.cpp
  Targets.cpp
  TokenKinds.cpp
  Version.cpp
  VersionTuple.cpp
  VirtualFileSystem.cpp
  Warnings.cpp
  ${version_inc}

  DEPENDS
  hlsl_dxcversion_autogen # HLSL Change
  )

# HLSL Change Starts
if ( HLSL_SUPPORT_QUERY_GIT_COMMIT_INFO )
  set(GIT_COMMIT_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/GitCommitInfo.inc)
  set(GET_GIT_COMMIT_SCRIPT ${PROJECT_SOURCE_DIR}/utils/GetCommitInfo.py)
  add_custom_command(
    OUTPUT  ${GIT_COMMIT_INFO_FILE}
    COMMAND ${Python3_EXECUTABLE} ${GET_GIT_COMMIT_SCRIPT}
            ${PROJECT_SOURCE_DIR} ${GIT_COMMIT_INFO_FILE}
    DEPENDS ${GET_GIT_COMMIT_SCRIPT} GIT_COMMIT_INFO_ALWAYS_REBUILD
    COMMENT "Collect Git commit info for versioning"
  )
  add_custom_target(
    GIT_COMMIT_INFO_ALWAYS_REBUILD
    ${CMAKE_COMMAND} -E touch ${GET_GIT_COMMIT_SCRIPT}
    COMMENT "Touch GetCommitInfo.py to trigger rebuild"
  )
  set_property(TARGET GIT_COMMIT_INFO_ALWAYS_REBUILD
    PROPERTY FOLDER "Utils")
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp
    PROPERTIES OBJECT_DEPENDS "${GIT_COMMIT_INFO_FILE}")
endif()

target_include_directories(clangBasic PUBLIC ${HLSL_VERSION_LOCATION}) # HLSL Change
# HLSL Change Ends
