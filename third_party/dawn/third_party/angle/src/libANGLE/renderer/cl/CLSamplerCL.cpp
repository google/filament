//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerCL.cpp: Implements the class methods for CLSamplerCL.

#include "libANGLE/renderer/cl/CLSamplerCL.h"

#include "libANGLE/renderer/cl/CLContextCL.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLSampler.h"

namespace rx
{

CLSamplerCL::CLSamplerCL(const cl::Sampler &sampler, cl_sampler native)
    : CLSamplerImpl(sampler), mNative(native)
{
    sampler.getContext().getImpl<CLContextCL>().mData->mSamplers.emplace(sampler.getNative());
}

CLSamplerCL::~CLSamplerCL()
{
    const size_t numRemoved =
        mSampler.getContext().getImpl<CLContextCL>().mData->mSamplers.erase(mSampler.getNative());
    ASSERT(numRemoved == 1u);

    if (mNative->getDispatch().clReleaseSampler(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL sampler";
    }
}

}  // namespace rx
