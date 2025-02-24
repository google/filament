//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerImpl.h: Defines the abstract rx::CLSamplerImpl class.

#ifndef LIBANGLE_RENDERER_CLSAMPLERIMPL_H_
#define LIBANGLE_RENDERER_CLSAMPLERIMPL_H_

#include "libANGLE/renderer/cl_types.h"

namespace rx
{

class CLSamplerImpl : angle::NonCopyable
{
  public:
    using Ptr = std::unique_ptr<CLSamplerImpl>;

    CLSamplerImpl(const cl::Sampler &sampler);
    virtual ~CLSamplerImpl();

  protected:
    const cl::Sampler &mSampler;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLSAMPLERIMPL_H_
