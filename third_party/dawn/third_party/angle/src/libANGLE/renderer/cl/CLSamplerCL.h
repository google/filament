//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerCL.h: Defines the class interface for CLSamplerCL, implementing CLSamplerImpl.

#ifndef LIBANGLE_RENDERER_CL_CLSAMPLERCL_H_
#define LIBANGLE_RENDERER_CL_CLSAMPLERCL_H_

#include "libANGLE/renderer/cl/cl_types.h"

#include "libANGLE/renderer/CLSamplerImpl.h"

namespace rx
{

class CLSamplerCL : public CLSamplerImpl
{
  public:
    CLSamplerCL(const cl::Sampler &sampler, cl_sampler native);
    ~CLSamplerCL() override;

    cl_sampler getNative() const;

  private:
    const cl_sampler mNative;
};

inline cl_sampler CLSamplerCL::getNative() const
{
    return mNative;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLSAMPLERCL_H_
