# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

add_library(VulkanCompilerConfiguration INTERFACE)
add_library(Vulkan::CompilerConfiguration ALIAS VulkanCompilerConfiguration)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(VulkanCompilerConfiguration INTERFACE
        -Wpedantic
        -Wunreachable-code
        -Wunused-function
        -Wall
        -Wextra
        -Wpointer-arith
        -Wextra-semi
    )
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        target_compile_options(VulkanCompilerConfiguration INTERFACE
            -Wunreachable-code-return
            -Wconversion
            -Wimplicit-fallthrough
            -Wstring-conversion
        )
    endif()
elseif(MSVC)
    target_compile_options(VulkanCompilerConfiguration INTERFACE
        /W4
        /we5038 # Enable warning about MIL ordering in constructors
	/wd4324 # Disable warning about alignment padding
    )

    # Enforce stricter ISO C++
    target_compile_options(VulkanCompilerConfiguration INTERFACE /permissive-)

    # Speed up Visual Studio builds
    if (MSVC_IDE)
        target_compile_options(VulkanCompilerConfiguration INTERFACE /MP)
    endif()
endif()

target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_ENABLE_BETA_EXTENSIONS)
if(WIN32)
    # Minimize what Windows.h leaks
    target_compile_definitions(VulkanCompilerConfiguration INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)

    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_METAL_EXT)
    if (IOS)
        target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_IOS_MVK)
    else()
        target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_MACOS_MVK)
    endif()
else()
    message(DEBUG "Figure out how to gracefully handle Linux|BSD WSI...")
    
    #option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    #option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    #option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)

    #find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    #if(BUILD_WSI_XCB_SUPPORT)
    #    pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
    #    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_XCB_KHR)
    #endif()

    #if(BUILD_WSI_XLIB_SUPPORT)
    #    pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
    #    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    #endif()

    #if(BUILD_WSI_WAYLAND_SUPPORT)
    #    pkg_check_modules(WAYlAND_CLIENT QUIET REQUIRED IMPORTED_TARGET wayland-client)
    #    target_compile_definitions(VulkanCompilerConfiguration INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    #endif()
endif()

option(VUL_WERROR "Treat compiler warnings as errors")
if (VUL_WERROR)
    if (MSVC)
        target_compile_options(VulkanCompilerConfiguration INTERFACE /WX)
        target_link_options(VulkanCompilerConfiguration INTERFACE /WX)
    else()
        target_compile_options(VulkanCompilerConfiguration INTERFACE -Werror)
        # TODO: Figure out linker warnings as errors for non-WIN32
    endif()
endif()

option(VUL_MOCK_ANDROID "Enable building for Android on desktop for testing with MockICD setup")

add_subdirectory(layer)
add_subdirectory(vulkan)
