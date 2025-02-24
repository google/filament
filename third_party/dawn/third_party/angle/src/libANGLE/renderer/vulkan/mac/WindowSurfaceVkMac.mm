//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkMac.mm:
//    Implements methods from WindowSurfaceVkMac.
//

#include "libANGLE/renderer/vulkan/mac/WindowSurfaceVkMac.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

WindowSurfaceVkMac::WindowSurfaceVkMac(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window), mMetalLayer(nullptr)
{}

WindowSurfaceVkMac::~WindowSurfaceVkMac()
{
    [mMetalDevice release];
    [mMetalLayer release];
}

angle::Result WindowSurfaceVkMac::createSurfaceVk(vk::ErrorContext *context,
                                                  gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    mMetalDevice = MTLCreateSystemDefaultDevice();

    CALayer *layer = reinterpret_cast<CALayer *>(mNativeWindowType);

    mMetalLayer        = [[CAMetalLayer alloc] init];
    mMetalLayer.frame  = CGRectMake(0, 0, layer.frame.size.width, layer.frame.size.height);
    mMetalLayer.device = mMetalDevice;
    mMetalLayer.drawableSize =
        CGSizeMake(mMetalLayer.bounds.size.width * mMetalLayer.contentsScale,
                   mMetalLayer.bounds.size.height * mMetalLayer.contentsScale);
    mMetalLayer.framebufferOnly  = NO;
    mMetalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    mMetalLayer.contentsScale    = layer.contentsScale;

    [layer addSublayer:mMetalLayer];

    VkMetalSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType                       = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.flags                       = 0;
    createInfo.pNext                       = nullptr;
    createInfo.pLayer                      = mMetalLayer;
    ANGLE_VK_TRY(context, vkCreateMetalSurfaceEXT(context->getRenderer()->getInstance(),
                                                  &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkMac::getCurrentWindowSize(vk::ErrorContext *context,
                                                       gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    ANGLE_VK_CHECK(context, (mMetalLayer != nullptr), VK_ERROR_INITIALIZATION_FAILED);

    mMetalLayer.drawableSize =
        CGSizeMake(mMetalLayer.bounds.size.width * mMetalLayer.contentsScale,
                   mMetalLayer.bounds.size.height * mMetalLayer.contentsScale);
    *extentsOut = gl::Extents(static_cast<int>(mMetalLayer.drawableSize.width),
                              static_cast<int>(mMetalLayer.drawableSize.height), 1);

    return angle::Result::Continue;
}

}  // namespace rx
