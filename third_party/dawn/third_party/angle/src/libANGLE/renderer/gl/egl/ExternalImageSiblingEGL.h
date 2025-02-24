//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ExternalImageSiblingEGL.h: Defines the ExternalImageSiblingEGL interface to abstract all external
// image siblings in the EGL backend

#ifndef LIBANGLE_RENDERER_GL_EGL_EXTERNALIMAGESIBLINGEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_EXTERNALIMAGESIBLINGEGL_H_

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{

class ExternalImageSiblingEGL : public ExternalImageSiblingImpl
{
  public:
    ExternalImageSiblingEGL() {}
    ~ExternalImageSiblingEGL() override {}

    virtual EGLClientBuffer getBuffer() const = 0;
    virtual void getImageCreationAttributes(std::vector<EGLint> *outAttributes) const {}
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_EXTERNALIMAGESIBLINGEGL_H_
