//
// Copyright 2021-2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWayland.cpp:
//    Implements the class methods for WindowSurfaceVkWayland.
//

#include "libANGLE/renderer/vulkan/linux/wayland/WindowSurfaceVkWayland.h"

#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include <wayland-egl-backend.h>

namespace rx
{

void WindowSurfaceVkWayland::ResizeCallback(wl_egl_window *eglWindow, void *payload)
{
    WindowSurfaceVkWayland *windowSurface = reinterpret_cast<WindowSurfaceVkWayland *>(payload);

    windowSurface->mExtents.width  = eglWindow->width;
    windowSurface->mExtents.height = eglWindow->height;
}

WindowSurfaceVkWayland::WindowSurfaceVkWayland(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window,
                                               wl_display *display)
    : WindowSurfaceVk(surfaceState, window), mWaylandDisplay(display)
{
    wl_egl_window *eglWindow   = reinterpret_cast<wl_egl_window *>(window);
    eglWindow->resize_callback = WindowSurfaceVkWayland::ResizeCallback;
    eglWindow->driver_private  = this;

    mExtents = gl::Extents(eglWindow->width, eglWindow->height, 1);
}

angle::Result WindowSurfaceVkWayland::createSurfaceVk(vk::ErrorContext *context,
                                                      gl::Extents *extentsOut)
{
    ANGLE_VK_CHECK(context,
                   vkGetPhysicalDeviceWaylandPresentationSupportKHR(
                       context->getRenderer()->getPhysicalDevice(),
                       context->getRenderer()->getQueueFamilyIndex(), mWaylandDisplay),
                   VK_ERROR_INITIALIZATION_FAILED);

    wl_egl_window *eglWindow = reinterpret_cast<wl_egl_window *>(mNativeWindowType);

    VkWaylandSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.flags   = 0;
    createInfo.display = mWaylandDisplay;
    createInfo.surface = eglWindow->surface;
    ANGLE_VK_TRY(context, vkCreateWaylandSurfaceKHR(context->getRenderer()->getInstance(),
                                                    &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkWayland::getCurrentWindowSize(vk::ErrorContext *context,
                                                           gl::Extents *extentsOut)
{
    *extentsOut = mExtents;
    return angle::Result::Continue;
}

egl::Error WindowSurfaceVkWayland::getUserWidth(const egl::Display *display, EGLint *value) const
{
    *value = getWidth();
    return egl::NoError();
}

egl::Error WindowSurfaceVkWayland::getUserHeight(const egl::Display *display, EGLint *value) const
{
    *value = getHeight();
    return egl::NoError();
}

}  // namespace rx
