//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkLinux.h:
//    Defines the class interface for DisplayVkLinux, which is the base of DisplayVkSimple,
//    DisplayVkHeadless, DisplayVkXcb and DisplayVkWayland.  This base class implements the
//    common functionality of handling Linux dma-bufs.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKLINUX_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKLINUX_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{
class DisplayVkLinux : public DisplayVk
{
  public:
    DisplayVkLinux(const egl::DisplayState &state);

    DeviceImpl *createDevice() override;

    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;
    std::vector<VkDrmFormatModifierPropertiesEXT> GetDrmModifiers(const DisplayVk *displayVk,
                                                                  VkFormat vkFormat);
    bool SupportsDrmModifiers(VkPhysicalDevice device, VkFormat vkFormat);
    std::vector<VkFormat> GetVkFormatsWithDrmModifiers(const vk::Renderer *renderer);
    std::vector<EGLint> GetDrmFormats(const vk::Renderer *renderer);
    bool supportsDmaBufFormat(EGLint format) const override;
    egl::Error queryDmaBufFormats(EGLint maxFormats, EGLint *formats, EGLint *numFormats) override;
    egl::Error queryDmaBufModifiers(EGLint format,
                                    EGLint maxModifiers,
                                    EGLuint64KHR *modifiers,
                                    EGLBoolean *externalOnly,
                                    EGLint *numModifiers) override;

  private:
    // Supported DRM formats
    std::vector<EGLint> mDrmFormats;

    bool mDrmFormatsInitialized;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKLINUX_H_
