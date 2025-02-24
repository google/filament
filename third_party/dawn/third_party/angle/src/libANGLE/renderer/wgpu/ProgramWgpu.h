//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramWgpu.h:
//    Defines the class interface for ProgramWgpu, implementing ProgramImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_PROGRAMWGPU_H_
#define LIBANGLE_RENDERER_WGPU_PROGRAMWGPU_H_

#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{

class ProgramWgpu : public ProgramImpl
{
  public:
    ProgramWgpu(const gl::ProgramState &state);
    ~ProgramWgpu() override;

    angle::Result load(const gl::Context *context,
                       gl::BinaryInputStream *stream,
                       std::shared_ptr<LinkTask> *loadTaskOut,
                       egl::CacheGetResult *resultOut) override;
    void save(const gl::Context *context, gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;
    void setSeparable(bool separable) override;

    angle::Result link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut) override;
    GLboolean validate(const gl::Caps &caps) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_PROGRAMWGPU_H_
