//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DmaBufImageSiblingVkLinux.h: Defines the DmaBufImageSiblingVkLinux to wrap
// EGL images created from Linux dma-buf objects

#ifndef LIBANGLE_RENDERER_VULKAN_LINUX_DMABUFIMAGESIBLINGVKLINUX_H_
#define LIBANGLE_RENDERER_VULKAN_LINUX_DMABUFIMAGESIBLINGVKLINUX_H_

#include "libANGLE/renderer/vulkan/ImageVk.h"

namespace rx
{

enum MutableFormat
{
    Allowed,
    NotAllowed
};

enum InitResult
{
    Success,
    Failed
};

class DmaBufImageSiblingVkLinux : public ExternalImageSiblingVk
{
  public:
    DmaBufImageSiblingVkLinux(const egl::AttributeMap &attribs);
    ~DmaBufImageSiblingVkLinux() override;

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
    angle::Result initWithFormat(DisplayVk *displayVk,
                                 const angle::Format &format,
                                 VkFormat vulkanFormat,
                                 MutableFormat mutableFormat,
                                 InitResult *outResult);

    angle::Result initImpl(DisplayVk *displayVk);

    egl::AttributeMap mAttribs;
    gl::Extents mSize;
    gl::Format mFormat;
    std::vector<VkFormat> mVkFormats;

    bool mRenderable;
    bool mTextureable;
    bool mYUV;
    bool mHasProtectedContent;
    size_t mSamples;

    vk::ImageHelper *mImage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_LINUX_DMABUFIMAGESIBLINGVKLINUX_H_
