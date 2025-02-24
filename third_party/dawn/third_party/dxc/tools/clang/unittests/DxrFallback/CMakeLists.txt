find_package(D3D12 REQUIRED)

set(LLVM_LINK_COMPONENTS
  analysis
  asmparser
  bitreader
  bitwriter
  core
  dxcsupport
  dxrfallback
  dxil
  hlsl
  instcombine
  ipa
  ipo
  irreader
  linker
#  lto
  mssupport
  option
  profiledata
  scalaropts
  support
  target
  transformutils
  vectorize
  )

set(TEST_FILES 
  testFiles/testShader1.hlsl
  testFiles/testShader2.hlsl
  testFiles/testShader3.hlsl
  testFiles/testShader4.hlsl
  testFiles/testTraversal.h
  testFiles/testTraversal.hlsl
  testFiles/testTraversal2.hlsl
  testFiles/testLib.h
  testFiles/testLib.hlsl
  testFiles/HLSLRayTracingInternalPrototypes.h
  )

# put test files in a folder in Visual Studio
source_group( "Test Files" FILES ${TEST_FILES} )

# don't compile test files
set_source_files_properties( ${TEST_FILES} PROPERTIES HEADER_FILE_ONLY ON)

# create a file to include to provide a default path for test files
set(DEFAULT_TEST_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/testFiles/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/defaultTestFilePath.h.in ${CMAKE_CURRENT_BINARY_DIR}/defaultTestFilePath.h)

include_directories(
    ${D3D12_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_clang_executable(test_DxrFallback
  test_DxrFallback.cpp

  DXSampleHelper.h
  ShaderTester.h
  ShaderTesterImpl.cpp
  ShaderTesterImpl.h
  
  ${TEST_FILES}
  )

target_link_libraries(test_DxrFallback
  dxcompiler
  d3d12
  dxgi
  )

add_dependencies(test_DxrFallback dxcompiler)

install(TARGETS test_DxrFallback
  RUNTIME DESTINATION bin)


