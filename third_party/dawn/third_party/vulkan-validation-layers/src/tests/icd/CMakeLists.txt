# ~~~
# Copyright (c) 2024 LunarG, Inc.
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
elseif (APPLE)
    return()
endif()


# These variables enable downstream users to customize the CMake targets
# based on the target API variant (e.g. Vulkan SC)
set(ICD_NAME VVL_Test_ICD)

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DVK_USE_PLATFORM_WIN32_KHX -DWIN32_LEAN_AND_MEAN)
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

add_library(VVL_Test_ICD MODULE)

target_sources(VVL_Test_ICD PRIVATE
    test_icd.h
    test_icd.cpp)

target_link_libraries(VVL_Test_ICD PRIVATE
    Vulkan::Headers
    VkLayer_utils
)

target_include_directories(VVL_Test_ICD PRIVATE
    ${CMAKE_SOURCE_DIR}/layers/${API_TYPE}/generated
    .
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
    target_compile_options(VVL_Test_ICD PRIVATE
        -Wpointer-arith
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

if(MSVC)
    target_compile_options(VVL_Test_ICD PRIVATE /bigobj)
    target_compile_options(VVL_Test_ICD PRIVATE /wd4100)
    target_compile_definitions(VVL_Test_ICD PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_options(VVL_Test_ICD PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/${ICD_NAME}.def)
else()
    message(DEBUG "Test ICD Functions are exported via EXPORT")
endif()

set_target_properties(VVL_Test_ICD PROPERTIES OUTPUT_NAME ${ICD_NAME})

if (DEFINED GIT_BRANCH_NAME AND DEFINED GIT_TAG_INFO)
    target_compile_definitions(VVL_Test_ICD PRIVATE GIT_BRANCH_NAME="${GIT_BRANCH_NAME}" GIT_TAG_INFO="${GIT_TAG_INFO}")
endif()

# There are 2 primary deliverables for the Test driver.
# - The actual library (lib)VVL_Test_ICD.(dll|so|dylib)
# - The respective json file, VVL_Test_ICD.json
# This code generates the appropriate json for both local testing and the installation.
# NOTE: For WIN32 the JSON and dll MUST be placed in the same location, due to Win32 using a relative path for installation.
set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${ICD_NAME}.json.in")
set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/test_icd.json")
set(OUTPUT_FILE_FINAL_NAME "${ICD_NAME}.json")
set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
if (WIN32)
    set(LAYER_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}) # WIN32 expect the dll in the `bin` dir, this matches our WIN32 SDK process
endif()

if (WIN32)
    set(JSON_LIBRARY_PATH ".\\\\${ICD_NAME}.dll")
else()
    set(JSON_LIBRARY_PATH "./lib${ICD_NAME}.so")
endif()

configure_file(${INPUT_FILE} ${INTERMEDIATE_FILE} @ONLY)

# To support both multi/single configuration generators just copy the json to the correct directory
add_custom_command(TARGET VVL_Test_ICD POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} $<TARGET_FILE_DIR:VVL_Test_ICD>/${OUTPUT_FILE_FINAL_NAME}
)

# Installing the Test ICD to system directories is probably not desired since this ICD is not a very complete implementation.
# Require the user to ask that it be installed if they really want it.
option(INSTALL_VVL_TEST_ICD "Install Test icd")
if (INSTALL_VVL_TEST_ICD)
    message(NOTICE "INSTALL_ICD enabled!")
else()
    return()
endif()

# For UNIX-based systems, `library_path` should not contain a relative path (indicated by "./") before installing to system directories
# This json isn't used for regular local development, it's used for installation
if (UNIX)
    set(UNIX_INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/json/unix_install_test_icd.json")
    set(JSON_LIBRARY_PATH "lib${ICD_NAME}.so")

    configure_file(${INPUT_FILE} ${UNIX_INTERMEDIATE_FILE} @ONLY)
    install(FILES ${UNIX_INTERMEDIATE_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${API_TYPE}/icd.d RENAME ${OUTPUT_FILE_FINAL_NAME})
elseif (WIN32)
    install(FILES ${INTERMEDIATE_FILE} DESTINATION ${LAYER_INSTALL_DIR} RENAME ${OUTPUT_FILE_FINAL_NAME})
endif()

install(TARGETS VVL_Test_ICD DESTINATION ${LAYER_INSTALL_DIR})
