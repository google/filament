//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerNULL.h:
//    Defines the class interface for SamplerNULL, implementing SamplerImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_SAMPLERNULL_H_
#define LIBANGLE_RENDERER_NULL_SAMPLERNULL_H_

#include "libANGLE/renderer/SamplerImpl.h"

namespace rx
{

class SamplerNULL : public SamplerImpl
{
  public:
    SamplerNULL(const gl::SamplerState &state);
    ~SamplerNULL() override;

    angle::Result syncState(const gl::Context *context, const bool dirty) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_SAMPLERNULL_H_
