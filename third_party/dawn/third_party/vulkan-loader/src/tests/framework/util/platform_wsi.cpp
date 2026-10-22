/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#include "platform_wsi.h"

#include "equality_helpers.h"

const char* get_platform_wsi_extension([[maybe_unused]] const char* api_selection) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    return "VK_KHR_android_surface";
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    return "VK_EXT_directfb_surface";
#elif defined(VK_USE_PLATFORM_FUCHSIA)
    return "VK_FUCHSIA_imagepipe_surface";
#elif defined(VK_USE_PLATFORM_GGP)
    return "VK_GGP_stream_descriptor_surface";
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    return "VK_MVK_ios_surface";
#elif defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (string_eq(api_selection, "VK_USE_PLATFORM_MACOS_MVK")) return "VK_MVK_macos_surface";
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (string_eq(api_selection, "VK_USE_PLATFORM_METAL_EXT")) return "VK_EXT_metal_surface";
    return "VK_EXT_metal_surface";
#endif
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    return "VK_QNX_screen_surface";
#elif defined(VK_USE_PLATFORM_VI_NN)
    return "VK_NN_vi_surface";
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR)
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_XCB_KHR")) return "VK_KHR_xcb_surface";
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_XLIB_KHR")) return "VK_KHR_xlib_surface";
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_WAYLAND_KHR")) return "VK_KHR_wayland_surface";
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    return "VK_KHR_xcb_surface";
#endif
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    return "VK_KHR_win32_surface";
#else
    return "VK_KHR_display";
#endif
}
