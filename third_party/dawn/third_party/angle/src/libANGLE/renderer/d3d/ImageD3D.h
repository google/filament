//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageD3D.h: Defines the rx::ImageD3D class, an abstract base class for the
// renderer-specific classes which will define the interface to the underlying
// surfaces or resources.

#ifndef LIBANGLE_RENDERER_D3D_IMAGED3D_H_
#define LIBANGLE_RENDERER_D3D_IMAGED3D_H_

#include "common/debug.h"

#include "common/PackedEnums.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class Context;
class Framebuffer;
class ImageIndex;
struct PixelUnpackState;
}  // namespace gl

namespace rx
{
class TextureStorage;
class RendererD3D;
class RenderTargetD3D;

class ImageD3D : angle::NonCopyable
{
  public:
    ImageD3D();
    virtual ~ImageD3D() {}

    GLsizei getWidth() const { return mWidth; }
    GLsizei getHeight() const { return mHeight; }
    GLsizei getDepth() const { return mDepth; }
    GLenum getInternalFormat() const { return mInternalFormat; }
    gl::TextureType getType() const { return mType; }
    bool isRenderableFormat() const { return mRenderable; }

    void markDirty() { mDirty = true; }
    void markClean() { mDirty = false; }
    virtual bool isDirty() const = 0;

    virtual bool redefine(gl::TextureType type,
                          GLenum internalformat,
                          const gl::Extents &size,
                          bool forceRelease) = 0;

    virtual angle::Result loadData(const gl::Context *context,
                                   const gl::Box &area,
                                   const gl::PixelUnpackState &unpack,
                                   GLenum type,
                                   const void *input,
                                   bool applySkipImages)        = 0;
    virtual angle::Result loadCompressedData(const gl::Context *context,
                                             const gl::Box &area,
                                             const void *input) = 0;

    virtual angle::Result setManagedSurface2D(const gl::Context *context,
                                              TextureStorage *storage,
                                              int level);
    virtual angle::Result setManagedSurfaceCube(const gl::Context *context,
                                                TextureStorage *storage,
                                                int face,
                                                int level);
    virtual angle::Result setManagedSurface3D(const gl::Context *context,
                                              TextureStorage *storage,
                                              int level);
    virtual angle::Result setManagedSurface2DArray(const gl::Context *context,
                                                   TextureStorage *storage,
                                                   int layer,
                                                   int level);
    virtual angle::Result copyToStorage(const gl::Context *context,
                                        TextureStorage *storage,
                                        const gl::ImageIndex &index,
                                        const gl::Box &region) = 0;

    virtual angle::Result copyFromTexStorage(const gl::Context *context,
                                             const gl::ImageIndex &imageIndex,
                                             TextureStorage *source)         = 0;
    virtual angle::Result copyFromFramebuffer(const gl::Context *context,
                                              const gl::Offset &destOffset,
                                              const gl::Rectangle &sourceArea,
                                              const gl::Framebuffer *source) = 0;

  protected:
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    bool mRenderable;
    gl::TextureType mType;

    bool mDirty;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_IMAGED3D_H_
