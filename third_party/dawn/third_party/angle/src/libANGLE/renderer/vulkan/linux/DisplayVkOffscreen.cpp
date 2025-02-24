//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkOffscreen.cpp:
//    Implements the class methods for DisplayVkOffscreen.
//

#include "DisplayVkOffscreen.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkOffscreen::DisplayVkOffscreen(const egl::DisplayState &state) : DisplayVkLinux(state) {}

bool DisplayVkOffscreen::isValidNativeWindow(EGLNativeWindowType window) const
{
    return false;
}

SurfaceImpl *DisplayVkOffscreen::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                       EGLNativeWindowType window)
{
    UNREACHABLE();
    return nullptr;
}

egl::ConfigSet DisplayVkOffscreen::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_RGBA8, GL_BGRA8_EXT, GL_RGB565, GL_RGB8};

    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

void DisplayVkOffscreen::checkConfigSupport(egl::Config *config) {}

const char *DisplayVkOffscreen::getWSIExtension() const
{
    return nullptr;
}

bool IsVulkanOffscreenDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanOffscreenDisplay(const egl::DisplayState &state)
{
    return new DisplayVkOffscreen(state);
}

}  // namespace rx
