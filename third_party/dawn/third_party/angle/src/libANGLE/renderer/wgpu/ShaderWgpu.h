//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderWgpu.h:
//    Defines the class interface for ShaderWgpu, implementing ShaderImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_SHADERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_SHADERWGPU_H_

#include "libANGLE/renderer/ShaderImpl.h"

namespace rx
{

class ShaderWgpu : public ShaderImpl
{
  public:
    ShaderWgpu(const gl::ShaderState &data);
    ~ShaderWgpu() override;

    std::shared_ptr<ShaderTranslateTask> compile(const gl::Context *context,
                                                 ShCompileOptions *options) override;
    std::shared_ptr<ShaderTranslateTask> load(const gl::Context *context,
                                              gl::BinaryInputStream *stream) override;

    std::string getDebugInfo() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_SHADERWGPU_H_
