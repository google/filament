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

#include "vulkan/PlatformVkLinuxWayland.h"

#include "VulkanConstants.h"
#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <dlfcn.h>

using namespace bluevk;

namespace filament {

using namespace backend;

Driver* PlatformVkLinuxWayland::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    const char* requiredInstanceExtensions[] = {
        "VK_KHR_wayland_surface",
    };
    return VulkanDriverFactory::create(this, requiredInstanceExtensions,
            sizeof(requiredInstanceExtensions) / sizeof(requiredInstanceExtensions[0]));
}

void* PlatformVkLinuxWayland::createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept {

    typedef struct _wl {
        struct wl_display *display;
        struct wl_surface *surface;
    } wl;

    VkSurfaceKHR surface = nullptr;

    wl* ptrval = reinterpret_cast<wl*>(nativeWindow);

    VkWaylandSurfaceCreateInfoKHR createInfo = {
       .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
       .pNext = NULL,
       .flags = 0,
       .display = ptrval->display,
       .surface = ptrval->surface
    };

    vkCreateWaylandSurfaceKHR((VkInstance) instance, &createInfo, VKALLOC, &surface);

    return surface;
}

} // namespace filament
