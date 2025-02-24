//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWin32.cpp:
//    Implements the class methods for WindowSurfaceVkWin32.
//

#include "libANGLE/renderer/vulkan/win32/WindowSurfaceVkWin32.h"

#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

WindowSurfaceVkWin32::WindowSurfaceVkWin32(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkWin32::createSurfaceVk(vk::ErrorContext *context,
                                                    gl::Extents *extentsOut)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {};

    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.flags     = 0;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd      = mNativeWindowType;
    ANGLE_VK_TRY(context, vkCreateWin32SurfaceKHR(context->getRenderer()->getInstance(),
                                                  &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkWin32::getCurrentWindowSize(vk::ErrorContext *context,
                                                         gl::Extents *extentsOut)
{
    RECT rect;
    ANGLE_VK_CHECK(context, GetClientRect(mNativeWindowType, &rect) == TRUE,
                   VK_ERROR_INITIALIZATION_FAILED);

    *extentsOut = gl::Extents(rect.right - rect.left, rect.bottom - rect.top, 1);
    return angle::Result::Continue;
}

}  // namespace rx
