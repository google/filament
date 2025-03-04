# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~
enable_language(CXX) # Tests use C++

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Make sure tests uses the dynamic runtime instead
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

if (IS_DIRECTORY "${GOOGLETEST_INSTALL_DIR}/googletest")
    set(BUILD_GTEST ON)
    set(BUILD_GMOCK OFF)
    set(gtest_force_shared_crt ON)
    set(INSTALL_GTEST OFF)
    add_subdirectory("${GOOGLETEST_INSTALL_DIR}" ${CMAKE_CURRENT_BINARY_DIR}/gtest)
else()
    message(FATAL_ERROR "Could not find googletest directory. See BUILD.md")
endif()

if (WIN32)
    if(NOT IS_DIRECTORY ${DETOURS_INSTALL_DIR})
        message(FATAL_ERROR "Could not find detours! See BUILD.md")
    endif()
    add_library(detours STATIC
        ${DETOURS_INSTALL_DIR}/src/creatwth.cpp
        ${DETOURS_INSTALL_DIR}/src/detours.cpp
        ${DETOURS_INSTALL_DIR}/src/detours.h
        ${DETOURS_INSTALL_DIR}/src/detver.h
        ${DETOURS_INSTALL_DIR}/src/disasm.cpp
        ${DETOURS_INSTALL_DIR}/src/disolarm.cpp
        ${DETOURS_INSTALL_DIR}/src/disolarm64.cpp
        ${DETOURS_INSTALL_DIR}/src/disolia64.cpp
        ${DETOURS_INSTALL_DIR}/src/disolx64.cpp
        ${DETOURS_INSTALL_DIR}/src/disolx86.cpp
        ${DETOURS_INSTALL_DIR}/src/image.cpp
        ${DETOURS_INSTALL_DIR}/src/modules.cpp
    )
    target_include_directories(detours PUBLIC ${DETOURS_INSTALL_DIR}/src)

    target_compile_definitions(detours PUBLIC WIN32_LEAN_AND_MEAN)

    if(MSVC)
        target_compile_definitions(detours PUBLIC  "_CRT_SECURE_NO_WARNINGS=1")
        set_target_properties(detours PROPERTIES COMPILE_FLAGS /EHsc)
    endif()

    # Silence errors found in clang-cl
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_C_SIMULATE_ID}" MATCHES "MSVC")
        target_compile_options(detours PRIVATE -Wno-sizeof-pointer-memaccess -Wno-microsoft-goto -Wno-microsoft-cast)
    endif()
endif()

option(ENABLE_LIVE_VERIFICATION_TESTS "Enable tests which expect to run on live drivers. Meant for manual verification only" OFF)

include(GoogleTest)
add_subdirectory(framework)

add_executable(
    test_regression
        loader_testing_main.cpp
        loader_alloc_callback_tests.cpp
        loader_envvar_tests.cpp
        loader_get_proc_addr_tests.cpp
        loader_debug_ext_tests.cpp
        loader_handle_validation_tests.cpp
        loader_layer_tests.cpp
        loader_regression_tests.cpp
        loader_phys_dev_inst_ext_tests.cpp
        loader_settings_tests.cpp
        loader_version_tests.cpp
        loader_unknown_ext_tests.cpp
        loader_wsi_tests.cpp)
target_link_libraries(test_regression PUBLIC testing_dependencies)
target_compile_definitions(test_regression PUBLIC VK_NO_PROTOTYPES)

add_executable(
    test_fuzzing
        loader_testing_main.cpp
        loader_fuzz_tests.cpp
)
target_link_libraries(test_fuzzing PUBLIC testing_dependencies vulkan)
target_include_directories(test_fuzzing PUBLIC ${CMAKE_SOURCE_DIR}/loader ${CMAKE_SOURCE_DIR}/loader/generated)

# Threading tests live in separate executabe just for threading tests as it'll need support
# in the test harness to enable in CI, as thread sanitizer doesn't work with address sanitizer enabled.
add_executable(
    test_threading
        loader_testing_main.cpp
        loader_threading_tests.cpp)
target_link_libraries(test_threading PUBLIC testing_dependencies)
target_compile_definitions(test_threading PUBLIC VK_NO_PROTOTYPES)

# executables that are meant for testing against real drivers rather than the mocks
if (ENABLE_LIVE_VERIFICATION_TESTS)
    add_subdirectory(live_verification)
endif()

if(WIN32)
    # Copy vulkan-1.dll to the test_fuzzing build directory so that the test_fuzzing exe can find it.
    if(TARGET vulkan)
        add_custom_command(TARGET test_fuzzing POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vulkan> $<TARGET_FILE_DIR:test_fuzzing>)
    endif()
endif()

# https://discourse.cmake.org/t/googletest-crash-when-using-cmake-xcode-arm64/5766
#
# TLDR: On macOS arm64, all binaries have to be signed before running.
# Delay test discovery until test time as a workaround.
if (XCODE)
    set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE "PRE_TEST")
endif()

# must happen after the dll's get copied over
if(NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(test_regression PROPERTIES DISCOVERY_TIMEOUT 100)
    gtest_discover_tests(test_fuzzing PROPERTIES DISCOVERY_TIMEOUT 100)
else()
    gtest_add_tests(TARGET test_regression)
    gtest_add_tests(TARGET test_fuzzing)
endif()

# When APPLE_STATIC_LOADER is ON installation is disabled
if (APPLE_STATIC_LOADER)
    return()
endif()

# Test installation
set(test_install_dir "${CMAKE_CURRENT_BINARY_DIR}/install")
add_test(NAME integration.install
    COMMAND ${CMAKE_COMMAND} --install ${PROJECT_BINARY_DIR} --prefix ${test_install_dir} --config $<CONFIG>
)

# find_package testing currently doesn't work well under cross-compilation scenarios.
if (CMAKE_CROSSCOMPILING OR
    NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    return()
endif()

# Test find_package suppport
add_test(NAME integration.find_package
    COMMAND ${CMAKE_CTEST_COMMAND}
        --build-and-test ${CMAKE_CURRENT_LIST_DIR}/integration
                        ${CMAKE_CURRENT_BINARY_DIR}/find_package
        --build-generator ${CMAKE_GENERATOR}
        --build-options -DCMAKE_PREFIX_PATH=${test_install_dir}
)

# Installing comes before testing
set_tests_properties(integration.find_package PROPERTIES DEPENDS integration.install)

if (CODE_COVERAGE)
    target_code_coverage(test_regression AUTO ALL )
    target_code_coverage(test_fuzzing AUTO ALL )
endif()
