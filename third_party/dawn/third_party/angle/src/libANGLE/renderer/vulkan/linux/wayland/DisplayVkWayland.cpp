//
// Copyright 2021-2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWayland.cpp:
//    Implements the class methods for DisplayVkWayland.
//

#include "libANGLE/renderer/vulkan/linux/wayland/DisplayVkWayland.h"

#include <wayland-client.h>

#include "common/angleutils.h"
#include "common/linux/dma_buf_utils.h"
#include "common/system_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/linux/wayland/WindowSurfaceVkWayland.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

DisplayVkWayland::DisplayVkWayland(const egl::DisplayState &state)
    : DisplayVkLinux(state), mOwnDisplay(false), mWaylandDisplay(nullptr)
{}

egl::Error DisplayVkWayland::initialize(egl::Display *display)
{
    EGLNativeDisplayType nativeDisplay = display->getNativeDisplayId();
    if (nativeDisplay == EGL_DEFAULT_DISPLAY)
    {
        mOwnDisplay     = true;
        mWaylandDisplay = wl_display_connect(nullptr);
    }
    else
    {
        mWaylandDisplay = reinterpret_cast<wl_display *>(nativeDisplay);
    }

    if (!mWaylandDisplay)
    {
        ERR() << "Failed to retrieve wayland display";
        return egl::EglNotInitialized();
    }

    egl::Error ret = DisplayVk::initialize(display);
    if (ret.isError())
    {
        return ret;
    }

    return ret;
}

void DisplayVkWayland::terminate()
{
    if (mOwnDisplay)
    {
        wl_display_disconnect(mWaylandDisplay);
        mOwnDisplay = false;
    }
    mWaylandDisplay = nullptr;
    DisplayVk::terminate();
}

bool DisplayVkWayland::isValidNativeWindow(EGLNativeWindowType window) const
{
    // Wayland display Errors are fatal.
    // If this function returns non-zero, the display is not valid anymore.
    int error = wl_display_get_error(mWaylandDisplay);
    if (error)
    {
        WARN() << "Wayland window is not valid: " << error << " " << strerror(error);
    }
    return error == 0;
}

SurfaceImpl *DisplayVkWayland::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window)
{
    return new WindowSurfaceVkWayland(state, window, mWaylandDisplay);
}

egl::ConfigSet DisplayVkWayland::generateConfigs()
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

void DisplayVkWayland::checkConfigSupport(egl::Config *config)
{
    // In wayland there is no native visual ID or type
}

const char *DisplayVkWayland::getWSIExtension() const
{
    return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
}

angle::NativeWindowSystem DisplayVkWayland::getWindowSystem() const
{
    return angle::NativeWindowSystem::Wayland;
}

bool IsVulkanWaylandDisplayAvailable()
{
    wl_display *display = wl_display_connect(nullptr);
    if (!display)
    {
        return false;
    }
    wl_display_disconnect(display);
    return true;
}

DisplayImpl *CreateVulkanWaylandDisplay(const egl::DisplayState &state)
{
    return new DisplayVkWayland(state);
}

}  // namespace rx
