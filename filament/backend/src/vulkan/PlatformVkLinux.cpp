/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vulkan/PlatformVkLinux.h"

#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <dlfcn.h>

namespace filament {

using namespace backend;

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr VkAllocationCallbacks* VKALLOC = nullptr;

static constexpr const char* LIBRARY_X11 = "libX11.so.6";

#ifdef FILAMENT_SUPPORTS_XCB
typedef xcb_connection_t* (*XCB_CONNECT)(const char *displayname, int *screenp);
#else
typedef Display* (*X11_OPEN_DISPLAY)(const char*);
#endif

struct X11Functions {
#ifdef FILAMENT_SUPPORTS_XCB
    XCB_CONNECT xcbConnect;
#else
    X11_OPEN_DISPLAY openDisplay;
#endif
    void* library = nullptr;
} g_x11;

Driver* PlatformVkLinux::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    const char* requiredInstanceExtensions[] = {
        "VK_KHR_surface",
#ifdef FILAMENT_SUPPORTS_XCB
        "VK_KHR_xcb_surface",
#else
        "VK_KHR_xlib_surface",
#endif
        "VK_KHR_get_physical_device_properties2",
#if VK_ENABLE_VALIDATION
        "VK_EXT_debug_utils",
#endif
    };
    return VulkanDriverFactory::create(this, requiredInstanceExtensions,
            sizeof(requiredInstanceExtensions) / sizeof(requiredInstanceExtensions[0]));
}

void* PlatformVkLinux::createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept {
#ifdef FILAMENT_SUPPORTS_XCB
    if (g_x11.library == nullptr) {
        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        ASSERT_PRECONDITION(g_x11.library, "Unable to open X11 library.");
        g_x11.xcbConnect = (XCB_CONNECT) dlsym(g_x11.library, "xcb_connect");
        int screen;
        mConnection = g_x11.xcbConnect(nullptr, &screen);
    }
    ASSERT_POSTCONDITION(vkCreateXcbSurfaceKHR, "Unable to load vkCreateXcbSurfaceKHR function.");
    VkSurfaceKHR surface = nullptr;
    const uint64_t ptrval = reinterpret_cast<uint64_t>(nativeWindow);
    VkXcbSurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = mConnection,
        .window = (xcb_window_t) ptrval,
    };
    VkResult result = vkCreateXcbSurfaceKHR((VkInstance) instance, &createInfo, VKALLOC, &surface);
#else
    if (g_x11.library == nullptr) {
        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        ASSERT_PRECONDITION(g_x11.library, "Unable to open X11 library.");
        g_x11.openDisplay  = (X11_OPEN_DISPLAY)  dlsym(g_x11.library, "XOpenDisplay");
        mDisplay = g_x11.openDisplay(NULL);
        ASSERT_PRECONDITION(mDisplay, "Unable to open X11 display.");
    }
    ASSERT_POSTCONDITION(vkCreateXlibSurfaceKHR, "Unable to load vkCreateXlibSurfaceKHR function.");
    VkSurfaceKHR surface = nullptr;
    VkXlibSurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = mDisplay,
        .window = (Window) nativeWindow,
    };
    VkResult result = vkCreateXlibSurfaceKHR((VkInstance) instance, &createInfo, VKALLOC, &surface);
#endif
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateXlibSurfaceKHR error.");
    return surface;
}

} // namespace filament
