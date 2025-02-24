include(HCT)

add_hlsl_hctgen(HLSLIntrinsicOp OUTPUT HlslIntrinsicOp.h)

set(HLSL_TEST_DATA_DIR ${LLVM_SOURCE_DIR}/tools/clang/test/HLSL/)
set(EXEC_TEST_DATA_DIR ${LLVM_SOURCE_DIR}/tools/clang/unittests/HLSLExec/)

configure_file(
  ${LLVM_MAIN_INCLUDE_DIR}/dxc/Test/TestConfig.h.in
  ${LLVM_INCLUDE_DIR}/dxc/Test/TestConfig.h
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${LLVM_INCLUDE_DIR}/dxc/config.h
  )

add_subdirectory(DXIL)
add_subdirectory(DxilContainer)
add_subdirectory(Support)
add_subdirectory(Tracing)

set(DXC_PUBLIC_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxcapi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxcerrors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxcisense.h)

if (NOT WIN32)
  set(DXC_PLATFORM_PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/WinAdapter.h)
endif()

# This target just needs to exist for the distribution build, it doesn't need to
# actually build anything since the headers are static.
add_custom_target(dxc-headers)

install(FILES 
    ${DXC_PUBLIC_HEADERS}
    ${DXC_PLATFORM_PUBLIC_HEADERS}
    DESTINATION include/dxc
    COMPONENT dxc-headers
    )

add_custom_target(install-dxc-headers
    DEPENDS dxc-headers
    COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_INSTALL_COMPONENT=dxc-headers
            -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
