//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerWgpu.h:
//    Defines the class interface for CompilerWgpu, implementing CompilerImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_COMPILERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_COMPILERWGPU_H_

#include "libANGLE/renderer/CompilerImpl.h"

namespace rx
{

class CompilerWgpu : public CompilerImpl
{
  public:
    CompilerWgpu();
    ~CompilerWgpu() override;

    // TODO(jmadill): Expose translator built-in resources init method.
    ShShaderOutput getTranslatorOutputType() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_COMPILERWGPU_H_
