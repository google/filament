//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_

#include "common/vulkan/vk_headers.h"
#include "libANGLE/Config.h"

namespace gl
{
struct Limitations;
struct Extensions;
class TextureCapsMap;
struct Caps;
struct TextureCaps;
struct InternalFormat;
}  // namespace gl

namespace rx
{
struct FeaturesVk;

class DisplayVk;

namespace egl_vk
{
constexpr GLenum kConfigDepthStencilFormats[] = {GL_NONE, GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT24,
                                                 GL_DEPTH_COMPONENT16};

// Permutes over all combinations of color format, depth stencil format and sample count and
// generates a basic config which is passed to DisplayVk::checkConfigSupport.
egl::ConfigSet GenerateConfigs(const GLenum *colorFormats,
                               size_t colorFormatsCount,
                               const GLenum *depthStencilFormats,
                               size_t depthStencilFormatCount,
                               DisplayVk *display);

template <size_t ColorFormatCount, size_t DepthStencilFormatCount>
egl::ConfigSet GenerateConfigs(const GLenum (&colorFormats)[ColorFormatCount],
                               const GLenum (&depthStencilFormats)[DepthStencilFormatCount],
                               DisplayVk *display)
{
    return GenerateConfigs(colorFormats, ColorFormatCount, depthStencilFormats,
                           DepthStencilFormatCount, display);
}

static ANGLE_INLINE EGLenum GetConfigCaveat(GLenum format)
{
    // Default EGL config sorting rule will result in rgb10a2 having higher precedence than rgb8
    // By marking `rgb10a2` as a slow config we switch the order. This ensures that we dont
    // return rgb10a2 at the top of the config list

    switch (format)
    {
        // For now we only mark rgb10a2 as a slow config
        case GL_RGB10_A2_EXT:
            return EGL_SLOW_CONFIG;
        default:
            return EGL_NONE;
    }
}

}  // namespace egl_vk

namespace vk
{
// Checks support for extensions required for GLES 3.2. If any of those extensions is missing, the
// context version should be capped to GLES 3.1 by default.
bool CanSupportGLES32(const gl::Extensions &nativeExtensions);

// Functions that determine support for a feature or extension, used both to advertise support for
// an extension, and to determine if a context version can be supported.
bool CanSupportTransformFeedbackExtension(
    const VkPhysicalDeviceTransformFeedbackFeaturesEXT &xfbFeatures);
bool CanSupportTransformFeedbackEmulation(const VkPhysicalDeviceFeatures &features);
}  // namespace vk

}  // namespace rx

#endif
