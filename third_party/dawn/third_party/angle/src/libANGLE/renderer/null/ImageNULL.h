//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageNULL.h:
//    Defines the class interface for ImageNULL, implementing ImageImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_IMAGENULL_H_
#define LIBANGLE_RENDERER_NULL_IMAGENULL_H_

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{

class ImageNULL : public ImageImpl
{
  public:
    ImageNULL(const egl::ImageState &state);
    ~ImageNULL() override;
    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_IMAGENULL_H_
