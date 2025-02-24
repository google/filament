//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer11.h: Defines the D3D11 IndexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_

#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace rx
{
class Renderer11;

class IndexBuffer11 : public IndexBuffer
{
  public:
    explicit IndexBuffer11(Renderer11 *const renderer);
    ~IndexBuffer11() override;

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

    DXGI_FORMAT getIndexFormat() const;
    const d3d11::Buffer &getBuffer() const;

  private:
    Renderer11 *const mRenderer;

    d3d11::Buffer mBuffer;
    unsigned int mBufferSize;
    gl::DrawElementsType mIndexType;
    bool mDynamicUsage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_INDEXBUFFER11_H_
