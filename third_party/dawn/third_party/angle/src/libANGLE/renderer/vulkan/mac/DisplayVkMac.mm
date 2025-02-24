//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkMac.mm:
//    Implements methods from DisplayVkMac
//

#include "libANGLE/renderer/vulkan/mac/DisplayVkMac.h"

#include <vulkan/vulkan.h>

#include "libANGLE/renderer/vulkan/mac/IOSurfaceSurfaceVkMac.h"
#include "libANGLE/renderer/vulkan/mac/WindowSurfaceVkMac.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#import <Cocoa/Cocoa.h>

namespace rx
{

DisplayVkMac::DisplayVkMac(const egl::DisplayState &state) : DisplayVk(state) {}

bool DisplayVkMac::isValidNativeWindow(EGLNativeWindowType window) const
{
    NSObject *layer = reinterpret_cast<NSObject *>(window);
    return [layer isKindOfClass:[CALayer class]];
}

SurfaceImpl *DisplayVkMac::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window)
{
    ASSERT(isValidNativeWindow(window));
    return new WindowSurfaceVkMac(state, window);
}

SurfaceImpl *DisplayVkMac::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                         EGLenum buftype,
                                                         EGLClientBuffer clientBuffer,
                                                         const egl::AttributeMap &attribs)
{
    ASSERT(buftype == EGL_IOSURFACE_ANGLE);

    return new IOSurfaceSurfaceVkMac(state, clientBuffer, attribs, mRenderer);
}

egl::ConfigSet DisplayVkMac::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_BGRA8_EXT, GL_BGRX8_ANGLEX};
    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

void DisplayVkMac::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/42261400
}

const char *DisplayVkMac::getWSIExtension() const
{
    return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
}

bool IsVulkanMacDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanMacDisplay(const egl::DisplayState &state)
{
    return new DisplayVkMac(state);
}

void DisplayVkMac::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->iosurfaceClientBuffer = true;

    DisplayVk::generateExtensions(outExtensions);
}

egl::Error DisplayVkMac::validateClientBuffer(const egl::Config *configuration,
                                              EGLenum buftype,
                                              EGLClientBuffer clientBuffer,
                                              const egl::AttributeMap &attribs) const
{
    ASSERT(buftype == EGL_IOSURFACE_ANGLE);

    if (!IOSurfaceSurfaceVkMac::ValidateAttributes(this, clientBuffer, attribs))
    {
        return egl::EglBadAttribute();
    }
    return egl::NoError();
}

}  // namespace rx
