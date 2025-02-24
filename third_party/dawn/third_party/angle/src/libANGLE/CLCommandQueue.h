//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueue.h: Defines the cl::CommandQueue class, which can be used to queue a set of OpenCL
// operations.

#ifndef LIBANGLE_CLCOMMANDQUEUE_H_
#define LIBANGLE_CLCOMMANDQUEUE_H_

#include "libANGLE/CLObject.h"
#include "libANGLE/cl_utils.h"
#include "libANGLE/renderer/CLCommandQueueImpl.h"

#include "common/SynchronizedValue.h"

#include <limits>

namespace cl
{

class CommandQueue final : public _cl_command_queue, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    angle::Result getInfo(CommandQueueInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const;

    angle::Result setProperty(CommandQueueProperties properties,
                              cl_bool enable,
                              cl_command_queue_properties *oldProperties);

    angle::Result enqueueReadBuffer(cl_mem buffer,
                                    cl_bool blockingRead,
                                    size_t offset,
                                    size_t size,
                                    void *ptr,
                                    cl_uint numEventsInWaitList,
                                    const cl_event *eventWaitList,
                                    cl_event *event);

    angle::Result enqueueWriteBuffer(cl_mem buffer,
                                     cl_bool blockingWrite,
                                     size_t offset,
                                     size_t size,
                                     const void *ptr,
                                     cl_uint numEventsInWaitList,
                                     const cl_event *eventWaitList,
                                     cl_event *event);

    angle::Result enqueueReadBufferRect(cl_mem buffer,
                                        cl_bool blockingRead,
                                        const cl::MemOffsets &bufferOrigin,
                                        const cl::MemOffsets &hostOrigin,
                                        const cl::Coordinate &region,
                                        size_t bufferRowPitch,
                                        size_t bufferSlicePitch,
                                        size_t hostRowPitch,
                                        size_t hostSlicePitch,
                                        void *ptr,
                                        cl_uint numEventsInWaitList,
                                        const cl_event *eventWaitList,
                                        cl_event *event);

    angle::Result enqueueWriteBufferRect(cl_mem buffer,
                                         cl_bool blockingWrite,
                                         const cl::MemOffsets &bufferOrigin,
                                         const cl::MemOffsets &hostOrigin,
                                         const cl::Coordinate &region,
                                         size_t bufferRowPitch,
                                         size_t bufferSlicePitch,
                                         size_t hostRowPitch,
                                         size_t hostSlicePitch,
                                         const void *ptr,
                                         cl_uint numEventsInWaitList,
                                         const cl_event *eventWaitList,
                                         cl_event *event);

    angle::Result enqueueCopyBuffer(cl_mem srcBuffer,
                                    cl_mem dstBuffer,
                                    size_t srcOffset,
                                    size_t dstOffset,
                                    size_t size,
                                    cl_uint numEventsInWaitList,
                                    const cl_event *eventWaitList,
                                    cl_event *event);

    angle::Result enqueueCopyBufferRect(cl_mem srcBuffer,
                                        cl_mem dstBuffer,
                                        const cl::MemOffsets &srcOrigin,
                                        const cl::MemOffsets &dstOrigin,
                                        const cl::Coordinate &region,
                                        size_t srcRowPitch,
                                        size_t srcSlicePitch,
                                        size_t dstRowPitch,
                                        size_t dstSlicePitch,
                                        cl_uint numEventsInWaitList,
                                        const cl_event *eventWaitList,
                                        cl_event *event);

    angle::Result enqueueFillBuffer(cl_mem buffer,
                                    const void *pattern,
                                    size_t patternSize,
                                    size_t offset,
                                    size_t size,
                                    cl_uint numEventsInWaitList,
                                    const cl_event *eventWaitList,
                                    cl_event *event);

    angle::Result enqueueMapBuffer(cl_mem buffer,
                                   cl_bool blockingMap,
                                   MapFlags mapFlags,
                                   size_t offset,
                                   size_t size,
                                   cl_uint numEventsInWaitList,
                                   const cl_event *eventWaitList,
                                   cl_event *event,
                                   void *&mapPtr);

    angle::Result enqueueReadImage(cl_mem image,
                                   cl_bool blockingRead,
                                   const cl::MemOffsets &origin,
                                   const cl::Coordinate &region,
                                   size_t rowPitch,
                                   size_t slicePitch,
                                   void *ptr,
                                   cl_uint numEventsInWaitList,
                                   const cl_event *eventWaitList,
                                   cl_event *event);

    angle::Result enqueueWriteImage(cl_mem image,
                                    cl_bool blockingWrite,
                                    const cl::MemOffsets &origin,
                                    const cl::Coordinate &region,
                                    size_t inputRowPitch,
                                    size_t inputSlicePitch,
                                    const void *ptr,
                                    cl_uint numEventsInWaitList,
                                    const cl_event *eventWaitList,
                                    cl_event *event);

    angle::Result enqueueCopyImage(cl_mem srcImage,
                                   cl_mem dstImage,
                                   const cl::MemOffsets &srcOrigin,
                                   const cl::MemOffsets &dstOrigin,
                                   const cl::Coordinate &region,
                                   cl_uint numEventsInWaitList,
                                   const cl_event *eventWaitList,
                                   cl_event *event);

    angle::Result enqueueFillImage(cl_mem image,
                                   const void *fillColor,
                                   const cl::MemOffsets &origin,
                                   const cl::Coordinate &region,
                                   cl_uint numEventsInWaitList,
                                   const cl_event *eventWaitList,
                                   cl_event *event);

    angle::Result enqueueCopyImageToBuffer(cl_mem srcImage,
                                           cl_mem dstBuffer,
                                           const cl::MemOffsets &srcOrigin,
                                           const cl::Coordinate &region,
                                           size_t dstOffset,
                                           cl_uint numEventsInWaitList,
                                           const cl_event *eventWaitList,
                                           cl_event *event);

    angle::Result enqueueCopyBufferToImage(cl_mem srcBuffer,
                                           cl_mem dstImage,
                                           size_t srcOffset,
                                           const cl::MemOffsets &dstOrigin,
                                           const cl::Coordinate &region,
                                           cl_uint numEventsInWaitList,
                                           const cl_event *eventWaitList,
                                           cl_event *event);

    angle::Result enqueueMapImage(cl_mem image,
                                  cl_bool blockingMap,
                                  MapFlags mapFlags,
                                  const cl::MemOffsets &origin,
                                  const cl::Coordinate &region,
                                  size_t *imageRowPitch,
                                  size_t *imageSlicePitch,
                                  cl_uint numEventsInWaitList,
                                  const cl_event *eventWaitList,
                                  cl_event *event,
                                  void *&mapPtr);

    angle::Result enqueueUnmapMemObject(cl_mem memobj,
                                        void *mappedPtr,
                                        cl_uint numEventsInWaitList,
                                        const cl_event *eventWaitList,
                                        cl_event *event);

    angle::Result enqueueMigrateMemObjects(cl_uint numMemObjects,
                                           const cl_mem *memObjects,
                                           MemMigrationFlags flags,
                                           cl_uint numEventsInWaitList,
                                           const cl_event *eventWaitList,
                                           cl_event *event);

    angle::Result enqueueNDRangeKernel(cl_kernel kernel,
                                       const NDRange &ndrange,
                                       cl_uint numEventsInWaitList,
                                       const cl_event *eventWaitList,
                                       cl_event *event);

    angle::Result enqueueTask(cl_kernel kernel,
                              cl_uint numEventsInWaitList,
                              const cl_event *eventWaitList,
                              cl_event *event);

    angle::Result enqueueNativeKernel(UserFunc userFunc,
                                      void *args,
                                      size_t cbArgs,
                                      cl_uint numMemObjects,
                                      const cl_mem *memList,
                                      const void **argsMemLoc,
                                      cl_uint numEventsInWaitList,
                                      const cl_event *eventWaitList,
                                      cl_event *event);

    angle::Result enqueueMarkerWithWaitList(cl_uint numEventsInWaitList,
                                            const cl_event *eventWaitList,
                                            cl_event *event);

    angle::Result enqueueMarker(cl_event *event);

    angle::Result enqueueWaitForEvents(cl_uint numEvents, const cl_event *eventList);

    angle::Result enqueueBarrierWithWaitList(cl_uint numEventsInWaitList,
                                             const cl_event *eventWaitList,
                                             cl_event *event);

    angle::Result enqueueBarrier();

    angle::Result flush();
    angle::Result finish();

  public:
    using PropArray = std::vector<cl_queue_properties>;

    static constexpr cl_uint kNoSize = std::numeric_limits<cl_uint>::max();

    ~CommandQueue() override;

    Context &getContext();
    const Context &getContext() const;
    const Device &getDevice() const;

    // Get index of device in the context.
    size_t getDeviceIndex() const;

    CommandQueueProperties getProperties() const;
    bool isOnHost() const;
    bool isOnDevice() const;

    bool hasSize() const;
    cl_uint getSize() const;

    template <typename T = rx::CLCommandQueueImpl>
    T &getImpl() const;

    cl_int onRelease();

  private:
    CommandQueue(Context &context,
                 Device &device,
                 PropArray &&propArray,
                 CommandQueueProperties properties,
                 cl_uint size);

    CommandQueue(Context &context, Device &device, CommandQueueProperties properties);

    const ContextPtr mContext;
    const DevicePtr mDevice;
    const PropArray mPropArray;
    angle::SynchronizedValue<CommandQueueProperties> mProperties;
    const cl_uint mSize = kNoSize;
    rx::CLCommandQueueImpl::Ptr mImpl;

    friend class Object;
};

inline Context &CommandQueue::getContext()
{
    return *mContext;
}

inline const Context &CommandQueue::getContext() const
{
    return *mContext;
}

inline const Device &CommandQueue::getDevice() const
{
    return *mDevice;
}

inline CommandQueueProperties CommandQueue::getProperties() const
{
    return *mProperties;
}

inline bool CommandQueue::isOnHost() const
{
    return mProperties->excludes(CL_QUEUE_ON_DEVICE);
}

inline bool CommandQueue::isOnDevice() const
{
    return mProperties->intersects(CL_QUEUE_ON_DEVICE);
}

inline bool CommandQueue::hasSize() const
{
    return mSize != kNoSize;
}

inline cl_uint CommandQueue::getSize() const
{
    return mSize;
}

template <typename T>
inline T &CommandQueue::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

inline cl_int CommandQueue::onRelease()
{
    // Perform implicit submission on queue release
    // https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clReleaseCommandQueue
    if (IsError(finish()))
    {
        WARN() << "Failed to perform implicit submission on queue release!";
        return CL_OUT_OF_RESOURCES;
    }
    return CL_SUCCESS;
}

}  // namespace cl

#endif  // LIBANGLE_CLCOMMANDQUEUE_H_
