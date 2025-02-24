//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelCL.h: Defines the class interface for CLKernelCL, implementing CLKernelImpl.

#ifndef LIBANGLE_RENDERER_CL_CLKERNELCL_H_
#define LIBANGLE_RENDERER_CL_CLKERNELCL_H_

#include "libANGLE/renderer/cl/cl_types.h"

#include "libANGLE/renderer/CLKernelImpl.h"

namespace rx
{

class CLKernelCL : public CLKernelImpl
{
  public:
    CLKernelCL(const cl::Kernel &kernel, cl_kernel native);
    ~CLKernelCL() override;

    cl_kernel getNative() const;

    angle::Result setArg(cl_uint argIndex, size_t argSize, const void *argValue) override;

    angle::Result createInfo(Info *infoOut) const override;

  private:
    const cl_kernel mNative;
};

inline cl_kernel CLKernelCL::getNative() const
{
    return mNative;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLKERNELCL_H_
