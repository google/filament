# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main # Live at head
)
FetchContent_MakeAvailable(googletest)

project(DirectX-Headers-GoogleTest-Suite CXX)
add_executable(Feature-Support-Test feature_support_test.cpp)
target_link_libraries(Feature-Support-Test DirectX-Headers DirectX-Guids d3d12 dxcore gtest_main)
