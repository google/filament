//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueImpl.h: Defines the abstract rx::CLCommandQueueImpl class.

#ifndef LIBANGLE_RENDERER_CLCOMMANDQUEUEIMPL_H_
#define LIBANGLE_RENDERER_CLCOMMANDQUEUEIMPL_H_

#include "libANGLE/renderer/CLEventImpl.h"

namespace rx
{

class CLCommandQueueImpl : angle::NonCopyable
{
  public:
    using Ptr = std::unique_ptr<CLCommandQueueImpl>;

    CLCommandQueueImpl(const cl::CommandQueue &commandQueue);
    virtual ~CLCommandQueueImpl();

    virtual angle::Result setProperty(cl::CommandQueueProperties properties, cl_bool enable) = 0;

    virtual angle::Result enqueueReadBuffer(const cl::Buffer &buffer,
                                            bool blocking,
                                            size_t offset,
                                            size_t size,
                                            void *ptr,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueWriteBuffer(const cl::Buffer &buffer,
                                             bool blocking,
                                             size_t offset,
                                             size_t size,
                                             const void *ptr,
                                             const cl::EventPtrs &waitEvents,
                                             CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueReadBufferRect(const cl::Buffer &buffer,
                                                bool blocking,
                                                const cl::MemOffsets &bufferOrigin,
                                                const cl::MemOffsets &hostOrigin,
                                                const cl::Coordinate &region,
                                                size_t bufferRowPitch,
                                                size_t bufferSlicePitch,
                                                size_t hostRowPitch,
                                                size_t hostSlicePitch,
                                                void *ptr,
                                                const cl::EventPtrs &waitEvents,
                                                CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueWriteBufferRect(const cl::Buffer &buffer,
                                                 bool blocking,
                                                 const cl::MemOffsets &bufferOrigin,
                                                 const cl::MemOffsets &hostOrigin,
                                                 const cl::Coordinate &region,
                                                 size_t bufferRowPitch,
                                                 size_t bufferSlicePitch,
                                                 size_t hostRowPitch,
                                                 size_t hostSlicePitch,
                                                 const void *ptr,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                            const cl::Buffer &dstBuffer,
                                            size_t srcOffset,
                                            size_t dstOffset,
                                            size_t size,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
                                                const cl::Buffer &dstBuffer,
                                                const cl::MemOffsets &srcOrigin,
                                                const cl::MemOffsets &dstOrigin,
                                                const cl::Coordinate &region,
                                                size_t srcRowPitch,
                                                size_t srcSlicePitch,
                                                size_t dstRowPitch,
                                                size_t dstSlicePitch,
                                                const cl::EventPtrs &waitEvents,
                                                CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueFillBuffer(const cl::Buffer &buffer,
                                            const void *pattern,
                                            size_t patternSize,
                                            size_t offset,
                                            size_t size,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueMapBuffer(const cl::Buffer &buffer,
                                           bool blocking,
                                           cl::MapFlags mapFlags,
                                           size_t offset,
                                           size_t size,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc,
                                           void *&mapPtr) = 0;

    virtual angle::Result enqueueReadImage(const cl::Image &image,
                                           bool blocking,
                                           const cl::MemOffsets &origin,
                                           const cl::Coordinate &region,
                                           size_t rowPitch,
                                           size_t slicePitch,
                                           void *ptr,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueWriteImage(const cl::Image &image,
                                            bool blocking,
                                            const cl::MemOffsets &origin,
                                            const cl::Coordinate &region,
                                            size_t inputRowPitch,
                                            size_t inputSlicePitch,
                                            const void *ptr,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueCopyImage(const cl::Image &srcImage,
                                           const cl::Image &dstImage,
                                           const cl::MemOffsets &srcOrigin,
                                           const cl::MemOffsets &dstOrigin,
                                           const cl::Coordinate &region,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueFillImage(const cl::Image &image,
                                           const void *fillColor,
                                           const cl::MemOffsets &origin,
                                           const cl::Coordinate &region,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueCopyImageToBuffer(const cl::Image &srcImage,
                                                   const cl::Buffer &dstBuffer,
                                                   const cl::MemOffsets &srcOrigin,
                                                   const cl::Coordinate &region,
                                                   size_t dstOffset,
                                                   const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueCopyBufferToImage(const cl::Buffer &srcBuffer,
                                                   const cl::Image &dstImage,
                                                   size_t srcOffset,
                                                   const cl::MemOffsets &dstOrigin,
                                                   const cl::Coordinate &region,
                                                   const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueMapImage(const cl::Image &image,
                                          bool blocking,
                                          cl::MapFlags mapFlags,
                                          const cl::MemOffsets &origin,
                                          const cl::Coordinate &region,
                                          size_t *imageRowPitch,
                                          size_t *imageSlicePitch,
                                          const cl::EventPtrs &waitEvents,
                                          CLEventImpl::CreateFunc *eventCreateFunc,
                                          void *&mapPtr) = 0;

    virtual angle::Result enqueueUnmapMemObject(const cl::Memory &memory,
                                                void *mappedPtr,
                                                const cl::EventPtrs &waitEvents,
                                                CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueMigrateMemObjects(const cl::MemoryPtrs &memObjects,
                                                   cl::MemMigrationFlags flags,
                                                   const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueNDRangeKernel(const cl::Kernel &kernel,
                                               const cl::NDRange &ndrange,
                                               const cl::EventPtrs &waitEvents,
                                               CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueTask(const cl::Kernel &kernel,
                                      const cl::EventPtrs &waitEvents,
                                      CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueNativeKernel(cl::UserFunc userFunc,
                                              void *args,
                                              size_t cbArgs,
                                              const cl::BufferPtrs &buffers,
                                              const std::vector<size_t> bufferPtrOffsets,
                                              const cl::EventPtrs &waitEvents,
                                              CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueMarkerWithWaitList(const cl::EventPtrs &waitEvents,
                                                    CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueMarker(CLEventImpl::CreateFunc &eventCreateFunc) = 0;

    virtual angle::Result enqueueWaitForEvents(const cl::EventPtrs &events) = 0;

    virtual angle::Result enqueueBarrierWithWaitList(const cl::EventPtrs &waitEvents,
                                                     CLEventImpl::CreateFunc *eventCreateFunc) = 0;

    virtual angle::Result enqueueBarrier() = 0;

    virtual angle::Result flush()  = 0;
    virtual angle::Result finish() = 0;

  protected:
    const cl::CommandQueue &mCommandQueue;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLCOMMANDQUEUEIMPL_H_
