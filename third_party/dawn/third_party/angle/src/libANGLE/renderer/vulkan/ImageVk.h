//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageVk.h:
//    Defines the class interface for ImageVk, implementing ImageImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_
#define LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_

#include "libANGLE/renderer/ImageImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ExternalImageSiblingVk : public ExternalImageSiblingImpl
{
  public:
    ExternalImageSiblingVk() {}
    ~ExternalImageSiblingVk() override {}

    virtual vk::ImageHelper *getImage() const = 0;

    virtual void release(vk::Renderer *renderer) = 0;
};

class ImageVk : public ImageImpl
{
  public:
    ImageVk(const egl::ImageState &state, const gl::Context *context);
    ~ImageVk() override;
    void onDestroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;

    egl::Error exportVkImage(void *vkImage, void *vkImageCreateInfo) override;

    bool isFixedRatedCompression(const gl::Context *context) override;

    vk::ImageHelper *getImage() const { return mImage; }
    gl::TextureType getImageTextureType() const;
    gl::LevelIndex getImageLevel() const;
    uint32_t getImageLayer() const;

    UniqueSerial generateSiblingSerial() { return mImageSiblingSerialFactory.generate(); }

  private:
    bool mOwnsImage;
    vk::ImageHelper *mImage;
    UniqueSerialFactory mImageSiblingSerialFactory;

    const gl::Context *mContext;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_
