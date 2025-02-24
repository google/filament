# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_STANDARD 11)

project(cppdap C CXX)

###########################################################
# Options
###########################################################
function (option_if_not_defined name description default)
    if(NOT DEFINED ${name})
        option(${name} ${description} ${default})
    endif()
endfunction()

option_if_not_defined(CPPDAP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option_if_not_defined(CPPDAP_BUILD_EXAMPLES "Build example applications" OFF)
option_if_not_defined(CPPDAP_BUILD_TESTS "Build tests" OFF)
option_if_not_defined(CPPDAP_BUILD_FUZZER "Build fuzzer" OFF)
option_if_not_defined(CPPDAP_ASAN "Build dap with address sanitizer" OFF)
option_if_not_defined(CPPDAP_MSAN "Build dap with memory sanitizer" OFF)
option_if_not_defined(CPPDAP_TSAN "Build dap with thread sanitizer" OFF)
option_if_not_defined(CPPDAP_INSTALL_VSCODE_EXAMPLES "Build and install dap examples into vscode extensions directory" OFF)
option_if_not_defined(CPPDAP_INSTALL "Create dap install target" OFF)

###########################################################
# Directories
###########################################################
function (set_if_not_defined name value)
    if(NOT DEFINED ${name})
        set(${name} ${value} PARENT_SCOPE)
    endif()
endfunction()

set(CPPDAP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CPPDAP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_if_not_defined(CPPDAP_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set_if_not_defined(CPPDAP_JSON_DIR        ${CPPDAP_THIRD_PARTY_DIR}/json)
set_if_not_defined(CPPDAP_GOOGLETEST_DIR  ${CPPDAP_THIRD_PARTY_DIR}/googletest)

###########################################################
# Submodules
###########################################################
if(CPPDAP_BUILD_TESTS)
    if(NOT EXISTS ${CPPDAP_GOOGLETEST_DIR}/.git)
        message(WARNING "third_party/googletest submodule missing.")
        message(WARNING "Run: `git submodule update --init` to build tests.")
        set(CPPDAP_BUILD_TESTS OFF)
    endif()
endif(CPPDAP_BUILD_TESTS)

###########################################################
# File lists
###########################################################
set(CPPDAP_LIST
    ${CPPDAP_SRC_DIR}/content_stream.cpp
    ${CPPDAP_SRC_DIR}/io.cpp
    ${CPPDAP_SRC_DIR}/json_serializer.cpp
    ${CPPDAP_SRC_DIR}/network.cpp
    ${CPPDAP_SRC_DIR}/protocol_events.cpp
    ${CPPDAP_SRC_DIR}/protocol_requests.cpp
    ${CPPDAP_SRC_DIR}/protocol_response.cpp
    ${CPPDAP_SRC_DIR}/protocol_types.cpp
    ${CPPDAP_SRC_DIR}/session.cpp
    ${CPPDAP_SRC_DIR}/socket.cpp
    ${CPPDAP_SRC_DIR}/typeinfo.cpp
    ${CPPDAP_SRC_DIR}/typeof.cpp
)

###########################################################
# OS libraries
###########################################################
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPPDAP_OS_LIBS WS2_32)
    set(CPPDAP_OS_EXE_EXT ".exe")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CPPDAP_OS_LIBS pthread)
    set(CPPDAP_OS_EXE_EXT "")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPPDAP_OS_LIBS)
    set(CPPDAP_OS_EXE_EXT "")
endif()

###########################################################
# Functions
###########################################################
function(cppdap_set_target_options target)
    # Enable all warnings
    if(MSVC)
        target_compile_options(${target} PRIVATE "-W4")
    else()
        target_compile_options(${target} PRIVATE "-Wall")
    endif()

    # Disable specific, pedantic warnings
    if(MSVC)
        target_compile_options(${target} PRIVATE
            "-D_CRT_SECURE_NO_WARNINGS"

            # Warnings from nlohmann/json headers.
            "/wd4267" # 'argument': conversion from 'size_t' to 'int', possible loss of data
        )
    endif()

    # Treat all warnings as errors
    if(CPPDAP_WARNINGS_AS_ERRORS)
        if(MSVC)
            target_compile_options(${target} PRIVATE "/WX")
        else()
            target_compile_options(${target} PRIVATE "-Werror")
        endif()
    endif(CPPDAP_WARNINGS_AS_ERRORS)

    if(CPPDAP_ASAN)
        target_compile_options(${target} PUBLIC "-fsanitize=address")
        target_link_libraries(${target} "-fsanitize=address")
    elseif(CPPDAP_MSAN)
        target_compile_options(${target} PUBLIC "-fsanitize=memory")
        target_link_libraries(${target} "-fsanitize=memory")
    elseif(CPPDAP_TSAN)
        target_compile_options(${target} PUBLIC "-fsanitize=thread")
        target_link_libraries(${target} "-fsanitize=thread")
    endif()

    # Error on undefined symbols
    # if(NOT MSVC)
    #     target_compile_options(${target} PRIVATE "-Wl,--no-undefined")
    # endif()

    target_include_directories(${target} PUBLIC ${CPPDAP_INCLUDE_DIR})
endfunction(cppdap_set_target_options)

###########################################################
# Targets
###########################################################

# dap
add_library(cppdap STATIC ${CPPDAP_LIST})
set_target_properties(cppdap PROPERTIES
    POSITION_INDEPENDENT_CODE 1
)

target_include_directories(cppdap PRIVATE "${CPPDAP_JSON_DIR}/include/")

cppdap_set_target_options(cppdap)

target_link_libraries(cppdap "${CPPDAP_OS_LIBS}")

# install
if(CPPDAP_INSTALL)
    include(GNUInstallDirs)

    install(DIRECTORY ${CPPDAP_INCLUDE_DIR}/cppdap
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        USE_SOURCE_PERMISSIONS
    )

    install(TARGETS cppdap
        EXPORT cppdap-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT cppdap-targets
        FILE cppdap-config.cmake
        NAMESPACE cppdap::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppdap
    )
endif(CPPDAP_INSTALL)

# tests
if(CPPDAP_BUILD_TESTS)
    set(DAP_TEST_LIST
        ${CPPDAP_SRC_DIR}/any_test.cpp
        ${CPPDAP_SRC_DIR}/chan_test.cpp
        ${CPPDAP_SRC_DIR}/content_stream_test.cpp
        ${CPPDAP_SRC_DIR}/dap_test.cpp
        ${CPPDAP_SRC_DIR}/json_serializer_test.cpp
        ${CPPDAP_SRC_DIR}/network_test.cpp
        ${CPPDAP_SRC_DIR}/optional_test.cpp
        ${CPPDAP_SRC_DIR}/rwmutex_test.cpp
        ${CPPDAP_SRC_DIR}/session_test.cpp
        ${CPPDAP_SRC_DIR}/socket_test.cpp
        ${CPPDAP_SRC_DIR}/typeinfo_test.cpp
        ${CPPDAP_SRC_DIR}/variant_test.cpp
        ${CPPDAP_GOOGLETEST_DIR}/googletest/src/gtest-all.cc
    )

    set(DAP_TEST_INCLUDE_DIR
        ${CPPDAP_GOOGLETEST_DIR}/googlemock/include/
        ${CPPDAP_GOOGLETEST_DIR}/googletest/
        ${CPPDAP_GOOGLETEST_DIR}/googletest/include/
        ${CPPDAP_JSON_DIR}/include/
    )

    add_executable(cppdap-unittests ${DAP_TEST_LIST})

    set_target_properties(cppdap-unittests PROPERTIES
        INCLUDE_DIRECTORIES "${DAP_TEST_INCLUDE_DIR}"
        FOLDER "Tests"
    )

    if(MSVC)
        # googletest emits warning C4244: 'initializing': conversion from 'double' to 'testing::internal::BiggestInt', possible loss of data
        target_compile_options(cppdap-unittests PRIVATE "/wd4244")
    endif()

    cppdap_set_target_options(cppdap-unittests)

    target_link_libraries(cppdap-unittests cppdap "${CPPDAP_OS_LIBS}")
endif(CPPDAP_BUILD_TESTS)

# fuzzer
if(CPPDAP_BUILD_FUZZER)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "CPPDAP_BUILD_FUZZER can currently only be used with the clang toolchain")
    endif()
    set(DAP_FUZZER_LIST
        ${CPPDAP_LIST}
        ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz.cpp
    )
    add_executable(cppdap-fuzzer ${DAP_FUZZER_LIST})
    if(CPPDAP_ASAN)
        target_compile_options(cppdap-fuzzer PUBLIC "-fsanitize=fuzzer,address")
        target_link_libraries(cppdap-fuzzer "-fsanitize=fuzzer,address")
    elseif(CPPDAP_MSAN)
        target_compile_options(cppdap-fuzzer PUBLIC "-fsanitize=fuzzer,memory")
        target_link_libraries(cppdap-fuzzer "-fsanitize=fuzzer,memory")
    elseif(CPPDAP_TSAN)
        target_compile_options(cppdap-fuzzer PUBLIC "-fsanitize=fuzzer,thread")
        target_link_libraries(cppdap-fuzzer "-fsanitize=fuzzer,thread")
    else()
        target_compile_options(cppdap-fuzzer PUBLIC "-fsanitize=fuzzer")
        target_link_libraries(cppdap-fuzzer "-fsanitize=fuzzer")
    endif()
    target_include_directories(cppdap-fuzzer PUBLIC
        ${CPPDAP_INCLUDE_DIR}
        ${CPPDAP_SRC_DIR}
        ${CPPDAP_JSON_DIR}/include/
    )
    target_link_libraries(cppdap-fuzzer cppdap "${CPPDAP_OS_LIBS}")
endif(CPPDAP_BUILD_FUZZER)

# examples
if(CPPDAP_BUILD_EXAMPLES)
    function(build_example target)
        add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/examples/${target}.cpp")
        set_target_properties(${target} PROPERTIES
            FOLDER "Examples"
        )
        cppdap_set_target_options(${target})
        target_link_libraries(${target} cppdap "${CPPDAP_OS_LIBS}")

        if(CPPDAP_INSTALL_VSCODE_EXAMPLES)
            if(CMAKE_SYSTEM_NAME MATCHES "Windows")
                set(extroot "$ENV{USERPROFILE}\\.vscode\\extensions")
            else()
                set(extroot "$ENV{HOME}/.vscode/extensions")
            endif()
            if(EXISTS ${extroot})
                set(extdir "${extroot}/google.cppdap-example-${target}-1.0.0")
                configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/vscode/package.json ${extdir}/package.json)
                add_custom_command(TARGET ${target}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${extdir})
            else()
                message(WARNING "Could not install vscode example extension as '${extroot}' does not exist")
            endif()
        endif(CPPDAP_INSTALL_VSCODE_EXAMPLES)
    endfunction(build_example)

    build_example(hello_debugger)

endif(CPPDAP_BUILD_EXAMPLES)
