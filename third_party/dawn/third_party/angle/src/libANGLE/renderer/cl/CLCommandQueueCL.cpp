//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueCL.cpp: Implements the class methods for CLCommandQueueCL.

#include "libANGLE/renderer/cl/CLCommandQueueCL.h"

#include "libANGLE/renderer/cl/CLContextCL.h"
#include "libANGLE/renderer/cl/CLEventCL.h"
#include "libANGLE/renderer/cl/CLKernelCL.h"
#include "libANGLE/renderer/cl/CLMemoryCL.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"

namespace rx
{

namespace
{

void CheckCreateEvent(cl_event nativeEvent, CLEventImpl::CreateFunc *createFunc)
{
    if (createFunc != nullptr)
    {
        *createFunc = [nativeEvent](const cl::Event &event) {
            return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
        };
    }
}

}  // namespace

CLCommandQueueCL::CLCommandQueueCL(const cl::CommandQueue &commandQueue, cl_command_queue native)
    : CLCommandQueueImpl(commandQueue), mNative(native)
{
    if (commandQueue.getProperties().intersects(CL_QUEUE_ON_DEVICE))
    {
        commandQueue.getContext().getImpl<CLContextCL>().mData->mDeviceQueues.emplace(
            commandQueue.getNative());
    }
}

CLCommandQueueCL::~CLCommandQueueCL()
{
    if (mCommandQueue.getProperties().intersects(CL_QUEUE_ON_DEVICE))
    {
        const size_t numRemoved =
            mCommandQueue.getContext().getImpl<CLContextCL>().mData->mDeviceQueues.erase(
                mCommandQueue.getNative());
        ASSERT(numRemoved == 1u);
    }

    if (mNative->getDispatch().clReleaseCommandQueue(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL command-queue";
    }
}

angle::Result CLCommandQueueCL::setProperty(cl::CommandQueueProperties properties, cl_bool enable)
{
    ANGLE_CL_TRY(mNative->getDispatch().clSetCommandQueueProperty(mNative, properties.get(), enable,
                                                                  nullptr));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueReadBuffer(const cl::Buffer &buffer,
                                                  bool blocking,
                                                  size_t offset,
                                                  size_t size,
                                                  void *ptr,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueReadBuffer(mNative, nativeBuffer, block, offset,
                                                            size, ptr, numEvents, nativeEventsPtr,
                                                            nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueWriteBuffer(const cl::Buffer &buffer,
                                                   bool blocking,
                                                   size_t offset,
                                                   size_t size,
                                                   const void *ptr,
                                                   const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueWriteBuffer(mNative, nativeBuffer, block, offset,
                                                             size, ptr, numEvents, nativeEventsPtr,
                                                             nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueReadBufferRect(const cl::Buffer &buffer,
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
                                                      CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t bufferOriginArray[3]              = {bufferOrigin.x, bufferOrigin.y, bufferOrigin.z};
    size_t hostOriginArray[3]                = {hostOrigin.x, hostOrigin.y, hostOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueReadBufferRect(
        mNative, nativeBuffer, block, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueWriteBufferRect(const cl::Buffer &buffer,
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
                                                       CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t bufferOriginArray[3]              = {bufferOrigin.x, bufferOrigin.y, bufferOrigin.z};
    size_t hostOriginArray[3]                = {hostOrigin.x, hostOrigin.y, hostOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueWriteBufferRect(
        mNative, nativeBuffer, block, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                                  const cl::Buffer &dstBuffer,
                                                  size_t srcOffset,
                                                  size_t dstOffset,
                                                  size_t size,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeSrc                   = srcBuffer.getImpl<CLMemoryCL>().getNative();
    const cl_mem nativeDst                   = dstBuffer.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueCopyBuffer(mNative, nativeSrc, nativeDst,
                                                            srcOffset, dstOffset, size, numEvents,
                                                            nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
                                                      const cl::Buffer &dstBuffer,
                                                      const cl::MemOffsets &srcOrigin,
                                                      const cl::MemOffsets &dstOrigin,
                                                      const cl::Coordinate &region,
                                                      size_t srcRowPitch,
                                                      size_t srcSlicePitch,
                                                      size_t dstRowPitch,
                                                      size_t dstSlicePitch,
                                                      const cl::EventPtrs &waitEvents,
                                                      CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeSrc                   = srcBuffer.getImpl<CLMemoryCL>().getNative();
    const cl_mem nativeDst                   = dstBuffer.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t srcOriginArray[3]                 = {srcOrigin.x, srcOrigin.y, srcOrigin.z};
    size_t dstOriginArray[3]                 = {dstOrigin.x, dstOrigin.y, dstOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueCopyBufferRect(
        mNative, nativeSrc, nativeDst, srcOriginArray, dstOriginArray, regionArray, srcRowPitch,
        srcSlicePitch, dstRowPitch, dstSlicePitch, numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueFillBuffer(const cl::Buffer &buffer,
                                                  const void *pattern,
                                                  size_t patternSize,
                                                  size_t offset,
                                                  size_t size,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueFillBuffer(mNative, nativeBuffer, pattern,
                                                            patternSize, offset, size, numEvents,
                                                            nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueMapBuffer(const cl::Buffer &buffer,
                                                 bool blocking,
                                                 cl::MapFlags mapFlags,
                                                 size_t offset,
                                                 size_t size,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc,
                                                 void *&mapPtr)
{
    const cl_mem nativeBuffer                = buffer.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    cl_int errorCode = CL_SUCCESS;
    mapPtr = mNative->getDispatch().clEnqueueMapBuffer(mNative, nativeBuffer, block, mapFlags.get(),
                                                       offset, size, numEvents, nativeEventsPtr,
                                                       nativeEventPtr, &errorCode);
    ANGLE_CL_TRY(errorCode);

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueReadImage(const cl::Image &image,
                                                 bool blocking,
                                                 const cl::MemOffsets &origin,
                                                 const cl::Coordinate &region,
                                                 size_t rowPitch,
                                                 size_t slicePitch,
                                                 void *ptr,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeImage                 = image.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t originArray[3]                    = {origin.x, origin.y, origin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueReadImage(
        mNative, nativeImage, block, originArray, regionArray, rowPitch, slicePitch, ptr, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueWriteImage(const cl::Image &image,
                                                  bool blocking,
                                                  const cl::MemOffsets &origin,
                                                  const cl::Coordinate &region,
                                                  size_t inputRowPitch,
                                                  size_t inputSlicePitch,
                                                  const void *ptr,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeImage                 = image.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t originArray[3]                    = {origin.x, origin.y, origin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueWriteImage(
        mNative, nativeImage, block, originArray, regionArray, inputRowPitch, inputSlicePitch, ptr,
        numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueCopyImage(const cl::Image &srcImage,
                                                 const cl::Image &dstImage,
                                                 const cl::MemOffsets &srcOrigin,
                                                 const cl::MemOffsets &dstOrigin,
                                                 const cl::Coordinate &region,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeSrc                   = srcImage.getImpl<CLMemoryCL>().getNative();
    const cl_mem nativeDst                   = dstImage.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t srcOriginArray[3]                 = {srcOrigin.x, srcOrigin.y, srcOrigin.z};
    size_t dstOriginArray[3]                 = {dstOrigin.x, dstOrigin.y, dstOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueCopyImage(
        mNative, nativeSrc, nativeDst, srcOriginArray, dstOriginArray, regionArray, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueFillImage(const cl::Image &image,
                                                 const void *fillColor,
                                                 const cl::MemOffsets &origin,
                                                 const cl::Coordinate &region,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeImage                 = image.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t originArray[3]                    = {origin.x, origin.y, origin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueFillImage(mNative, nativeImage, fillColor,
                                                           originArray, regionArray, numEvents,
                                                           nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueCopyImageToBuffer(const cl::Image &srcImage,
                                                         const cl::Buffer &dstBuffer,
                                                         const cl::MemOffsets &srcOrigin,
                                                         const cl::Coordinate &region,
                                                         size_t dstOffset,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeSrc                   = srcImage.getImpl<CLMemoryCL>().getNative();
    const cl_mem nativeDst                   = dstBuffer.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t srcOriginArray[3]                 = {srcOrigin.x, srcOrigin.y, srcOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueCopyImageToBuffer(
        mNative, nativeSrc, nativeDst, srcOriginArray, regionArray, dstOffset, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueCopyBufferToImage(const cl::Buffer &srcBuffer,
                                                         const cl::Image &dstImage,
                                                         size_t srcOffset,
                                                         const cl::MemOffsets &dstOrigin,
                                                         const cl::Coordinate &region,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeSrc                   = srcBuffer.getImpl<CLMemoryCL>().getNative();
    const cl_mem nativeDst                   = dstImage.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t dstOriginArray[3]                 = {dstOrigin.x, dstOrigin.y, dstOrigin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueCopyBufferToImage(
        mNative, nativeSrc, nativeDst, srcOffset, dstOriginArray, regionArray, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueMapImage(const cl::Image &image,
                                                bool blocking,
                                                cl::MapFlags mapFlags,
                                                const cl::MemOffsets &origin,
                                                const cl::Coordinate &region,
                                                size_t *imageRowPitch,
                                                size_t *imageSlicePitch,
                                                const cl::EventPtrs &waitEvents,
                                                CLEventImpl::CreateFunc *eventCreateFunc,
                                                void *&mapPtr)
{
    const cl_mem nativeImage                 = image.getImpl<CLMemoryCL>().getNative();
    const cl_bool block                      = blocking ? CL_TRUE : CL_FALSE;
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    size_t originArray[3]                    = {origin.x, origin.y, origin.z};
    size_t regionArray[3]                    = {region.x, region.y, region.z};

    cl_int errorCode = CL_SUCCESS;
    mapPtr           = mNative->getDispatch().clEnqueueMapImage(
        mNative, nativeImage, block, mapFlags.get(), originArray, regionArray, imageRowPitch,
        imageSlicePitch, numEvents, nativeEventsPtr, nativeEventPtr, &errorCode);
    ANGLE_CL_TRY(errorCode);

    // TODO(jplate) Remove workaround after bug is fixed http://anglebug.com/42264597
    if (imageSlicePitch != nullptr && (image.getType() == cl::MemObjectType::Image1D ||
                                       image.getType() == cl::MemObjectType::Image1D_Buffer ||
                                       image.getType() == cl::MemObjectType::Image2D))
    {
        *imageSlicePitch = 0u;
    }

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueUnmapMemObject(const cl::Memory &memory,
                                                      void *mappedPtr,
                                                      const cl::EventPtrs &waitEvents,
                                                      CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_mem nativeMemory                = memory.getImpl<CLMemoryCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueUnmapMemObject(
        mNative, nativeMemory, mappedPtr, numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueMigrateMemObjects(const cl::MemoryPtrs &memObjects,
                                                         cl::MemMigrationFlags flags,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::vector<cl_mem> nativeMemories;
    nativeMemories.reserve(memObjects.size());
    for (const cl::MemoryPtr &memory : memObjects)
    {
        nativeMemories.emplace_back(memory->getImpl<CLMemoryCL>().getNative());
    }
    const cl_uint numMemories                = static_cast<cl_uint>(nativeMemories.size());
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueMigrateMemObjects(
        mNative, numMemories, nativeMemories.data(), flags.get(), numEvents, nativeEventsPtr,
        nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueNDRangeKernel(const cl::Kernel &kernel,
                                                     const cl::NDRange &ndrange,
                                                     const cl::EventPtrs &waitEvents,
                                                     CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_kernel nativeKernel             = kernel.getImpl<CLKernelCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;
    std::array<size_t, 3> globalWorkOffset{ndrange.globalWorkOffset[0], ndrange.globalWorkOffset[1],
                                           ndrange.globalWorkOffset[2]};
    std::array<size_t, 3> globalWorkSize{ndrange.globalWorkSize[0], ndrange.globalWorkSize[1],
                                         ndrange.globalWorkSize[2]};
    std::array<size_t, 3> localWorkSize{ndrange.localWorkSize[0], ndrange.localWorkSize[1],
                                        ndrange.localWorkSize[2]};

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueNDRangeKernel(
        mNative, nativeKernel, ndrange.workDimensions, globalWorkOffset.data(),
        globalWorkSize.data(), localWorkSize.data(), numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueTask(const cl::Kernel &kernel,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc)
{
    const cl_kernel nativeKernel             = kernel.getImpl<CLKernelCL>().getNative();
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueTask(mNative, nativeKernel, numEvents,
                                                      nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueNativeKernel(cl::UserFunc userFunc,
                                                    void *args,
                                                    size_t cbArgs,
                                                    const cl::BufferPtrs &buffers,
                                                    const std::vector<size_t> bufferPtrOffsets,
                                                    const cl::EventPtrs &waitEvents,
                                                    CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::vector<unsigned char> funcArgs;
    std::vector<const void *> locs;
    if (!bufferPtrOffsets.empty())
    {
        // If argument memory block contains buffers, make a copy.
        funcArgs.resize(cbArgs);
        std::memcpy(funcArgs.data(), args, cbArgs);

        locs.reserve(bufferPtrOffsets.size());
        for (size_t offset : bufferPtrOffsets)
        {
            // Fetch location of buffer in copied function argument memory block.
            void *const loc = &funcArgs[offset];
            locs.emplace_back(loc);

            // Cast cl::Buffer to native cl_mem object in place.
            cl::Buffer *const buffer         = *reinterpret_cast<cl::Buffer **>(loc);
            *reinterpret_cast<cl_mem *>(loc) = buffer->getImpl<CLMemoryCL>().getNative();
        }

        // Use copied argument memory block.
        args = funcArgs.data();
    }

    std::vector<cl_mem> nativeBuffers;
    nativeBuffers.reserve(buffers.size());
    for (const cl::BufferPtr &buffer : buffers)
    {
        nativeBuffers.emplace_back(buffer->getImpl<CLMemoryCL>().getNative());
    }
    const cl_uint numBuffers             = static_cast<cl_uint>(nativeBuffers.size());
    const cl_mem *const nativeBuffersPtr = nativeBuffers.empty() ? nullptr : nativeBuffers.data();
    const void **const locsPtr           = locs.empty() ? nullptr : locs.data();

    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueNativeKernel(
        mNative, userFunc, args, cbArgs, numBuffers, nativeBuffersPtr, locsPtr, numEvents,
        nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueMarkerWithWaitList(const cl::EventPtrs &waitEvents,
                                                          CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueMarkerWithWaitList(
        mNative, numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueMarker(CLEventImpl::CreateFunc &eventCreateFunc)
{
    cl_event nativeEvent = nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueMarker(mNative, &nativeEvent));

    eventCreateFunc = [nativeEvent](const cl::Event &event) {
        return CLEventImpl::Ptr(new CLEventCL(event, nativeEvent));
    };
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueWaitForEvents(const cl::EventPtrs &events)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(events);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());

    ANGLE_CL_TRY(
        mNative->getDispatch().clEnqueueWaitForEvents(mNative, numEvents, nativeEvents.data()));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueBarrierWithWaitList(const cl::EventPtrs &waitEvents,
                                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    const std::vector<cl_event> nativeEvents = CLEventCL::Cast(waitEvents);
    const cl_uint numEvents                  = static_cast<cl_uint>(nativeEvents.size());
    const cl_event *const nativeEventsPtr    = nativeEvents.empty() ? nullptr : nativeEvents.data();
    cl_event nativeEvent                     = nullptr;
    cl_event *const nativeEventPtr           = eventCreateFunc != nullptr ? &nativeEvent : nullptr;

    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueBarrierWithWaitList(
        mNative, numEvents, nativeEventsPtr, nativeEventPtr));

    CheckCreateEvent(nativeEvent, eventCreateFunc);
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::enqueueBarrier()
{
    ANGLE_CL_TRY(mNative->getDispatch().clEnqueueBarrier(mNative));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::flush()
{
    ANGLE_CL_TRY(mNative->getDispatch().clFlush(mNative));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueCL::finish()
{
    ANGLE_CL_TRY(mNative->getDispatch().clFinish(mNative));
    return angle::Result::Continue;
}

}  // namespace rx
