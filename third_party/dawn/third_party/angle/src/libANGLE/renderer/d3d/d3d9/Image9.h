//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image9.h: Defines the rx::Image9 class, which acts as the interface to
// the actual underlying surfaces of a Texture.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_IMAGE9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_IMAGE9_H_

#include "common/debug.h"
#include "libANGLE/renderer/d3d/ImageD3D.h"

namespace gl
{
class Framebuffer;
}

namespace rx
{
class Context9;
class Renderer9;

class Image9 : public ImageD3D
{
  public:
    Image9(Renderer9 *renderer);
    ~Image9() override;

    static angle::Result GenerateMipmap(Context9 *context9, Image9 *dest, Image9 *source);
    static angle::Result GenerateMip(Context9 *context9,
                                     IDirect3DSurface9 *destSurface,
                                     IDirect3DSurface9 *sourceSurface);
    static angle::Result CopyLockableSurfaces(Context9 *context9,
                                              IDirect3DSurface9 *dest,
                                              IDirect3DSurface9 *source);
    static angle::Result CopyImage(const gl::Context *context,
                                   Image9 *dest,
                                   Image9 *source,
                                   const gl::Rectangle &sourceRect,
                                   const gl::Offset &destOffset,
                                   bool unpackFlipY,
                                   bool unpackPremultiplyAlpha,
                                   bool unpackUnmultiplyAlpha);

    bool redefine(gl::TextureType type,
                  GLenum internalformat,
                  const gl::Extents &size,
                  bool forceRelease) override;

    D3DFORMAT getD3DFormat() const;

    bool isDirty() const override;

    angle::Result setManagedSurface2D(const gl::Context *context,
                                      TextureStorage *storage,
                                      int level) override;
    angle::Result setManagedSurfaceCube(const gl::Context *context,
                                        TextureStorage *storage,
                                        int face,
                                        int level) override;
    angle::Result copyToStorage(const gl::Context *context,
                                TextureStorage *storage,
                                const gl::ImageIndex &index,
                                const gl::Box &region) override;

    angle::Result loadData(const gl::Context *context,
                           const gl::Box &area,
                           const gl::PixelUnpackState &unpack,
                           GLenum type,
                           const void *input,
                           bool applySkipImages) override;
    angle::Result loadCompressedData(const gl::Context *context,
                                     const gl::Box &area,
                                     const void *input) override;

    angle::Result copyFromTexStorage(const gl::Context *context,
                                     const gl::ImageIndex &imageIndex,
                                     TextureStorage *source) override;
    angle::Result copyFromFramebuffer(const gl::Context *context,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      const gl::Framebuffer *source) override;

  private:
    angle::Result getSurface(Context9 *context9, IDirect3DSurface9 **outSurface);

    angle::Result createSurface(Context9 *context9);
    angle::Result setManagedSurface(Context9 *context9, IDirect3DSurface9 *surface);
    angle::Result copyToSurface(Context9 *context9, IDirect3DSurface9 *dest, const gl::Box &area);

    angle::Result lock(Context9 *context9, D3DLOCKED_RECT *lockedRect, const RECT &rect);
    void unlock();

    angle::Result copyFromRTInternal(Context9 *context9,
                                     const gl::Offset &destOffset,
                                     const gl::Rectangle &sourceArea,
                                     RenderTargetD3D *source);

    Renderer9 *mRenderer;

    D3DPOOL mD3DPool;  // can only be D3DPOOL_SYSTEMMEM or D3DPOOL_MANAGED since it needs to be
                       // lockable.
    D3DFORMAT mD3DFormat;

    IDirect3DSurface9 *mSurface;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_IMAGE9_H_
