//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Indexffer9.h: Defines the D3D9 IndexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_

#include "libANGLE/renderer/d3d/IndexBuffer.h"

namespace rx
{
class Renderer9;

class IndexBuffer9 : public IndexBuffer
{
  public:
    explicit IndexBuffer9(Renderer9 *const renderer);
    ~IndexBuffer9() override;

    angle::Result initialize(const gl::Context *context,
                             unsigned int bufferSize,
                             gl::DrawElementsType indexType,
                             bool dynamic) override;

    angle::Result mapBuffer(const gl::Context *context,
                            unsigned int offset,
                            unsigned int size,
                            void **outMappedMemory) override;
    angle::Result unmapBuffer(const gl::Context *context) override;

    gl::DrawElementsType getIndexType() const override;
    unsigned int getBufferSize() const override;
    angle::Result setSize(const gl::Context *context,
                          unsigned int bufferSize,
                          gl::DrawElementsType indexType) override;

    angle::Result discard(const gl::Context *context) override;

    D3DFORMAT getIndexFormat() const;
    IDirect3DIndexBuffer9 *getBuffer() const;

  private:
    Renderer9 *const mRenderer;

    IDirect3DIndexBuffer9 *mIndexBuffer;
    unsigned int mBufferSize;
    gl::DrawElementsType mIndexType;
    bool mDynamic;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_INDEXBUFFER9_H_
