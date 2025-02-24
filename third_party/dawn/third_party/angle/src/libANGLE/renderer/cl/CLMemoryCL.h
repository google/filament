//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryCL.h: Defines the class interface for CLMemoryCL, implementing CLMemoryImpl.

#ifndef LIBANGLE_RENDERER_CL_CLMEMORYCL_H_
#define LIBANGLE_RENDERER_CL_CLMEMORYCL_H_

#include "libANGLE/renderer/CLMemoryImpl.h"

namespace rx
{

class CLMemoryCL : public CLMemoryImpl
{
  public:
    CLMemoryCL(const cl::Memory &memory, cl_mem native);
    ~CLMemoryCL() override;

    cl_mem getNative() const;

    angle::Result createSubBuffer(const cl::Buffer &buffer,
                                  cl::MemFlags flags,
                                  size_t size,
                                  CLMemoryImpl::Ptr *subBufferOut) override;

  private:
    const cl_mem mNative;
};

inline cl_mem CLMemoryCL::getNative() const
{
    return mNative;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLMEMORYCL_H_
