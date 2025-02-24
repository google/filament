//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferWgpu.h:
//    Defines the class interface for BufferWgpu, implementing BufferImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_

#include "libANGLE/renderer/BufferImpl.h"

#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

#include <dawn/webgpu_cpp.h>

namespace rx
{

class BufferWgpu : public BufferImpl
{
  public:
    BufferWgpu(const gl::BufferState &state);
    ~BufferWgpu() override;

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

    angle::Result getIndexRange(const gl::Context *context,
                                gl::DrawElementsType type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                gl::IndexRange *outRange) override;

    webgpu::BufferHelper &getBuffer() { return mBuffer; }

  private:
    webgpu::BufferHelper mBuffer;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_
