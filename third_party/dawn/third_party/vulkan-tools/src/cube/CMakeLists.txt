# ~~~
# Copyright (c) 2018 Valve Corporation
# Copyright (c) 2018 LunarG, Inc.
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

if(APPLE)
    set(MOLTENVK_REPO_ROOT "MOLTENVK-NOTFOUND" CACHE PATH "Absolute path to a MoltenVK repo directory")
    if(NOT MOLTENVK_REPO_ROOT)
        message(FATAL_ERROR "Must define location of MoltenVK repo -- see BUILD.md")
    endif()
    message(STATUS "Using MoltenVK repo location at ${MOLTENVK_REPO_ROOT}")

    # Source for the MoltenVK ICD library and JSON file
    set(MOLTENVK_DIR ${MOLTENVK_REPO_ROOT})

    # MoltenVK JSON File
    execute_process(COMMAND mkdir -p ${PROJECT_BINARY_DIR}/staging-json)
    execute_process(COMMAND sed -e "/\"library_path\":/s$:[[:space:]]*\"[[:space:]]*[\\.\\/]*$: \"..\\/..\\/..\\/Frameworks\\/$"
                            ${MOLTENVK_DIR}/MoltenVK/icd/MoltenVK_icd.json
                    OUTPUT_FILE ${PROJECT_BINARY_DIR}/staging-json/MoltenVK_icd.json)

    # ~~~
    # Modify the ICD JSON file to adjust the library path.
    # The ICD JSON file goes in the Resources/vulkan/icd.d directory, so adjust the
    # library_path to the relative path to the Frameworks directory in the bundle.
    # The regex does: substitute ':<whitespace>"<whitespace><all occurences of . and />' with:
    # ': "../../../Frameworks/'
    # ~~~
    add_custom_target(MoltenVK_icd-staging-json ALL
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/staging-json
        COMMAND sed -e "/\"library_path\":/s$:[[:space:]]*\"[[:space:]]*[\\.\\/]*$: \"..\\/..\\/..\\/Frameworks\\/$"
                ${MOLTENVK_DIR}/MoltenVK/icd/MoltenVK_icd.json > ${PROJECT_BINARY_DIR}/staging-json/MoltenVK_icd.json
        VERBATIM
        DEPENDS "${MOLTENVK_DIR}/MoltenVK/icd/MoltenVK_icd.json"
    )
    set_source_files_properties(${PROJECT_BINARY_DIR}/staging-json/MoltenVK_icd.json PROPERTIES GENERATED TRUE)

    find_library(COCOA NAMES Cocoa)

    # Locate Interface Builder Tool, needed to build things like Storyboards outside of Xcode.
    if(NOT XCODE)
        # Make sure we can find the 'ibtool' program. If we can NOT find it we skip generation of this project.
        find_program(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
        if(${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
            message(SEND_ERROR "ibtool can not be found and is needed to compile the .xib files. "
                            "It should have been installed with the Apple developer tools. "
                            "The default system paths were searched in addition to ${OSX_DEVELOPER_ROOT}/usr/bin.")
        endif()
    endif()
endif()

if(ANDROID OR APPLE)
    set(WSI_DISPLAY_DEFAULT_SETTING "OFF")
else()
    set(WSI_DISPLAY_DEFAULT_SETTING "ON")
endif()

option(BUILD_WSI_DISPLAY_SUPPORT "Build DISPLAY WSI support" ${WSI_DISPLAY_DEFAULT_SETTING})

if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_DIRECTFB_SUPPORT "Build DirectFB WSI support" OFF)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        pkg_get_variable(XCB_INCLUDE_DIRS xcb includedir)
        message(DEBUG "XCB_INCLUDE_DIRS = ${XCB_INCLUDE_DIRS}")
    endif()

    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        pkg_get_variable(XLIB_INCLUDE_DIRS x11 includedir)
        message(DEBUG "XLIB_INCLUDE_DIRS = ${XLIB_INCLUDE_DIRS}")
    endif()

    if(BUILD_WSI_WAYLAND_SUPPORT)
        pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
        pkg_get_variable(WAYLAND_INCLUDE_DIRS wayland-client includedir)

        pkg_get_variable(WAYLAND_SCANNER_EXECUTABLE wayland-scanner wayland_scanner)
        message(DEBUG "WAYLAND_SCANNER_EXECUTABLE = ${WAYLAND_SCANNER_EXECUTABLE}")

        pkg_get_variable(WAYLAND_CLIENT_PATH wayland-client pkgdatadir)
        message(DEBUG "WAYLAND_CLIENT_PATH = ${WAYLAND_CLIENT_PATH}")
        set(WAYLAND_CODE_PROTOCOL ${WAYLAND_CLIENT_PATH}/wayland.xml)

        pkg_get_variable(WAYLAND_PROTOCOLS_PATH wayland-protocols pkgdatadir)
        message(DEBUG "WAYLAND_PROTOCOLS_PATH = ${WAYLAND_PROTOCOLS_PATH}")
        set(XDG_SHELL_PROTOCOL ${WAYLAND_PROTOCOLS_PATH}/stable/xdg-shell/xdg-shell.xml)

        add_custom_command(COMMENT "Generating wayland client protocol dispatch data"
                          OUTPUT wayland-client.c
                          COMMAND ${WAYLAND_SCANNER_EXECUTABLE}
                                  private-code
                                  ${WAYLAND_CODE_PROTOCOL}
                                  ${CMAKE_CURRENT_BINARY_DIR}/wayland-client.c
                          MAIN_DEPENDENCY ${WAYLAND_CODE_PROTOCOL}
                          DEPENDS ${WAYLAND_CODE_PROTOCOL} ${WAYLAND_SCANNER_EXECUTABLE})

        add_custom_command(COMMENT "Generating xdg-shell protocol dispatch data"
                           OUTPUT xdg-shell-code.c
                           COMMAND ${WAYLAND_SCANNER_EXECUTABLE}
                                   private-code
                                   ${XDG_SHELL_PROTOCOL}
                                   ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-code.c
                           MAIN_DEPENDENCY ${XDG_SHELL_PROTOCOL}
                           DEPENDS ${XDG_SHELL_PROTOCOL} ${WAYLAND_SCANNER_EXECUTABLE})
        add_custom_command(COMMENT "Generating xdg-shell protocol header"
                           OUTPUT xdg-shell-client-header.h
                           COMMAND ${WAYLAND_SCANNER_EXECUTABLE}
                                   client-header
                                   ${XDG_SHELL_PROTOCOL}
                                   ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-header.h
                           MAIN_DEPENDENCY ${XDG_SHELL_PROTOCOL}
                           DEPENDS ${XDG_SHELL_PROTOCOL} ${WAYLAND_SCANNER_EXECUTABLE})

        set(XDG_DECORATION_PROTOCOL ${WAYLAND_PROTOCOLS_PATH}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml)
        add_custom_command(COMMENT "Generating xdg-decoration protocol dispatch data"
                           OUTPUT xdg-decoration-code.c
                           COMMAND ${WAYLAND_SCANNER_EXECUTABLE}
                                   private-code
                                   ${XDG_DECORATION_PROTOCOL}
                                   ${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-code.c
                           MAIN_DEPENDENCY ${XDG_DECORATION_PROTOCOL}
                           DEPENDS ${XDG_DECORATION_PROTOCOL} ${WAYLAND_SCANNER_EXECUTABLE})
        add_custom_command(COMMENT "Generating xdg-decoration protocol header"
                           OUTPUT xdg-decoration-client-header.h
                           COMMAND ${WAYLAND_SCANNER_EXECUTABLE}
                                   client-header
                                   ${XDG_DECORATION_PROTOCOL}
                                   ${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-client-header.h
                           MAIN_DEPENDENCY ${XDG_DECORATION_PROTOCOL}
                           DEPENDS ${XDG_DECORATION_PROTOCOL} ${WAYLAND_SCANNER_EXECUTABLE})

        set(WAYLAND_ADDITIONAL_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/wayland-client.c
                           ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-code.c
                           ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-header.h
                           ${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-code.c
                           ${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-client-header.h)
    endif()

    if(BUILD_WSI_DIRECTFB_SUPPORT)
        pkg_check_modules(DirectFB REQUIRED QUIET IMPORTED_TARGET directfb)
    endif()
endif()

if(BUILD_WSI_DISPLAY_SUPPORT)
    list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_DISPLAY_KHR)
endif()

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_METAL_EXT)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    if(BUILD_WSI_XCB_SUPPORT)
        list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_XCB_KHR)
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_XLIB_KHR)
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
    if(BUILD_WSI_DIRECTFB_SUPPORT)
        list(APPEND ENABLED_CUBE_PLATFORMS VK_USE_PLATFORM_DIRECTFB_EXT)
    endif()
    link_libraries(${API_LOWERCASE} m)
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

if(NOT DEFINED ENABLED_CUBE_PLATFORMS)
        message(FATAL_ERROR "There are no supported WSI platforms on this system, vkcube requires a WSI platform be available to be able to render its output")
endif()

if (COMPILE_CUBE_SHADERS)
    # Try to find glslang in system paths or in an SDK if the VULKAN_SDK env-var is set
    find_program(GLSLANG_VALIDATOR names glslang glslangValidator HINTS $ENV{GLSLANG_INSTALL_DIR} $ENV{VULKAN_SDK}/bin $ENV{VULKAN_SDK}/Bin)

    add_custom_command(COMMENT "Compiling cube vertex shader"
                    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cube.vert.inc
                    COMMAND ${GLSLANG_VALIDATOR} -V -x -o ${CMAKE_CURRENT_SOURCE_DIR}/cube.vert.inc
                            ${PROJECT_SOURCE_DIR}/cube/cube.vert
                    MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/cube/cube.vert
                    DEPENDS ${PROJECT_SOURCE_DIR}/cube/cube.vert ${GLSLANG_VALIDATOR})
    add_custom_command(COMMENT "Compiling cube fragment shader"
                    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cube.frag.inc
                    COMMAND ${GLSLANG_VALIDATOR} -V -x -o ${CMAKE_CURRENT_SOURCE_DIR}/cube.frag.inc
                            ${PROJECT_SOURCE_DIR}/cube/cube.frag
                    MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/cube/cube.frag
                    DEPENDS ${PROJECT_SOURCE_DIR}/cube/cube.frag ${GLSLANG_VALIDATOR})
endif()

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # vkcube / vkcube make use of M_PI and various other math defines.
    add_compile_definitions(_USE_MATH_DEFINES)
endif()


# On Ubuntu 20.04, it was found that the validation layers fail to launch in vkcube & vkcubepp due to
# a missing dependency on libpthread. While newer Ubuntu versions use a glibc version where libpthread
# is integrated into libc, older ubuntu's do not so we need to link threads directly in order for
# validation layers to be loadable.
if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    find_package(Threads REQUIRED)
endif()

# ----------------------------------------------------------------------------
# vkcube

if(APPLE)
    add_subdirectory(macOS/cube)
elseif (ANDROID)
    add_library(vkcube MODULE)

    target_sources(vkcube PRIVATE cube.c)

    add_subdirectory(android)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    add_executable(vkcube)
    target_sources(vkcube PRIVATE
        cube.c
        ${PROJECT_SOURCE_DIR}/cube/cube.vert
        ${PROJECT_SOURCE_DIR}/cube/cube.frag
        cube.vert.inc
        cube.frag.inc
    )
    target_link_libraries(vkcube  Threads::Threads)
    if(BUILD_WSI_XCB_SUPPORT)
        target_sources(vkcube PRIVATE xcb_loader.h)
        target_include_directories(vkcube PRIVATE ${xcb_INCLUDE_DIRS})
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        target_sources(vkcube PRIVATE xlib_loader.h)
        target_include_directories(vkcube PRIVATE ${XLIB_INCLUDE_DIRS})
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_include_directories(vkcube PRIVATE ${WAYLAND_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
        target_sources(vkcube PRIVATE PRIVATE ${WAYLAND_ADDITIONAL_SOURCES})
    endif()
    if(BUILD_WSI_DIRECTFB_SUPPORT)
        target_link_libraries(vkcube PkgConfig::DirectFB)
    endif()
    include(CheckLibraryExists)
    CHECK_LIBRARY_EXISTS("rt" clock_gettime "" NEED_RT)
    if (NEED_RT)
        target_link_libraries(vkcube  rt)
    endif()
elseif(WIN32)
    add_executable(vkcube WIN32)
    target_sources(vkcube PRIVATE
        cube.c
        ${PROJECT_SOURCE_DIR}/cube/cube.vert
        ${PROJECT_SOURCE_DIR}/cube/cube.frag
        cube.vert.inc
        cube.frag.inc
    )
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

target_compile_definitions(vkcube PRIVATE ${ENABLED_CUBE_PLATFORMS})
target_include_directories(vkcube PRIVATE .)
target_link_libraries(vkcube  Vulkan::Headers volk::volk_headers)

if (ANDROID)
    install(TARGETS vkcube DESTINATION ${CMAKE_INSTALL_LIBDIR})
elseif(APPLE)
    install(
        TARGETS vkcube
        # Install runtime dependencies like the Vulkan::Loader so the app is self-contained
        RUNTIME_DEPENDENCIES DESTINATION "cube/vkcube.app/Contents/Frameworks/"
        BUNDLE DESTINATION "cube"
    )
    set_target_properties(vkcube PROPERTIES
        INSTALL_RPATH @executable_path/../Frameworks
    )
else()
    install(TARGETS vkcube)
endif()

if (ANDROID)
    return()
endif()

if (XCB_LINK_LIBRARIES)
    target_compile_definitions(vkcube PRIVATE "XCB_LIBRARY=\"${XCB_LINK_LIBRARIES}\"")
endif()
if (X11_LINK_LIBRARIES)
    target_compile_definitions(vkcube PRIVATE "XLIB_LIBRARY=\"${X11_LINK_LIBRARIES}\"")
endif()
if (WAYLAND_CLIENT_LINK_LIBRARIES)
    target_compile_definitions(vkcube PRIVATE "WAYLAND_LIBRARY=\"${WAYLAND_CLIENT_LINK_LIBRARIES}\"")
endif()

# ----------------------------------------------------------------------------
# vkcubepp

if(APPLE)
    add_subdirectory(macOS/cubepp)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|GNU")
    add_executable(vkcubepp
                   cube.cpp
                   ${PROJECT_SOURCE_DIR}/cube/cube.vert
                   ${PROJECT_SOURCE_DIR}/cube/cube.frag
                   cube.vert.inc
                   cube.frag.inc)
    target_link_libraries(vkcubepp  Threads::Threads)

    if(BUILD_WSI_XCB_SUPPORT)
        target_sources(vkcubepp PRIVATE xcb_loader.h)
        target_include_directories(vkcubepp PRIVATE ${xcb_INCLUDE_DIRS})
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        target_sources(vkcubepp PRIVATE xlib_loader.h)
        target_include_directories(vkcubepp PRIVATE ${XLIB_INCLUDE_DIRS})
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_include_directories(vkcubepp PRIVATE ${WAYLAND_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
        target_sources(vkcubepp PRIVATE ${WAYLAND_ADDITIONAL_SOURCES})
    endif()
    if(BUILD_WSI_DIRECTFB_SUPPORT)
        target_link_libraries(vkcubepp  PkgConfig::DirectFB)
    endif()
else()
    add_executable(vkcubepp
                   WIN32
                   cube.cpp
                   ${PROJECT_SOURCE_DIR}/cube/cube.vert
                   ${PROJECT_SOURCE_DIR}/cube/cube.frag
                   cube.vert.inc
                   cube.frag.inc)
endif()

target_include_directories(vkcubepp PRIVATE .)
target_compile_definitions(vkcubepp PRIVATE ${ENABLED_CUBE_PLATFORMS})
target_link_libraries(vkcubepp  Vulkan::Headers volk::volk_headers)

if (XCB_LINK_LIBRARIES )
    target_compile_definitions(vkcubepp PUBLIC "XCB_LIBRARY=\"${XCB_LINK_LIBRARIES}\"")
endif()
if (X11_LINK_LIBRARIES)
    target_compile_definitions(vkcubepp PUBLIC "XLIB_LIBRARY=\"${X11_LINK_LIBRARIES}\"")
endif()
if (WAYLAND_CLIENT_LINK_LIBRARIES)
    target_compile_definitions(vkcubepp PUBLIC "WAYLAND_LIBRARY=\"${WAYLAND_CLIENT_LINK_LIBRARIES}\"")
endif()

if(APPLE)
    install(
        TARGETS vkcubepp
        # Install runtime dependencies like the Vulkan::Loader so the app is self-contained
        RUNTIME_DEPENDENCIES DESTINATION "cube/vkcubepp.app/Contents/Frameworks/"
        BUNDLE DESTINATION "cube"
    )
    set_target_properties(vkcubepp PROPERTIES
        INSTALL_RPATH @executable_path/../Frameworks
    )
else()
    install(TARGETS vkcubepp)
endif()
