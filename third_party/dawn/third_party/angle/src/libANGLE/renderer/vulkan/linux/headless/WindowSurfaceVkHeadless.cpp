//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkHeadless.cpp:
//    Implements the class methods for WindowSurfaceVkHeadless.
//

#include "WindowSurfaceVkHeadless.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

WindowSurfaceVkHeadless::WindowSurfaceVkHeadless(const egl::SurfaceState &surfaceState,
                                                 EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

WindowSurfaceVkHeadless::~WindowSurfaceVkHeadless() {}

angle::Result WindowSurfaceVkHeadless::createSurfaceVk(vk::ErrorContext *context,
                                                       gl::Extents *extentsOut)
{
    vk::Renderer *renderer = context->getRenderer();
    ASSERT(renderer != nullptr);
    VkInstance instance = renderer->getInstance();

    VkHeadlessSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType                          = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;

    ANGLE_VK_TRY(context, vkCreateHeadlessSurfaceEXT(instance, &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkHeadless::getCurrentWindowSize(vk::ErrorContext *context,
                                                            gl::Extents *extentsOut)
{
    const VkPhysicalDevice &physicalDevice = context->getRenderer()->getPhysicalDevice();
    ANGLE_VK_TRY(context, vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface,
                                                                    &mSurfaceCaps));

    // Spec: "For headless surfaces, currentExtent is the reserved value (0xFFFFFFFF, 0xFFFFFFFF).
    // Whatever the application sets a swapchain's imageExtent to will be the size of the surface,
    // after the first image is presented."
    // For ANGLE, in headless mode, we share the same 'SimpleDisplayWindow' structure with front
    // EGL window info to define the vulkan backend surface/image extents.
    angle::vk::SimpleDisplayWindow *simpleWindow =
        reinterpret_cast<angle::vk::SimpleDisplayWindow *>(mNativeWindowType);

    // Update surface extent before output the new extent.
    mSurfaceCaps.currentExtent.width  = simpleWindow->width;
    mSurfaceCaps.currentExtent.height = simpleWindow->height;

    *extentsOut =
        gl::Extents(mSurfaceCaps.currentExtent.width, mSurfaceCaps.currentExtent.height, 1);

    return angle::Result::Continue;
}

}  // namespace rx
