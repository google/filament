//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageWgpu.h:
//    Defines the class interface for ImageWgpu, implementing ImageImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_IMAGEWGPU_H_
#define LIBANGLE_RENDERER_WGPU_IMAGEWGPU_H_

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{

class ImageWgpu : public ImageImpl
{
  public:
    ImageWgpu(const egl::ImageState &state);
    ~ImageWgpu() override;
    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_IMAGEWGPU_H_
