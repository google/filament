//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer9.h: Defines the rx::Buffer9 class which implements rx::BufferImpl via rx::BufferD3D.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_

#include "common/MemoryBuffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"

namespace rx
{
class Renderer9;

class Buffer9 : public BufferD3D
{
  public:
    Buffer9(const gl::BufferState &state, Renderer9 *renderer);
    ~Buffer9() override;

    // BufferD3D implementation
    size_t getSize() const override;
    bool supportsDirectBinding() const override;
    angle::Result getData(const gl::Context *context, const uint8_t **outData) override;

    // BufferImpl implementation
    angle::Result setData(const gl::Context *context,
                          gl::BufferBinding target,
                          const void *data,
                          size_t size,
                          gl::BufferUsage usage) override;
    angle::Result setSubData(const gl::Context *context,
                             gl::BufferBinding target,
                             const void *data,
                             size_t size,
                             size_t offset) override;
    angle::Result copySubData(const gl::Context *context,
                              BufferImpl *source,
                              GLintptr sourceOffset,
                              GLintptr destOffset,
                              GLsizeiptr size) override;
    angle::Result map(const gl::Context *context, GLenum access, void **mapPtr) override;
    angle::Result mapRange(const gl::Context *context,
                           size_t offset,
                           size_t length,
                           GLbitfield access,
                           void **mapPtr) override;
    angle::Result unmap(const gl::Context *context, GLboolean *result) override;
    angle::Result markTransformFeedbackUsage(const gl::Context *context) override;

  private:
    angle::MemoryBuffer mMemory;
    size_t mSize;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_BUFFER9_H_
