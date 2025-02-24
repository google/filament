//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkFuchsia.cpp:
//    Implements methods from DisplayVkFuchsia
//

#include "libANGLE/renderer/vulkan/fuchsia/DisplayVkFuchsia.h"

#include "libANGLE/renderer/vulkan/fuchsia/WindowSurfaceVkFuchsia.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkFuchsia::DisplayVkFuchsia(const egl::DisplayState &state) : DisplayVk(state) {}

bool DisplayVkFuchsia::isValidNativeWindow(EGLNativeWindowType window) const
{
    return WindowSurfaceVkFuchsia::isValidNativeWindow(window);
}

SurfaceImpl *DisplayVkFuchsia::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window)
{
    ASSERT(isValidNativeWindow(window));
    return new WindowSurfaceVkFuchsia(state, window);
}

egl::ConfigSet DisplayVkFuchsia::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_BGRA8_EXT, GL_BGRX8_ANGLEX};
    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

void DisplayVkFuchsia::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/42261400
}

const char *DisplayVkFuchsia::getWSIExtension() const
{
    return VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME;
}

const char *DisplayVkFuchsia::getWSILayer() const
{
    return "VK_LAYER_FUCHSIA_imagepipe_swapchain";
}

bool IsVulkanFuchsiaDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanFuchsiaDisplay(const egl::DisplayState &state)
{
    return new DisplayVkFuchsia(state);
}
}  // namespace rx
