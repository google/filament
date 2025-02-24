//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkXcb.cpp:
//    Implements the class methods for DisplayVkXcb.
//

#include "libANGLE/renderer/vulkan/linux/xcb/DisplayVkXcb.h"

#include <xcb/xcb.h>

#include "common/system_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/linux/xcb/WindowSurfaceVkXcb.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

namespace
{
EGLint GetXcbVisualType(xcb_screen_t *screen)
{
    // Visual type is the class member of xcb_visualtype_t whose id matches the root visual.
    for (xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);
         depth_iter.rem; xcb_depth_next(&depth_iter))
    {
        for (xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
             visual_iter.rem; xcb_visualtype_next(&visual_iter))
        {
            if (screen->root_visual == visual_iter.data->visual_id)
            {
                return visual_iter.data->_class;
            }
        }
    }

    return EGL_NONE;
}
}  // namespace

DisplayVkXcb::DisplayVkXcb(const egl::DisplayState &state)
    : DisplayVkLinux(state), mXcbConnection(nullptr), mHasXDisplay(false)
{}

egl::Error DisplayVkXcb::initialize(egl::Display *display)
{
    mHasXDisplay = !angle::GetEnvironmentVar("DISPLAY").empty();
    if (mHasXDisplay)
    {
        mXcbConnection = xcb_connect(nullptr, nullptr);
        ASSERT(mXcbConnection != nullptr);
        int xcb_connection_error = xcb_connection_has_error(mXcbConnection);
        if (xcb_connection_error)
        {
            ERR() << "xcb_connect() failed, error " << xcb_connection_error;
            xcb_disconnect(mXcbConnection);
            mXcbConnection = nullptr;
            return egl::EglNotInitialized();
        }
    }
    return DisplayVk::initialize(display);
}

void DisplayVkXcb::terminate()
{
    if (mHasXDisplay)
    {
        ASSERT(mXcbConnection != nullptr);
        xcb_disconnect(mXcbConnection);
        mXcbConnection = nullptr;
    }
    DisplayVk::terminate();
}

bool DisplayVkXcb::isValidNativeWindow(EGLNativeWindowType window) const
{
    ASSERT(mHasXDisplay);
    // There doesn't appear to be an xcb function explicitly for checking the validity of a
    // window ID, but xcb_query_tree_reply will return nullptr if the window doesn't exist.
    xcb_query_tree_cookie_t cookie =
        xcb_query_tree(mXcbConnection, static_cast<xcb_window_t>(window));
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(mXcbConnection, cookie, nullptr);
    if (reply)
    {
        free(reply);
        return true;
    }
    return false;
}

SurfaceImpl *DisplayVkXcb::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window)
{
    ASSERT(mHasXDisplay);
    return new WindowSurfaceVkXcb(state, window, mXcbConnection);
}

egl::ConfigSet DisplayVkXcb::generateConfigs()
{
    const std::array<GLenum, 1> kColorFormats = {GL_BGRA8_EXT};

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    if (getCaps().stencil8)
    {
        depthStencilFormats.push_back(GL_STENCIL_INDEX8);
    }
    return egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(),
                                   depthStencilFormats.data(), depthStencilFormats.size(), this);
}

void DisplayVkXcb::checkConfigSupport(egl::Config *config)
{
    // If no window system, cannot support windows.
    if (!mHasXDisplay)
    {
        // No window support if no X11.
        config->surfaceType &= ~EGL_WINDOW_BIT;
        return;
    }

    // TODO(geofflang): Test for native support and modify the config accordingly.
    // http://anglebug.com/42261400

    // Find the screen the window was created on:
    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(xcb_get_setup(mXcbConnection));
    ASSERT(screenIterator.rem);

    xcb_screen_t *screen = screenIterator.data;
    ASSERT(screen);

    // Visual id is root_visual of the screen
    config->nativeVisualID   = screen->root_visual;
    config->nativeVisualType = GetXcbVisualType(screen);
}

const char *DisplayVkXcb::getWSIExtension() const
{
    return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
}

bool IsVulkanXcbDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanXcbDisplay(const egl::DisplayState &state)
{
    return new DisplayVkXcb(state);
}

angle::Result DisplayVkXcb::waitNativeImpl()
{
    // The following is the implementation of xcb_aux_sync (equivalent of XSync).  xcb_aux_sync is
    // not called directly because it's in the libxcb-util package, which Chrome does not want to
    // depend on (due to different package versions in popular distros).
    free(xcb_get_input_focus_reply(mXcbConnection, xcb_get_input_focus(mXcbConnection), nullptr));
    return angle::Result::Continue;
}
}  // namespace rx
