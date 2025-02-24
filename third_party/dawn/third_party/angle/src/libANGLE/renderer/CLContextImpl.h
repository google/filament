//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextImpl.h: Defines the abstract rx::CLContextImpl class.

#ifndef LIBANGLE_RENDERER_CLCONTEXTIMPL_H_
#define LIBANGLE_RENDERER_CLCONTEXTIMPL_H_

#include "libANGLE/renderer/CLCommandQueueImpl.h"
#include "libANGLE/renderer/CLEventImpl.h"
#include "libANGLE/renderer/CLMemoryImpl.h"
#include "libANGLE/renderer/CLProgramImpl.h"
#include "libANGLE/renderer/CLSamplerImpl.h"

namespace rx
{

class CLContextImpl : angle::NonCopyable
{
  public:
    using Ptr = std::unique_ptr<CLContextImpl>;

    CLContextImpl(const cl::Context &context);
    virtual ~CLContextImpl();

    virtual angle::Result getDevices(cl::DevicePtrs *devicePtrsOut) const = 0;

    virtual angle::Result createCommandQueue(const cl::CommandQueue &commandQueue,
                                             CLCommandQueueImpl::Ptr *commandQueueOut) = 0;

    virtual angle::Result createBuffer(const cl::Buffer &buffer,
                                       void *hostPtr,
                                       CLMemoryImpl::Ptr *bufferOut) = 0;

    virtual angle::Result createImage(const cl::Image &image,
                                      void *hostPtr,
                                      CLMemoryImpl::Ptr *imageOut) = 0;

    virtual angle::Result getSupportedImageFormats(cl::MemFlags flags,
                                                   cl::MemObjectType imageType,
                                                   cl_uint numEntries,
                                                   cl_image_format *imageFormats,
                                                   cl_uint *numImageFormats) = 0;

    virtual angle::Result createSampler(const cl::Sampler &sampler,
                                        CLSamplerImpl::Ptr *samplerOut) = 0;

    virtual angle::Result createProgramWithSource(const cl::Program &program,
                                                  const std::string &source,
                                                  CLProgramImpl::Ptr *programOut) = 0;

    virtual angle::Result createProgramWithIL(const cl::Program &program,
                                              const void *il,
                                              size_t length,
                                              CLProgramImpl::Ptr *programOut) = 0;

    virtual angle::Result createProgramWithBinary(const cl::Program &program,
                                                  const size_t *lengths,
                                                  const unsigned char **binaries,
                                                  cl_int *binaryStatus,
                                                  CLProgramImpl::Ptr *programOut) = 0;

    virtual angle::Result createProgramWithBuiltInKernels(const cl::Program &program,
                                                          const char *kernel_names,
                                                          CLProgramImpl::Ptr *programOut) = 0;

    virtual angle::Result linkProgram(const cl::Program &program,
                                      const cl::DevicePtrs &devices,
                                      const char *options,
                                      const cl::ProgramPtrs &inputPrograms,
                                      cl::Program *notify,
                                      CLProgramImpl::Ptr *programOut) = 0;

    virtual angle::Result createUserEvent(const cl::Event &event, CLEventImpl::Ptr *eventOut) = 0;

    virtual angle::Result waitForEvents(const cl::EventPtrs &events) = 0;

  protected:
    const cl::Context &mContext;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLCONTEXTIMPL_H_
