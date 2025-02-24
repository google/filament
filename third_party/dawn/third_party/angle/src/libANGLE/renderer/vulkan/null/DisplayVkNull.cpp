//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkNull.cpp:
//    Implements the class methods for DisplayVkNull.
//

#include "DisplayVkNull.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

DisplayVkNull::DisplayVkNull(const egl::DisplayState &state) : DisplayVk(state) {}

bool DisplayVkNull::isValidNativeWindow(EGLNativeWindowType window) const
{
    return false;
}

SurfaceImpl *DisplayVkNull::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                  EGLNativeWindowType window)
{
    return new OffscreenSurfaceVk(state, mRenderer);
}

const char *DisplayVkNull::getWSIExtension() const
{
    return nullptr;
}

egl::ConfigSet DisplayVkNull::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_RGBA8, GL_BGRA8_EXT, GL_RGB565, GL_RGB8};

    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

void DisplayVkNull::checkConfigSupport(egl::Config *config) {}

bool IsVulkanNullDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanNullDisplay(const egl::DisplayState &state)
{
    return new DisplayVkNull(state);
}

}  // namespace rx
