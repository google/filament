//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkSimple.cpp:
//    Implements the class methods for WindowSurfaceVkSimple.
//

#include "WindowSurfaceVkSimple.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

WindowSurfaceVkSimple::WindowSurfaceVkSimple(const egl::SurfaceState &surfaceState,
                                             EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

WindowSurfaceVkSimple::~WindowSurfaceVkSimple() {}

angle::Result WindowSurfaceVkSimple::createSurfaceVk(vk::ErrorContext *context,
                                                     gl::Extents *extentsOut)
{
    vk::Renderer *renderer = context->getRenderer();
    ASSERT(renderer != nullptr);
    VkInstance instance             = renderer->getInstance();
    VkPhysicalDevice physicalDevice = renderer->getPhysicalDevice();

    // Query if there is a valid display
    uint32_t count = 1;
    ANGLE_VK_TRY(context, vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &count, nullptr));

    // Get display properties
    VkDisplayPropertiesKHR prop = {};
    count                       = 1;
    ANGLE_VK_TRY(context, vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &count, &prop));

    // we should have a valid display now
    ASSERT(prop.display != VK_NULL_HANDLE);
    ANGLE_VK_TRY(context,
                 vkGetDisplayModePropertiesKHR(physicalDevice, prop.display, &count, nullptr));

    ASSERT(count != 0);
    std::vector<VkDisplayModePropertiesKHR> modeProperties(count);
    ANGLE_VK_TRY(context, vkGetDisplayModePropertiesKHR(physicalDevice, prop.display, &count,
                                                        modeProperties.data()));

    angle::vk::SimpleDisplayWindow *displayWindow =
        reinterpret_cast<angle::vk::SimpleDisplayWindow *>(mNativeWindowType);
    VkDisplaySurfaceCreateInfoKHR info = {};
    info.sType                         = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
    info.flags                         = 0;
    info.displayMode                   = modeProperties[0].displayMode;
    info.planeIndex                    = 0;
    info.planeStackIndex               = 0;
    info.transform                     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.globalAlpha                   = 1.0f;
    info.alphaMode                     = VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR;
    info.imageExtent.width             = displayWindow->width;
    info.imageExtent.height            = displayWindow->height;

    ANGLE_VK_TRY(context, vkCreateDisplayPlaneSurfaceKHR(instance, &info, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkSimple::getCurrentWindowSize(vk::ErrorContext *context,
                                                          gl::Extents *extentsOut)
{
    vk::Renderer *renderer                 = context->getRenderer();
    const VkPhysicalDevice &physicalDevice = renderer->getPhysicalDevice();

    ANGLE_VK_TRY(context, vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface,
                                                                    &mSurfaceCaps));

    *extentsOut =
        gl::Extents(mSurfaceCaps.currentExtent.width, mSurfaceCaps.currentExtent.height, 1);
    return angle::Result::Continue;
}

}  // namespace rx
