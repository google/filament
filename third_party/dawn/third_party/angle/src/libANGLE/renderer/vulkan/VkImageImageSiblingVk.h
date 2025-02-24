//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VkImageImageSiblingVk.h: Defines the VkImageImageSiblingVk to wrap
// EGL images created from VkImage.

#ifndef LIBANGLE_RENDERER_VULKAN_VKIMAGEIMAGESIBLINGVK_H_
#define LIBANGLE_RENDERER_VULKAN_VKIMAGEIMAGESIBLINGVK_H_

#include "libANGLE/renderer/vulkan/ImageVk.h"

namespace rx
{

class VkImageImageSiblingVk final : public ExternalImageSiblingVk
{
  public:
    VkImageImageSiblingVk(EGLClientBuffer buffer, const egl::AttributeMap &attribs);
    ~VkImageImageSiblingVk() override;

    egl::Error initialize(const egl::Display *display) override;
    void onDestroy(const egl::Display *display) override;

    // ExternalImageSiblingImpl interface
    gl::Format getFormat() const override;
    bool isRenderable(const gl::Context *context) const override;
    bool isTexturable(const gl::Context *context) const override;
    bool isYUV() const override;
    bool hasProtectedContent() const override;
    gl::Extents getSize() const override;
    size_t getSamples() const override;

    // ExternalImageSiblingVk interface
    vk::ImageHelper *getImage() const override;

    void release(vk::Renderer *renderer) override;

  private:
    angle::Result initImpl(DisplayVk *displayVk);

    vk::Image mVkImage;
    VkImageCreateInfo mVkImageInfo;
    GLenum mInternalFormat = GL_NONE;
    gl::Format mFormat{GL_NONE};
    vk::ImageHelper *mImage = nullptr;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VKIMAGEIMAGESIBLINGVK_H_
