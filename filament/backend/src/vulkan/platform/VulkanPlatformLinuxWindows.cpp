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
#include <backend/DriverEnums.h>

#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <tuple>

#include <stdint.h>
#include <stddef.h>

#if defined(__linux__) || defined(__FreeBSD__)
#define LINUX_OR_FREEBSD 1
#endif

// Platform specific includes and defines
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    #include <dlfcn.h>
    namespace {
        typedef struct _wl {
            struct wl_display* display;
            struct wl_surface* surface;
            uint32_t width;
            uint32_t height;
        } wl;
    }// anonymous namespace
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
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
    // Not a supported Vulkan platform
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExternalImageMetadata VulkanPlatform::getExternalImageMetadataImpl(
        ExternalImageHandleRef externalImage, VkDevice device) {
    return {};
}

VulkanPlatform::ImageData VulkanPlatform::createExternalImageDataImpl(
        ExternalImageHandleRef externalImage, VkDevice device,
        const ExternalImageMetadata& metadata, uint32_t memoryTypeIndex, VkImageUsageFlags usage) {
    return {};
}

VkSampler VulkanPlatform::createExternalSamplerImpl(VkDevice device,
    SamplerYcbcrConversion chroma,
    SamplerParams sampler,
    uint32_t internalFormat) {
    return VK_NULL_HANDLE;
}

VkImageView VulkanPlatform::createExternalImageViewImpl(VkDevice device,
        SamplerYcbcrConversion chroma, uint32_t internalFormat, VkImage image,
        VkImageSubresourceRange range, VkImageViewType viewType, VkComponentMapping swizzle) {
    return VK_NULL_HANDLE;
}

VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensions() {
    VulkanPlatform::ExtensionSet const ret = {
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
    #if defined(FILAMENT_SUPPORTS_XCB)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    #endif
    #if defined(FILAMENT_SUPPORTS_XLIB)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    #endif
#elif defined(WIN32)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    };
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

    #if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
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
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateWaylandSurfaceKHR error.";
    #elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
        if (g_x11_vk.library == nullptr) {
            g_x11_vk.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
            FILAMENT_CHECK_PRECONDITION(g_x11_vk.library) << "Unable to open X11 library.";
            #if defined(FILAMENT_SUPPORTS_XCB)
                g_x11_vk.xcbConnect = (XCB_CONNECT) dlsym(g_x11_vk.library, "xcb_connect");
                int screen;
                g_x11_vk.connection = g_x11_vk.xcbConnect(nullptr, &screen);
            #endif
            #if defined(FILAMENT_SUPPORTS_XLIB)
                g_x11_vk.openDisplay = (X11_OPEN_DISPLAY) dlsym(g_x11_vk.library, "XOpenDisplay");
                g_x11_vk.display = g_x11_vk.openDisplay(NULL);
                FILAMENT_CHECK_PRECONDITION(g_x11_vk.display) << "Unable to open X11 display.";
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
                FILAMENT_CHECK_POSTCONDITION(vkCreateXcbSurfaceKHR)
                        << "Unable to load vkCreateXcbSurfaceKHR function.";

                VkXcbSurfaceCreateInfoKHR const createInfo = {
                        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                        .connection = g_x11_vk.connection,
                        .window = (xcb_window_t) reinterpret_cast<uint64_t>(nativeWindow),
                };
                VkResult const result = vkCreateXcbSurfaceKHR(instance, &createInfo, VKALLOC,
                        (VkSurfaceKHR*) &surface);
                FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                        << "vkCreateXcbSurfaceKHR error=" << static_cast<int32_t>(result);
            }
        #endif
        #if defined(FILAMENT_SUPPORTS_XLIB)
            if (!useXcb) {
                FILAMENT_CHECK_POSTCONDITION(vkCreateXlibSurfaceKHR)
                        << "Unable to load vkCreateXlibSurfaceKHR function.";

                VkXlibSurfaceCreateInfoKHR const createInfo = {
                        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                        .dpy = g_x11_vk.display,
                        .window = (Window) nativeWindow,
                };
                VkResult const result = vkCreateXlibSurfaceKHR(instance, &createInfo, VKALLOC,
                        (VkSurfaceKHR*) &surface);
                FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                        << "vkCreateXlibSurfaceKHR error=" << static_cast<int32_t>(result);
            }
        #endif
    #elif defined(WIN32)
        // On (at least) NVIDIA drivers, the Vulkan implementation (specifically the call to
        // vkGetPhysicalDeviceSurfaceCapabilitiesKHR()) does not correctly handle the fact that
        // each native window has its own DPI_AWARENESS_CONTEXT, and erroneously uses the context
        // of the calling thread. As a workaround, we set the current thread's DPI_AWARENESS_CONTEXT
        // to that of the native window we've been given. This isn't a perfect solution, because an
        // application could create swap chains on multiple native windows with varying DPI-awareness,
        // but even then, at least one of the windows would be guaranteed to work correctly.
        SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext((HWND) nativeWindow));

        VkWin32SurfaceCreateInfoKHR const createInfo = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = (HWND) nativeWindow,
        };
        VkResult const result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr,
                (VkSurfaceKHR*) &surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "vkCreateWin32SurfaceKHR failed."
                << " error=" << static_cast<int32_t>(result);
#endif
        return std::make_tuple(surface, extent);
}

} // namespace filament::backend

#undef LINUX_OR_FREEBSD
