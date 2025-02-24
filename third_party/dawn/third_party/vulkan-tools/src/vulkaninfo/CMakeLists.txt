# ~~~
# Copyright (c) 2018-2023 Valve Corporation
# Copyright (c) 2018-2023 LunarG, Inc.
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

# These variables enable downstream users to customize the CMake targets
# based on the target API variant (e.g. Vulkan SC)
set(VULKANINFO_NAME vulkaninfo)
set(GENERATED generated)

add_executable(vulkaninfo)
set_target_properties(vulkaninfo PROPERTIES OUTPUT_NAME ${VULKANINFO_NAME})

target_sources(vulkaninfo PRIVATE vulkaninfo.cpp)

# Setup the vulkaninfo.rc file to contain the correct info
# Optionally uses the VULKANINFO_BUILD_DLL_VERSIONINFO build option to allow setting the exact build version
# When VULKANINFO_BUILD_DLL_VERSIONINFO is not provided, "Dev Build" is added to the version strings
if(WIN32)
    set(VULKANINFO_BUILD_DLL_VERSIONINFO "default" CACHE STRING "Set the version to be used in the vulkaninfo.rc file")

    string(TIMESTAMP CURRENT_YEAR "%Y")
    set(VULKANINFO_CUR_COPYRIGHT_STR "${CURRENT_YEAR}")
    if ("$CACHE{VULKANINFO_BUILD_DLL_VERSIONINFO}" STREQUAL "default")
        message(DEBUG "Setting RC version based on VulkanHeaders Version")
        set(VULKANINFO_RC_VERSION "${VulkanHeaders_VERSION}")
        set(VULKANINFO_VER_FILE_VERSION_STR "\"${VULKANINFO_RC_VERSION}.Dev Build\"")
        set(VULKANINFO_VER_FILE_DESCRIPTION_STR "\"Vulkaninfo - Dev Build\"")
    else()
        message(DEBUG "Setting RC version based on VULKANINFO_BUILD_DLL_VERSIONINFO")
        set(VULKANINFO_RC_VERSION "$CACHE{VULKANINFO_BUILD_DLL_VERSIONINFO}")
        set(VULKANINFO_VER_FILE_VERSION_STR "\"${VULKANINFO_RC_VERSION}\"")
        set(VULKANINFO_VER_FILE_DESCRIPTION_STR "\"vulkaninfo\"")
    endif()

    # RC file wants the value of FILEVERSION to separated by commas
    string(REPLACE "." ", " VULKANINFO_VER_FILE_VERSION "${VULKANINFO_RC_VERSION}")

    # Configure the file to include the versioning info
    configure_file(vulkaninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/vulkaninfo.rc)

    target_sources(vulkaninfo PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/vulkaninfo.rc)
endif()

target_include_directories(vulkaninfo PRIVATE
    ${GENERATED}
    .
)

target_compile_definitions(vulkaninfo PRIVATE VK_ENABLE_BETA_EXTENSIONS VK_NO_PROTOTYPES)

if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_DIRECTFB_SUPPORT "Build DirectFB WSI support" OFF)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_XCB_KHR)
        target_link_libraries(vulkaninfo PRIVATE PkgConfig::XCB)
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_XLIB_KHR)
        target_link_libraries(vulkaninfo PRIVATE PkgConfig::X11)
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
        target_link_libraries(vulkaninfo PRIVATE PkgConfig::WAYLAND_CLIENT)
    endif()

    if(BUILD_WSI_DIRECTFB_SUPPORT)
        pkg_check_modules(DirectFB REQUIRED QUIET IMPORTED_TARGET directfb)
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_DIRECTFB_EXT)
        target_link_libraries(vulkaninfo PRIVATE PkgConfig::DirectFB)
    endif()
endif()

if(APPLE)
    # We do this so vulkaninfo is linked to an individual library and NOT a framework.
    target_link_libraries(vulkaninfo PRIVATE "-framework AppKit -framework QuartzCore")
    target_include_directories(vulkaninfo PRIVATE macOS/vulkaninfo)

    target_sources(vulkaninfo PRIVATE
        macOS/vulkaninfo/metal_view.mm
        macOS/vulkaninfo/metal_view.h
    )
endif()

target_link_libraries(vulkaninfo PRIVATE
    Vulkan::Headers
    ${CMAKE_DL_LIBS}
    volk::volk_headers
)

if(WIN32)
    target_compile_definitions(vulkaninfo PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
elseif(APPLE)
    target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_METAL_EXT)
    if (IOS)
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_IOS_MVK)
    else()
        target_compile_definitions(vulkaninfo PRIVATE VK_USE_PLATFORM_MACOS_MVK)
    endif()
endif()

install(TARGETS vulkaninfo)
