//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextCL.h: Defines the class interface for CLContextCL, implementing CLContextImpl.

#ifndef LIBANGLE_RENDERER_CL_CLCONTEXTCL_H_
#define LIBANGLE_RENDERER_CL_CLCONTEXTCL_H_

#include "libANGLE/renderer/cl/cl_types.h"

#include "libANGLE/renderer/CLContextImpl.h"

#include "common/SynchronizedValue.h"

#include <unordered_set>

namespace rx
{

class CLContextCL : public CLContextImpl
{
  public:
    CLContextCL(const cl::Context &context, cl_context native);
    ~CLContextCL() override;

    bool hasMemory(cl_mem memory) const;
    bool hasSampler(cl_sampler sampler) const;
    bool hasDeviceQueue(cl_command_queue queue) const;

    angle::Result getDevices(cl::DevicePtrs *devicePtrsOut) const override;

    angle::Result createCommandQueue(const cl::CommandQueue &commandQueue,
                                     CLCommandQueueImpl::Ptr *commandQueueOut) override;

    angle::Result createBuffer(const cl::Buffer &buffer,
                               void *hostPtr,
                               CLMemoryImpl::Ptr *bufferOut) override;

    angle::Result createImage(const cl::Image &image,
                              void *hostPtr,
                              CLMemoryImpl::Ptr *imageOut) override;

    angle::Result getSupportedImageFormats(cl::MemFlags flags,
                                           cl::MemObjectType imageType,
                                           cl_uint numEntries,
                                           cl_image_format *imageFormats,
                                           cl_uint *numImageFormats) override;

    angle::Result createSampler(const cl::Sampler &sampler,
                                CLSamplerImpl::Ptr *samplerOut) override;

    angle::Result createProgramWithSource(const cl::Program &program,
                                          const std::string &source,
                                          CLProgramImpl::Ptr *programOut) override;

    angle::Result createProgramWithIL(const cl::Program &program,
                                      const void *il,
                                      size_t length,
                                      CLProgramImpl::Ptr *programOut) override;

    angle::Result createProgramWithBinary(const cl::Program &program,
                                          const size_t *lengths,
                                          const unsigned char **binaries,
                                          cl_int *binaryStatus,
                                          CLProgramImpl::Ptr *programOut) override;

    angle::Result createProgramWithBuiltInKernels(const cl::Program &program,
                                                  const char *kernel_names,
                                                  CLProgramImpl::Ptr *programOut) override;

    angle::Result linkProgram(const cl::Program &program,
                              const cl::DevicePtrs &devices,
                              const char *options,
                              const cl::ProgramPtrs &inputPrograms,
                              cl::Program *notify,
                              CLProgramImpl::Ptr *programOut) override;

    angle::Result createUserEvent(const cl::Event &event, CLEventImpl::Ptr *eventOut) override;

    angle::Result waitForEvents(const cl::EventPtrs &events) override;

  private:
    struct Mutable
    {
        std::unordered_set<const _cl_mem *> mMemories;
        std::unordered_set<const _cl_sampler *> mSamplers;
        std::unordered_set<const _cl_command_queue *> mDeviceQueues;
    };
    using MutableData = angle::SynchronizedValue<Mutable>;

    const cl_context mNative;
    MutableData mData;

    friend class CLCommandQueueCL;
    friend class CLMemoryCL;
    friend class CLSamplerCL;
};

inline bool CLContextCL::hasMemory(cl_mem memory) const
{
    const auto data = mData.synchronize();
    return data->mMemories.find(memory) != data->mMemories.cend();
}

inline bool CLContextCL::hasSampler(cl_sampler sampler) const
{
    const auto data = mData.synchronize();
    return data->mSamplers.find(sampler) != data->mSamplers.cend();
}

inline bool CLContextCL::hasDeviceQueue(cl_command_queue queue) const
{
    const auto data = mData.synchronize();
    return data->mDeviceQueues.find(queue) != data->mDeviceQueues.cend();
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLCONTEXTCL_H_
