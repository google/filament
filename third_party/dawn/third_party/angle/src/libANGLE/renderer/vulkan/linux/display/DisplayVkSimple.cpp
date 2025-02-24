//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkSimple.cpp:
//    Implements the class methods for DisplayVkSimple.
//

#include "DisplayVkSimple.h"
#include "WindowSurfaceVkSimple.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

DisplayVkSimple::DisplayVkSimple(const egl::DisplayState &state) : DisplayVkLinux(state) {}

void DisplayVkSimple::terminate()
{
    DisplayVk::terminate();
}

bool DisplayVkSimple::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

SurfaceImpl *DisplayVkSimple::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                    EGLNativeWindowType window)
{
    return new WindowSurfaceVkSimple(state, window);
}

egl::ConfigSet DisplayVkSimple::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_RGBA8, GL_BGRA8_EXT, GL_RGB565, GL_RGB8};

    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

// TODO: anglebug.com/40096731
// Detemine if check is needed.
void DisplayVkSimple::checkConfigSupport(egl::Config *config) {}

const char *DisplayVkSimple::getWSIExtension() const
{
    return VK_KHR_DISPLAY_EXTENSION_NAME;
}

bool IsVulkanSimpleDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanSimpleDisplay(const egl::DisplayState &state)
{
    return new DisplayVkSimple(state);
}

}  // namespace rx
