//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkGbm.cpp:
//    Implements the class methods for DisplayVkGbm.
//

#include "libANGLE/renderer/vulkan/linux/gbm/DisplayVkGbm.h"

#include <gbm.h>

#include "common/linux/dma_buf_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkGbm::DisplayVkGbm(const egl::DisplayState &state)
    : DisplayVkLinux(state), mGbmDevice(nullptr)
{}

egl::Error DisplayVkGbm::initialize(egl::Display *display)
{
    mGbmDevice = reinterpret_cast<gbm_device *>(display->getNativeDisplayId());
    if (!mGbmDevice)
    {
        ERR() << "Failed to retrieve GBM device";
        return egl::EglNotInitialized();
    }

    return DisplayVk::initialize(display);
}

void DisplayVkGbm::terminate()
{
    mGbmDevice = nullptr;
    DisplayVk::terminate();
}

bool DisplayVkGbm::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (void *)window != nullptr;
}

SurfaceImpl *DisplayVkGbm::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window)
{
    return nullptr;
}

egl::ConfigSet DisplayVkGbm::generateConfigs()
{
    const std::array<GLenum, 1> kColorFormats = {GL_BGRA8_EXT};

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    if (getCaps().stencil8)
    {
        depthStencilFormats.push_back(GL_STENCIL_INDEX8);
    }

    egl::ConfigSet cfgSet =
        egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(),
                                depthStencilFormats.data(), depthStencilFormats.size(), this);

    return cfgSet;
}

void DisplayVkGbm::checkConfigSupport(egl::Config *config) {}

const char *DisplayVkGbm::getWSIExtension() const
{
    return nullptr;
}

angle::NativeWindowSystem DisplayVkGbm::getWindowSystem() const
{
    return angle::NativeWindowSystem::Gbm;
}

bool IsVulkanGbmDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanGbmDisplay(const egl::DisplayState &state)
{
    return new DisplayVkGbm(state);
}

}  // namespace rx
