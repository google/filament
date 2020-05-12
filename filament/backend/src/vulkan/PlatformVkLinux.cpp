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

typedef Display* (*X11_OPEN_DISPLAY)(const char*);
typedef Display* (*X11_CLOSE_DISPLAY)(Display*);
typedef Status (*X11_GET_GEOMETRY)(Display*, Drawable, Window*, int*, int*, unsigned int*,
        unsigned int*, unsigned int*, unsigned int*);

struct X11Functions {
    X11_OPEN_DISPLAY openDisplay;
    X11_CLOSE_DISPLAY closeDisplay;
    X11_GET_GEOMETRY getGeometry;
    void* library;
} g_x11;

Driver* PlatformVkLinux::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    const char* requestedExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_xlib_surface",
#if !defined(NDEBUG)
        "VK_EXT_debug_report",
#endif
    };
    g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
    ASSERT_PRECONDITION(g_x11.library, "Unable to open X11 library.");
    g_x11.openDisplay  = (X11_OPEN_DISPLAY)  dlsym(g_x11.library, "XOpenDisplay");
    g_x11.closeDisplay = (X11_CLOSE_DISPLAY) dlsym(g_x11.library, "XCloseDisplay");
    g_x11.getGeometry = (X11_GET_GEOMETRY) dlsym(g_x11.library, "XGetGeometry");
    mDisplay = g_x11.openDisplay(NULL);
    ASSERT_PRECONDITION(mDisplay, "Unable to open X11 display.");
    return VulkanDriverFactory::create(this, requestedExtensions,
            sizeof(requestedExtensions) / sizeof(requestedExtensions[0]));
}

void* PlatformVkLinux::createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept {
    ASSERT_POSTCONDITION(vkCreateXlibSurfaceKHR, "Unable to load vkCreateXlibSurfaceKHR function.");
    VkSurfaceKHR surface = nullptr;
    VkXlibSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = mDisplay;
    createInfo.window = (Window) nativeWindow;
    VkResult result = vkCreateXlibSurfaceKHR((VkInstance) instance, &createInfo, VKALLOC, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateXlibSurfaceKHR error.");
    return surface;
}

void PlatformVkLinux::getClientExtent(void* window, uint32_t* width, uint32_t* height) noexcept {
    Window root;
    int x = 0;
    int y = 0;
    unsigned int border_width = 0;
    unsigned int depth = 0;
    g_x11.getGeometry(mDisplay, (Window) window, &root, &x, &y, width, height, &border_width,
            &depth);
 }

} // namespace filament
