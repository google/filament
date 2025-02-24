//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer11.h: Defines the D3D11 VertexBuffer implementation.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_

#include <stdint.h>

#include "libANGLE/renderer/d3d/VertexBuffer.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace rx
{
class Renderer11;

class VertexBuffer11 : public VertexBuffer
{
  public:
    explicit VertexBuffer11(Renderer11 *const renderer);

    angle::Result initialize(const gl::Context *context,
                             unsigned int size,
                             bool dynamicUsage) override;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using this
    // function.
    angle::Result storeVertexAttributes(const gl::Context *context,
                                        const gl::VertexAttribute &attrib,
                                        const gl::VertexBinding &binding,
                                        gl::VertexAttribType currentValueType,
                                        GLint start,
                                        size_t count,
                                        GLsizei instances,
                                        unsigned int offset,
                                        const uint8_t *sourceData) override;

    unsigned int getBufferSize() const override;
    angle::Result setBufferSize(const gl::Context *context, unsigned int size) override;
    angle::Result discard(const gl::Context *context) override;

    void hintUnmapResource() override;

    const d3d11::Buffer &getBuffer() const;

  private:
    ~VertexBuffer11() override;
    angle::Result mapResource(const gl::Context *context);

    Renderer11 *const mRenderer;

    d3d11::Buffer mBuffer;
    unsigned int mBufferSize;
    bool mDynamicUsage;

    uint8_t *mMappedResourceData;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_VERTEXBUFFER11_H_
