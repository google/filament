//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkGGP.cpp:
//    Implements the class methods for WindowSurfaceVkGGP.
//

#include "libANGLE/renderer/vulkan/ggp/WindowSurfaceVkGGP.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace
{
constexpr EGLAttrib kDefaultStreamDescriptor = static_cast<EGLAttrib>(kGgpPrimaryStreamDescriptor);
}  // namespace

WindowSurfaceVkGGP::WindowSurfaceVkGGP(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkGGP::createSurfaceVk(vk::ErrorContext *context,
                                                  gl::Extents *extentsOut)
{
    vk::Renderer *renderer = context->getRenderer();

    // Get the stream descriptor if specified. Default is kGgpPrimaryStreamDescriptor.
    EGLAttrib streamDescriptor =
        mState.attributes.get(EGL_GGP_STREAM_DESCRIPTOR_ANGLE, kDefaultStreamDescriptor);

    VkStreamDescriptorSurfaceCreateInfoGGP createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
    createInfo.streamDescriptor = static_cast<GgpStreamDescriptor>(streamDescriptor);

    ANGLE_VK_TRY(context, vkCreateStreamDescriptorSurfaceGGP(renderer->getInstance(), &createInfo,
                                                             nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkGGP::getCurrentWindowSize(vk::ErrorContext *context,
                                                       gl::Extents *extentsOut)
{
    vk::Renderer *renderer                 = context->getRenderer();
    const VkPhysicalDevice &physicalDevice = renderer->getPhysicalDevice();

    ANGLE_VK_TRY(context, vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface,
                                                                    &mSurfaceCaps));

    *extentsOut =
        gl::Extents(mSurfaceCaps.currentExtent.width, mSurfaceCaps.currentExtent.height, 1);
    return angle::Result::Continue;
}

egl::Error WindowSurfaceVkGGP::swapWithFrameToken(const gl::Context *context,
                                                  EGLFrameTokenANGLE frameToken)
{
    VkPresentFrameTokenGGP frameTokenData = {};
    frameTokenData.sType                  = VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP;
    frameTokenData.frameToken             = static_cast<GgpFrameToken>(frameToken);

    angle::Result result = swapImpl(context, nullptr, 0, &frameTokenData);
    return angle::ToEGL(result, EGL_BAD_SURFACE);
}
}  // namespace rx
