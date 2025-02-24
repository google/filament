//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.h: Implements the rx::Image class, an abstract base class for the
// renderer-specific classes which will define the interface to the underlying
// surfaces or resources.

#include "libANGLE/renderer/d3d/ImageD3D.h"

namespace rx
{

ImageD3D::ImageD3D()
    : mWidth(0),
      mHeight(0),
      mDepth(0),
      mInternalFormat(GL_NONE),
      mRenderable(false),
      mType(gl::TextureType::InvalidEnum),
      mDirty(false)
{}

angle::Result ImageD3D::setManagedSurface2D(const gl::Context *context,
                                            TextureStorage *storage,
                                            int level)
{
    return angle::Result::Continue;
}

angle::Result ImageD3D::setManagedSurfaceCube(const gl::Context *context,
                                              TextureStorage *storage,
                                              int face,
                                              int level)
{
    return angle::Result::Continue;
}

angle::Result ImageD3D::setManagedSurface3D(const gl::Context *context,
                                            TextureStorage *storage,
                                            int level)
{
    return angle::Result::Continue;
}

angle::Result ImageD3D::setManagedSurface2DArray(const gl::Context *context,
                                                 TextureStorage *storage,
                                                 int layer,
                                                 int level)
{
    return angle::Result::Continue;
}

}  // namespace rx
