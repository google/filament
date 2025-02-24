//
// Copyright The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DmaBufImageSiblingEGL.h: Defines the DmaBufImageSiblingEGL to wrap EGL images
// created from dma_buf objects

#ifndef LIBANGLE_RENDERER_GL_EGL_DMABUFIMAGESIBLINGEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_DMABUFIMAGESIBLINGEGL_H_

#include "libANGLE/renderer/gl/egl/ExternalImageSiblingEGL.h"

namespace rx
{

class DmaBufImageSiblingEGL : public ExternalImageSiblingEGL
{
  public:
    DmaBufImageSiblingEGL(const egl::AttributeMap &attribs);
    ~DmaBufImageSiblingEGL() override;

    egl::Error initialize(const egl::Display *display) override;

    // ExternalImageSiblingImpl interface
    gl::Format getFormat() const override;
    bool isRenderable(const gl::Context *context) const override;
    bool isTexturable(const gl::Context *context) const override;
    bool isYUV() const override;
    bool hasProtectedContent() const override;
    gl::Extents getSize() const override;
    size_t getSamples() const override;

    // ExternalImageSiblingEGL interface
    EGLClientBuffer getBuffer() const override;
    void getImageCreationAttributes(std::vector<EGLint> *outAttributes) const override;

  private:
    egl::AttributeMap mAttribs;
    gl::Extents mSize;
    gl::Format mFormat;
    bool mYUV;
    bool mHasProtectedContent;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_DMABUFIMAGESIBLINGEGL_H_
