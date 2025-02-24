# ~~~
# Copyright (c) 2014-2018 Valve Corporation
# Copyright (c) 2014-2018 LunarG, Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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

if (ANDROID OR MINGW)
    return()
endif()

# These variables enable downstream users to customize the CMake targets
# based on the target API variant (e.g. Vulkan SC)
set(MOCK_ICD_NAME VkICD_mock_icd)
set(GENERATED generated)

option(BUILD_MOCK_ANDROID_SUPPORT "Build with Android Platform headers" OFF)

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DVK_USE_PLATFORM_WIN32_KHX -DWIN32_LEAN_AND_MEAN)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
elseif(BUILD_MOCK_ANDROID_SUPPORT)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR -DVK_USE_PLATFORM_XCB_KHX)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_XLIB_KHX -DVK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR -DVK_USE_PLATFORM_WAYLAND_KHX)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

add_library(VkICD_mock_icd MODULE)
target_sources(VkICD_mock_icd PRIVATE mock_icd.cpp)
target_link_libraries(VkICD_mock_icd PRIVATE Vulkan::Headers)

target_include_directories(VkICD_mock_icd PRIVATE
    ${GENERATED}
    .
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
    target_compile_options(VkICD_mock_icd PRIVATE
        -Wpointer-arith
        -Wno-unused-function
        -Wno-sign-compare
    )
endif()

if(MSVC)
    target_compile_options(VkICD_mock_icd PRIVATE /bigobj)
    target_compile_definitions(VkICD_mock_icd PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_options(VkICD_mock_icd PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/${MOCK_ICD_NAME}.def)
else()
    if(APPLE)
        set_target_properties(VkICD_mock_icd PROPERTIES SUFFIX ".dylib")
    endif()
    message(DEBUG "Mock ICD Functions are exported via EXPORT")
endif()

set_target_properties(VkICD_mock_icd PROPERTIES OUTPUT_NAME ${MOCK_ICD_NAME})

if (DEFINED GIT_BRANCH_NAME AND DEFINED GIT_TAG_INFO)
    target_compile_definitions(VkICD_mock_icd PRIVATE GIT_BRANCH_NAME="${GIT_BRANCH_NAME}" GIT_TAG_INFO="${GIT_TAG_INFO}")
endif()

# There are 2 primary deliverables for the mock driver.
# - The actual library (lib)VkICD_mock_icd.(dll|so|dylib)
# - The respective json file, VkICD_mock_icd.json
# This code generates the appropriate json for both local testing and the installation.
# NOTE: For WIN32 the JSON and dll MUST be placed in the same location, due to Win32 using a relative path for installation.
set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${MOCK_ICD_NAME}.json.in")
set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/mock_icd.json")
set(OUTPUT_FILE_FINAL_NAME "${MOCK_ICD_NAME}.json")
set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
if (WIN32)
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32 expect the dll in the `bin` dir, this matches our WIN32 SDK process
endif()

if (WIN32)
    set(JSON_LIBRARY_PATH ".\\\\${MOCK_ICD_NAME}.dll")
elseif(APPLE)
    set(JSON_LIBRARY_PATH "./lib${MOCK_ICD_NAME}.dylib")
else()
    set(JSON_LIBRARY_PATH "./lib${MOCK_ICD_NAME}.so")
endif()

configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

# To support both multi/single configuration generators just copy the json to the correct directory
add_custom_command(TARGET VkICD_mock_icd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:VkICD_mock_icd>/${OUTPUT_FILE_FINAL_NAME}
)

# Installing the Mock ICD to system directories is probably not desired since this ICD is not a very complete implementation.
# Require the user to ask that it be installed if they really want it.
option(INSTALL_ICD "Install Mock icd")
if (INSTALL_ICD)
    message(NOTICE "INSTALL_ICD enabled!")
else()
    return()
endif()

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system directories
# This json isn't used for regular local development, it's used for installation
if (UNIX)
    set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_mock_icd.json")

    if(APPLE)
        set(JSON_LIBRARY_PATH "lib${MOCK_ICD_NAME}.dylib")
    else()
        set(JSON_LIBRARY_PATH "lib${MOCK_ICD_NAME}.so")
    endif()

    configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)

    install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/vulkan/icd.d RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()

if (WIN32)
    install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()

install(TARGETS VkICD_mock_icd DESTINATION ${LAYER_INSTALL_DIR})
