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
#if defined(__ANDROID__)
#include <android/hardware_buffer.h>
#include <android/native_window.h>
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

#if defined(__ANDROID__)
    void GetVKFormatAndUsage(const AHardwareBuffer_Desc& desc,
        VkFormat& format,
        VkImageUsageFlags& usage,
        bool& isProtected) {
        // Refer to "11.2.17. External Memory Handle Types" in the spec, and
        // Tables 13/14 for how the following derivation works.
        bool is_depth_format = false;
        isProtected = false;
        switch (desc.format) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            format = VK_FORMAT_R8G8B8_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        case AHARDWAREBUFFER_FORMAT_D16_UNORM:
            is_depth_format = true;
            format = VK_FORMAT_D16_UNORM;
            break;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM:
            is_depth_format = true;
            format = VK_FORMAT_X8_D24_UNORM_PACK32;
            break;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
            is_depth_format = true;
            format = VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
            is_depth_format = true;
            format = VK_FORMAT_D32_SFLOAT;
            break;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
            is_depth_format = true;
            format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            break;
        case AHARDWAREBUFFER_FORMAT_S8_UINT:
            is_depth_format = true;
            format = VK_FORMAT_S8_UINT;
            break;
        default:
            format = VK_FORMAT_UNDEFINED;
        }

        // The following only concern usage flags derived from Table 14.
        usage = 0;
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
            if (is_depth_format) {
                usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else {
                usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
            usage = VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) {
            isProtected = true;
        }
    }
#endif

//    void VulkanPlatform::describeExternalImageOS(void* image, VkFormat& format, uint32_t& width, uint32_t& height, uint32_t& depth, VkImageUsageFlags& usage, bool& isProtected)
//    {
//#if defined(__ANDROID__)
//        AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(image);
//        AHardwareBuffer_Desc buffer_desc;
//        AHardwareBuffer_describe(buffer, &buffer_desc);
//
//        GetVKFormatAndUsage(buffer_desc, format, usage, isProtected);
//
//        width = buffer_desc.width;
//        height = buffer_desc.height;
//
//        // This might not be the right translation
//        depth = buffer_desc.layers;
//#endif
//    }
    uint32_t VulkanPlatform::getExternalImageMemoryBits(void* externalBuffer, VkDevice device)
    {
        uint32_t bits = 0;
#if defined(__ANDROID__)
        AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
        VkAndroidHardwareBufferFormatPropertiesANDROID format_info = {
        .sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
        .pNext = nullptr,
        };
        VkAndroidHardwareBufferPropertiesANDROID properties = {
            .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
            .pNext = &format_info,
        };
        vkGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &properties);
        bits = properties.memoryTypeBits;
#endif //__ANDROID__
        return bits;
    }
    void VulkanPlatform::createExternalImage(void* externalBuffer, VkDevice device, const VkAllocationCallbacks* allocator, VkImage* pImage) {
#if defined(__ANDROID__)
        AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
        AHardwareBuffer_Desc buffer_desc;
        AHardwareBuffer_describe(buffer, &buffer_desc);

        VkFormat format;
        VkImageUsageFlags usage;
        //technically we don't need the format (since whe will query it in the following APIs
        //directly from VK). But we still need to check the format to differenciate DS from Color
        bool isProtected;
        GetVKFormatAndUsage(buffer_desc, format, usage, isProtected);

        // All this work now is for external formats (query the underlying VK for the format)
        VkAndroidHardwareBufferFormatPropertiesANDROID format_info = {
            .sType =
                VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
            .pNext = nullptr,
        };
        VkAndroidHardwareBufferPropertiesANDROID properties = {
            .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
            .pNext = &format_info,
        };
        vkGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &properties);

        //if external format we need to specifiy it in the allocation
        const bool use_external_format = format_info.format == VK_FORMAT_UNDEFINED;

        const VkExternalFormatANDROID external_format = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
            .pNext = nullptr,
            .externalFormat = format_info.externalFormat,//pass down the format (external means we don't have it VK defined)
        };
        const VkExternalMemoryImageCreateInfo external_create_info = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
            .pNext = use_external_format ? &external_format : nullptr,
            .handleTypes =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
        };

        VkImageCreateInfo imageInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .pNext = nullptr };
        imageInfo.pNext = &external_create_info;
        imageInfo.format = format_info.format;
        imageInfo.extent =
        {
            buffer_desc.width,
            buffer_desc.height,
            1u,
        },
        imageInfo.arrayLayers = buffer_desc.layers;
        imageInfo.usage =
            (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage);

        VkResult const result =
            vkCreateImage(device, &imageInfo, allocator, pImage);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);
#endif
    }
    void VulkanPlatform::allocateExternalImage(void* externalBuffer, VkDevice device, const VkAllocationCallbacks* allocator, VkImage pImage, uint32_t memoryTypeIndex, VkDeviceMemory* pMemory) {
#if defined(__ANDROID__)
        AHardwareBuffer* buffer = static_cast<AHardwareBuffer*>(externalBuffer);
        VkAndroidHardwareBufferFormatPropertiesANDROID format_info = {
        .sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
        .pNext = nullptr,
        };
        VkAndroidHardwareBufferPropertiesANDROID properties = {
            .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
            .pNext = &format_info,
        };
        vkGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &properties);

        // Now handle the allocation
        VkImportAndroidHardwareBufferInfoANDROID android_hardware_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
            .pNext = nullptr,
            .buffer = buffer,
        };
        VkMemoryDedicatedAllocateInfo memory_dedicated_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
            .pNext = &android_hardware_buffer_info,
            .image = pImage,
            .buffer = VK_NULL_HANDLE,
        };
        VkMemoryAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = &memory_dedicated_allocate_info,
            .allocationSize = properties.allocationSize,
            .memoryTypeIndex = memoryTypeIndex };
        VkResult const result =
            vkAllocateMemory(device, &alloc_info, allocator, pMemory);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS);
#endif
    }


    VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensions() {
        VulkanPlatform::ExtensionSet const ret = {
    #if defined(__ANDROID__)
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    #elif defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
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

#if defined(__ANDROID__)
        VkAndroidSurfaceCreateInfoKHR const createInfo{
                .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                .window = (ANativeWindow*)nativeWindow,
        };
        VkResult const result = vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC,
            (VkSurfaceKHR*)&surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateAndroidSurfaceKHR error.";
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
            (VkSurfaceKHR*)&surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateWaylandSurfaceKHR error.";
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
        if (g_x11_vk.library == nullptr) {
            g_x11_vk.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
            FILAMENT_CHECK_PRECONDITION(g_x11_vk.library) << "Unable to open X11 library.";
#if defined(FILAMENT_SUPPORTS_XCB)
            g_x11_vk.xcbConnect = (XCB_CONNECT)dlsym(g_x11_vk.library, "xcb_connect");
            int screen;
            g_x11_vk.connection = g_x11_vk.xcbConnect(nullptr, &screen);
#endif
#if defined(FILAMENT_SUPPORTS_XLIB)
            g_x11_vk.openDisplay = (X11_OPEN_DISPLAY)dlsym(g_x11_vk.library, "XOpenDisplay");
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
                (VkSurfaceKHR*)&surface);
            FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "vkCreateXcbSurfaceKHR error.";
        }
#endif
#if defined(FILAMENT_SUPPORTS_XLIB)
        if (!useXcb) {
            FILAMENT_CHECK_POSTCONDITION(vkCreateXlibSurfaceKHR)
                << "Unable to load vkCreateXlibSurfaceKHR function.";

            VkXlibSurfaceCreateInfoKHR const createInfo = {
                    .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                    .dpy = g_x11_vk.display,
                    .window = (Window)nativeWindow,
            };
            VkResult const result = vkCreateXlibSurfaceKHR(instance, &createInfo, VKALLOC,
                (VkSurfaceKHR*)&surface);
            FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "vkCreateXlibSurfaceKHR error.";
        }
#endif
#elif defined(WIN32)
        VkWin32SurfaceCreateInfoKHR const createInfo = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = (HWND)nativeWindow,
        };
        VkResult const result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr,
            (VkSurfaceKHR*)&surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateWin32SurfaceKHR error.";
#endif
        return std::make_tuple(surface, extent);
    }

} // namespace filament::backend

#undef LINUX_OR_FREEBSD
