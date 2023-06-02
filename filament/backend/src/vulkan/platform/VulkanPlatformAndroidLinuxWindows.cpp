/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <backend/platforms/VulkanPlatform.h>

#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#if defined(__linux__) || defined(__FreeBSD__)
#define LINUX_OR_FREEBSD 1
#endif

// Platform specific includes and defines
#if defined(__ANDROID__)
    #include <android/native_window.h>
#elif defined(__linux__) && defined(FILAMENT_SUPPORTS_GGP)
    #include <ggp_c/ggp.h>
#elif defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    #include <dlfcn.h>
    namespace {
        typedef struct _wl {
            struct wl_display* display;
            struct wl_surface* surface;
            uint32_t width;
            uint32_t height;
        } wl;
    }// anonymous namespace
#elif LINUX_OR_FREEBSD && defined(FILAMENT_SUPPORTS_X11)
    // TODO: we should allow for headless on Linux explicitly. Right now this is the headless path
    // (with no FILAMENT_SUPPORTS_XCB or FILAMENT_SUPPORTS_XLIB).
    #include <dlfcn.h>
    #if defined(FILAMENT_SUPPORTS_XCB)
        #include <xcb/xcb.h>
        namespace {
            typedef xcb_connection_t* (*XCB_CONNECT)(const char* displayname, int* screenp);
        }
    #endif
    #if defined(FILAMENT_SUPPORTS_XLIB)
        #include <X11/Xlib.h>
        namespace {
            typedef Display* (*X11_OPEN_DISPLAY)(const char*);
        }
    #endif
    static constexpr const char* LIBRARY_X11 = "libX11.so.6";
    namespace {
        struct XEnv {
        #if defined(FILAMENT_SUPPORTS_XCB)
            XCB_CONNECT xcbConnect;
            xcb_connection_t* connection;
        #endif
        #if defined(FILAMENT_SUPPORTS_XLIB)
            X11_OPEN_DISPLAY openDisplay;
            Display* display;
        #endif
            void* library = nullptr;
        } g_x11_vk;
    }// anonymous namespace
#elif defined(WIN32)
    // No platform specific includes
#else
    #error Not a supported Vulkan platform
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExtensionSet VulkanPlatform::getRequiredInstanceExtensions() {
    VulkanPlatform::ExtensionSet ret;
    #if defined(__ANDROID__)
        ret.insert("VK_KHR_android_surface");
    #elif defined(__linux__) && defined(FILAMENT_SUPPORTS_GGP)
        ret.insert(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME);
    #elif defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
        ret.insert("VK_KHR_wayland_surface");
    #elif LINUX_OR_FREEBSD && defined(FILAMENT_SUPPORTS_X11)
        #if defined(FILAMENT_SUPPORTS_XCB)
            ret.insert("VK_KHR_xcb_surface");
        #endif
        #if defined(FILAMENT_SUPPORTS_XLIB)
            ret.insert("VK_KHR_xlib_surface");
        #endif
    #elif defined(WIN32)
        ret.insert("VK_KHR_win32_surface");
    #endif
    return ret;
}

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHR(void* nativeWindow,
        VkInstance instance, uint64_t flags) noexcept {
    VkSurfaceKHR surface;

    // On certain platforms, the extent of the surface cannot be queried from Vulkan. In those
    // situations, we allow the frontend to pass in the extent to use in creating the swap
    // chains. Platform implementation should set extent to 0 if they do not expect to set the
    // swap chain extent.
    VkExtent2D extent;

    #if defined(__ANDROID__)
        VkAndroidSurfaceCreateInfoKHR const createInfo{
                .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                .window = (ANativeWindow*) nativeWindow,
        };
        VkResult const result = vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC,
                (VkSurfaceKHR*) &surface);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateAndroidSurfaceKHR error.");
    #elif defined(__linux__) && defined(FILAMENT_SUPPORTS_GGP)
        VkStreamDescriptorSurfaceCreateInfoGGP const surface_create_info = {
                .sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP,
                .streamDescriptor = kGgpPrimaryStreamDescriptor,
        };
        PFN_vkCreateStreamDescriptorSurfaceGGP fpCreateStreamDescriptorSurfaceGGP
                = reinterpret_cast<PFN_vkCreateStreamDescriptorSurfaceGGP>(
                        vkGetInstanceProcAddr(instance, "vkCreateStreamDescriptorSurfaceGGP"));
        ASSERT_PRECONDITION(fpCreateStreamDescriptorSurfaceGGP != nullptr,
                "Error getting VkInstance "
                "function vkCreateStreamDescriptorSurfaceGGP");
        VkResult const result = fpCreateStreamDescriptorSurfaceGGP(instance, &surface_create_info,
                nullptr, (VkSurfaceKHR*) &surface);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateStreamDescriptorSurfaceGGP error.");
    #elif defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
        wl* ptrval = reinterpret_cast<wl*>(nativeWindow);
        extent.width = ptrval->width;
        extent.height = ptrval->height;

        VkWaylandSurfaceCreateInfoKHR const createInfo = {
                .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                .pNext = NULL,
                .flags = 0,
                .display = ptrval->display,
                .surface = ptrval->surface,
        };
        VkResult const result = vkCreateWaylandSurfaceKHR(instance, &createInfo, VKALLOC,
                (VkSurfaceKHR*) &surface);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateWaylandSurfaceKHR error.");
    #elif LINUX_OR_FREEBSD && defined(FILAMENT_SUPPORTS_X11)
        if (g_x11_vk.library == nullptr) {
            g_x11_vk.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
            ASSERT_PRECONDITION(g_x11_vk.library, "Unable to open X11 library.");
            #if defined(FILAMENT_SUPPORTS_XCB)
                g_x11_vk.xcbConnect = (XCB_CONNECT) dlsym(g_x11_vk.library, "xcb_connect");
                int screen;
                g_x11_vk.connection = g_x11_vk.xcbConnect(nullptr, &screen);
                ASSERT_POSTCONDITION(vkCreateXcbSurfaceKHR,
                        "Unable to load vkCreateXcbSurfaceKHR function.");
            #endif
            #if defined(FILAMENT_SUPPORTS_XLIB)
                g_x11_vk.openDisplay = (X11_OPEN_DISPLAY) dlsym(g_x11_vk.library, "XOpenDisplay");
                g_x11_vk.display = g_x11_vk.openDisplay(NULL);
                ASSERT_PRECONDITION(g_x11_vk.display, "Unable to open X11 display.");
                ASSERT_POSTCONDITION(vkCreateXlibSurfaceKHR,
                        "Unable to load vkCreateXlibSurfaceKHR function.");
            #endif
        }
        #if defined(FILAMENT_SUPPORTS_XCB) || defined(FILAMENT_SUPPORTS_XLIB)
            bool useXcb = false;
        #endif
        #if defined(FILAMENT_SUPPORTS_XCB)
            #if defined(FILAMENT_SUPPORTS_XLIB)
                useXcb = (flags & SWAP_CHAIN_CONFIG_ENABLE_XCB) != 0;
            #else
                useXcb = true;
            #endif
            if (useXcb) {
                VkXcbSurfaceCreateInfoKHR const createInfo = {
                        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                        .connection = g_x11_vk.connection,
                        .window = (xcb_window_t) reinterpret_cast<uint64_t>(nativeWindow),
                };
                VkResult const result = vkCreateXcbSurfaceKHR(instance, &createInfo, VKALLOC,
                        (VkSurfaceKHR*) &surface);
                ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateXcbSurfaceKHR error.");
            }
        #endif
        #if defined(FILAMENT_SUPPORTS_XLIB)
            if (!useXcb) {
                VkXlibSurfaceCreateInfoKHR const createInfo = {
                        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                        .dpy = g_x11_vk.display,
                        .window = (Window) nativeWindow,
                };
                VkResult const result = vkCreateXlibSurfaceKHR(instance, &createInfo, VKALLOC,
                        (VkSurfaceKHR*) &surface);
                ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateXlibSurfaceKHR error.");
            }
        #endif
    #elif defined(WIN32)
        VkWin32SurfaceCreateInfoKHR const createInfo = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = (HWND) nativeWindow,
        };
        VkResult const result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr,
                (VkSurfaceKHR*) &surface);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateWin32SurfaceKHR error.");
    #endif
    return std::make_tuple(surface, extent);
}

} // namespace filament::backend

#undef LINUX_OR_FREEBSD
