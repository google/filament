add_custom_target(ClangUnitTests)
set_target_properties(ClangUnitTests PROPERTIES FOLDER "Clang tests")

# HLSL Change - rpath
# Add LLVM_LIBRARY_OUTPUT_INTDIR to install rpath for unit tests. This allows
# unit test binaries on Unix to be run directly even if they dlopen or
# dynamically link against dxcompiler.
set(CMAKE_INSTALL_RPATH "${LLVM_LIBRARY_OUTPUT_INTDIR};${CMAKE_INSTALL_RPATH}")
# HLSL Change End - rpath

# add_clang_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against the clang
# Produces a binary named 'basename(test_dirname)'.
function(add_clang_unittest test_dirname)
  add_unittest(ClangUnitTests ${test_dirname} ${ARGN})
endfunction()

if ( CLANG_INCLUDE_TESTS ) # HLSL Change

add_subdirectory(Basic)
add_subdirectory(Lex)
#add_subdirectory(Driver) # HLSL Disabled :(
if(CLANG_ENABLE_STATIC_ANALYZER)
  add_subdirectory(StaticAnalyzer)
  add_subdirectory(Frontend)
endif()
#add_subdirectory(ASTMatchers) # HLSL Disabled :(
#add_subdirectory(AST) # HLSL Disabled :(
#add_subdirectory(Tooling) # HLSL Disabled :(
#add_subdirectory(Format) # HLSL Disabled :(
add_subdirectory(Rewrite)
#add_subdirectory(Sema) # HLSL Disabled :(
add_subdirectory(CodeGen)
# FIXME: libclang unit tests are disabled on Windows due
# to failures, mostly in libclang.VirtualFileOverlay_*.
if(NOT WIN32)
  #add_subdirectory(libclang) # HLSL Disabled :(
endif()

endif (CLANG_INCLUDE_TESTS) # HLSL Change

# HLSL Change Starts

if (HLSL_INCLUDE_TESTS) 
  add_subdirectory(HLSL)
  add_subdirectory(HLSLTestLib)
  if (WIN32) # These tests require MS specific TAEF and DIA SDK
    add_subdirectory(HLSLErrors)
    add_subdirectory(HLSLExec)
    add_subdirectory(HLSLHost)
    add_subdirectory(dxc_batch)
    add_subdirectory(DxrFallback)
  endif (WIN32)
endif (HLSL_INCLUDE_TESTS)

# HLSL Change Ends


# SPIRV Change Starts

if (SPIRV_BUILD_TESTS)
  add_subdirectory(SPIRV)
endif (SPIRV_BUILD_TESTS)

# SPIRV Change Ends
