//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncWgpu.h:
//    Defines the class interface for SyncWgpu, implementing SyncImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_FENCESYNCWGPU_H_
#define LIBANGLE_RENDERER_WGPU_FENCESYNCWGPU_H_

#include "libANGLE/renderer/SyncImpl.h"

namespace rx
{
class SyncWgpu : public SyncImpl
{
  public:
    SyncWgpu();
    ~SyncWgpu() override;

    angle::Result set(const gl::Context *context, GLenum condition, GLbitfield flags) override;
    angle::Result clientWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout,
                             GLenum *outResult) override;
    angle::Result serverWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout) override;
    angle::Result getStatus(const gl::Context *context, GLint *outResult) override;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_FENCESYNCWGPU_H_
