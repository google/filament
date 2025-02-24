//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkGGP.cpp:
//    Implements the class methods for DisplayVkGGP.
//

#include "libANGLE/renderer/vulkan/ggp/DisplayVkGGP.h"

#include "libANGLE/renderer/vulkan/ggp/WindowSurfaceVkGGP.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{
DisplayVkGGP::DisplayVkGGP(const egl::DisplayState &state) : DisplayVk(state) {}

bool DisplayVkGGP::isValidNativeWindow(EGLNativeWindowType window) const
{
    // GGP doesn't use window handles.
    return true;
}

SurfaceImpl *DisplayVkGGP::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window)
{
    return new WindowSurfaceVkGGP(state, window);
}

egl::ConfigSet DisplayVkGGP::generateConfigs()
{
    // Not entirely sure what backbuffer formats GGP supports.
    constexpr GLenum kColorFormats[] = {GL_BGRA8_EXT, GL_BGRX8_ANGLEX};
    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

void DisplayVkGGP::checkConfigSupport(egl::Config *config) {}

const char *DisplayVkGGP::getWSIExtension() const
{
    return VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME;
}

bool IsVulkanGGPDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanGGPDisplay(const egl::DisplayState &state)
{
    return new DisplayVkGGP(state);
}
}  // namespace rx
