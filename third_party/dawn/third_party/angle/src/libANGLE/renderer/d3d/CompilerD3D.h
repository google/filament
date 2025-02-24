//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompilerD3D.h: Defines the rx::CompilerD3D class, an implementation of rx::CompilerImpl.

#ifndef LIBANGLE_RENDERER_COMPILERD3D_H_
#define LIBANGLE_RENDERER_COMPILERD3D_H_

#include "libANGLE/renderer/CompilerImpl.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{

class CompilerD3D : public CompilerImpl
{
  public:
    CompilerD3D(ShShaderOutput translatorOutputType);
    ~CompilerD3D() override {}

    ShShaderOutput getTranslatorOutputType() const override;

  private:
    ShShaderOutput mTranslatorOutputType;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_COMPILERD3D_H_
