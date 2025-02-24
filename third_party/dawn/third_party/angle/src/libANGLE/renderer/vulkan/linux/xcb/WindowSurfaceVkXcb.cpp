//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkXcb.cpp:
//    Implements the class methods for WindowSurfaceVkXcb.
//

#include "libANGLE/renderer/vulkan/linux/xcb/WindowSurfaceVkXcb.h"

#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

WindowSurfaceVkXcb::WindowSurfaceVkXcb(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window,
                                       xcb_connection_t *conn)
    : WindowSurfaceVk(surfaceState, window), mXcbConnection(conn)
{}

angle::Result WindowSurfaceVkXcb::createSurfaceVk(vk::ErrorContext *context,
                                                  gl::Extents *extentsOut)
{
    VkXcbSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.flags      = 0;
    createInfo.connection = mXcbConnection;
    createInfo.window     = static_cast<xcb_window_t>(mNativeWindowType);
    ANGLE_VK_TRY(context, vkCreateXcbSurfaceKHR(context->getRenderer()->getInstance(), &createInfo,
                                                nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkXcb::getCurrentWindowSize(vk::ErrorContext *context,
                                                       gl::Extents *extentsOut)
{
    xcb_get_geometry_cookie_t cookie =
        xcb_get_geometry(mXcbConnection, static_cast<xcb_drawable_t>(mNativeWindowType));
    xcb_generic_error_t *error      = nullptr;
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(mXcbConnection, cookie, &error);
    if (error)
    {
        free(error);
        ANGLE_VK_CHECK(context, false, VK_ERROR_INITIALIZATION_FAILED);
    }
    ASSERT(reply);
    *extentsOut = gl::Extents(reply->width, reply->height, 1);
    free(reply);
    return angle::Result::Continue;
}

}  // namespace rx
