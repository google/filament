# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License.
# See LICENSE.TXT for details.

add_hlsl_hctgen(DxcDisassembler OUTPUT DxcDisassembler.inc BUILD_DIR)

set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  analysis
  asmparser
#  asmprinter # no support for LLVM codegen
  bitreader
  bitwriter
#  codegen # no support for LLVM codegen
  core
#  debuginfodwarf # no support for DWARF files (IR debug info is OK)
#  debuginfopdb # no support for PDB files
  dxcsupport
  dxil
  dxilcontainer
  dxilpixpasses # for DxcOptimizerPass
  dxilrootsignature
  dxcbindingtable
  hlsl
  instcombine
  ipa
  ipo
  irreader
#  libdriver
#  lineeditor
  linker
#  lto
#  mirparser # no support for LLVM codegen
  mssupport
#  object # no support for object files (coff, elf)
  option
#  passes
  profiledata
  scalaropts
#  selectiondag # no support for LLVM codegen
  support
  target
  transformutils
  vectorize
  dxilcompression
  passprinters
  )

if (WIN32)
set(SOURCES
  dxcapi.cpp
  dxcassembler.cpp
  dxclibrary.cpp
  dxcompilerobj.cpp
  dxcvalidator.cpp
  DXCompiler.cpp
  DXCompiler.rc
  DXCompiler.def
  dxcfilesystem.cpp
  dxcutil.cpp
  dxcdisassembler.cpp
  dxcpdbutils.cpp
  dxclinker.cpp
  dxcshadersourceinfo.cpp
)
else ()
set(SOURCES
  dxcapi.cpp
  dxcassembler.cpp
  dxclibrary.cpp
  dxcompilerobj.cpp
  DXCompiler.cpp
  dxcfilesystem.cpp
  dxcutil.cpp
  dxcdisassembler.cpp
  dxcpdbutils.cpp
  dxcvalidator.cpp
  dxclinker.cpp
  dxcshadersourceinfo.cpp
)
set (HLSL_IGNORE_SOURCES
  dxcdia.cpp
)
endif(WIN32)

set(LIBRARIES
  clangIndex
#  clangARCMigrate
  clangRewrite
  clangCodeGen
  clangRewriteFrontend
  clangFrontend
  clangDriver
#  clangSerialization
  clangSema
  clangEdit
  clangAST
  clangCodeGen
  clangLex
  clangTooling
  clangBasic
  libclang
  dxcvalidator
  )

if(WIN32)
  set(LIBRARIES
    ${LIBRARIES} 
    LLVMDxilDia)
endif(WIN32)

set(GENERATED_HEADERS
  ClangAttrClasses
  ClangAttrList
  ClangAttrParsedAttrList
  ClangCommentNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangDeclNodes
  ClangStmtNodes
  )

if (MSVC)
  find_package(DiaSDK REQUIRED) # Used for constants and declarations.
endif (MSVC)

add_clang_library(dxcompiler SHARED ${SOURCES})
add_dependencies(dxcompiler TablegenHLSLOptions) 
if (MSVC)
  # No DxcEtw on non-Windows platforms.
  add_dependencies(dxcompiler DxcEtw)
endif()
target_link_libraries(dxcompiler PRIVATE ${LIBRARIES})
if (ENABLE_SPIRV_CODEGEN)
  target_link_libraries(dxcompiler PRIVATE clangSPIRV)
endif (ENABLE_SPIRV_CODEGEN)
if (ENABLE_METAL_CODEGEN)
  target_link_libraries(dxcompiler PRIVATE ${METAL_IRCONVERTER_LIB})
  target_include_directories(dxcompiler PRIVATE ${METAL_IRCONVERTER_INCLUDE_DIR})

  get_filename_component(METAL_IRCONVERTER_LIB_DIR ${METAL_IRCONVERTER_LIB} DIRECTORY CACHE)
  set_property(TARGET dxcompiler APPEND_STRING
               PROPERTY LINK_FLAGS " -Wl,-rpath,${METAL_IRCONVERTER_LIB_DIR}")
endif (ENABLE_METAL_CODEGEN)
include_directories(AFTER ${LLVM_INCLUDE_DIR}/dxc/Tracing ${DIASDK_INCLUDE_DIRS} ${HLSL_VERSION_LOCATION})
include_directories(${LLVM_SOURCE_DIR}/tools/clang/tools/dxcvalidator)

set_target_properties(dxcompiler
  PROPERTIES
  OUTPUT_NAME "dxcompiler"
  VERSION ${LIBCLANG_LIBRARY_VERSION}
  DEFINE_SYMBOL _CINDEX_LIB_)

if (WIN32)
  set(install_dest RUNTIME DESTINATION bin)
else()
  set(install_dest LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX})
endif()

install(TARGETS dxcompiler
  ${install_dest}
  COMPONENT dxcompiler)

add_custom_target(install-dxcompiler
  DEPENDS dxcompiler
  COMMAND "${CMAKE_COMMAND}"
          -DCMAKE_INSTALL_COMPONENT=dxcompiler
          -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
