//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkAndroid.cpp:
//    Implements the class methods for WindowSurfaceVkAndroid.
//

#include "libANGLE/renderer/vulkan/android/WindowSurfaceVkAndroid.h"

#include <android/native_window.h>

#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

WindowSurfaceVkAndroid::WindowSurfaceVkAndroid(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkAndroid::createSurfaceVk(vk::ErrorContext *context,
                                                      gl::Extents *extentsOut)
{
    VkAndroidSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.flags  = 0;
    createInfo.window = mNativeWindowType;
    ANGLE_VK_TRY(context, vkCreateAndroidSurfaceKHR(context->getRenderer()->getInstance(),
                                                    &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkAndroid::getCurrentWindowSize(vk::ErrorContext *context,
                                                           gl::Extents *extentsOut)
{
    vk::Renderer *renderer                 = context->getRenderer();
    const VkPhysicalDevice &physicalDevice = renderer->getPhysicalDevice();
    VkSurfaceCapabilitiesKHR surfaceCaps;
    ANGLE_VK_TRY(context,
                 vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &surfaceCaps));
    *extentsOut = gl::Extents(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1);
    return angle::Result::Continue;
}

}  // namespace rx
