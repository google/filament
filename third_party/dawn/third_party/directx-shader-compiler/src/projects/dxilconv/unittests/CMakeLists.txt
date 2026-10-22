# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.

find_package(TAEF REQUIRED)
#find_package(DiaSDK REQUIRED) # Used for constants and declarations.

add_dxilconv_project_test_library(dxilconv-tests SHARED
  DxilConvTests.cpp
)

target_link_libraries(dxilconv-tests PRIVATE
  HLSLTestLib
  LLVMDxilContainer # for RDAT used by HLSLTestLib
  LLVMDxcSupport
  LLVMMSSupport
  LLVMSupport
  LLVMOption
  ${TAEF_LIBRARIES}
  shlwapi
)

target_include_directories(dxilconv-tests PRIVATE
  ${TAEF_INCLUDE_DIRS}
)

# dxilconv-tests calls out to several external tools. Those need to be listed
# here to ensure they build with dxilconv for our test target depencencies to be
# correct.
add_dependencies(dxilconv-tests dxilconv HLSLTestLib dxbc2dxil dxa opt)

install(TARGETS dxilconv-tests
  RUNTIME DESTINATION bin)

# Add a .user file with settings for te.exe.
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" DOS_STYLE_SOURCE_DIR)
file(TO_NATIVE_PATH "${TAEF_BIN_DIR}" DOS_TAEF_BIN_DIR)
configure_file(dxilconv-tests.vcxproj.user.in dxilconv-tests.vcxproj.user)
