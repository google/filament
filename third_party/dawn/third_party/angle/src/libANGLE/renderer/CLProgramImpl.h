//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramImpl.h: Defines the abstract rx::CLProgramImpl class.

#ifndef LIBANGLE_RENDERER_CLPROGRAMIMPL_H_
#define LIBANGLE_RENDERER_CLPROGRAMIMPL_H_

#include "libANGLE/renderer/CLKernelImpl.h"

namespace rx
{

class CLProgramImpl : angle::NonCopyable
{
  public:
    using Ptr = std::unique_ptr<CLProgramImpl>;

    CLProgramImpl(const cl::Program &program);
    virtual ~CLProgramImpl();

    virtual angle::Result build(const cl::DevicePtrs &devices,
                                const char *options,
                                cl::Program *notify) = 0;

    virtual angle::Result compile(const cl::DevicePtrs &devices,
                                  const char *options,
                                  const cl::ProgramPtrs &inputHeaders,
                                  const char **headerIncludeNames,
                                  cl::Program *notify) = 0;

    virtual angle::Result getInfo(cl::ProgramInfo name,
                                  size_t valueSize,
                                  void *value,
                                  size_t *valueSizeRet) const = 0;

    virtual angle::Result getBuildInfo(const cl::Device &device,
                                       cl::ProgramBuildInfo name,
                                       size_t valueSize,
                                       void *value,
                                       size_t *valueSizeRet) const = 0;

    virtual angle::Result createKernel(const cl::Kernel &kernel,
                                       const char *name,
                                       CLKernelImpl::Ptr *kernelOut) = 0;

    virtual angle::Result createKernels(cl_uint numKernels,
                                        CLKernelImpl::CreateFuncs &createFuncs,
                                        cl_uint *numKernelsRet) = 0;

  protected:
    const cl::Program &mProgram;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLPROGRAMIMPL_H_
