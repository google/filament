//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueue.cpp: Implements the cl::CommandQueue class.

#include "libANGLE/CLCommandQueue.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"

#include <cstring>

namespace cl
{

namespace
{

void InitializeOutputEvent(cl_event *event)
{
    if (event != nullptr)
    {
        *event = nullptr;
    }
}

void CheckCreateEvent(CommandQueue &queue,
                      cl_command_type commandType,
                      const rx::CLEventImpl::CreateFunc &createFunc,
                      cl_event *event)
{
    if (event != nullptr)
    {
        ASSERT(createFunc);
        *event = Object::Create<Event>(queue, commandType, createFunc);
    }
}

}  // namespace

angle::Result CommandQueue::getInfo(CommandQueueInfo name,
                                    size_t valueSize,
                                    void *value,
                                    size_t *valueSizeRet) const
{
    cl_command_queue_properties properties = 0u;
    cl_uint valUInt                        = 0u;
    void *valPointer                       = nullptr;
    const void *copyValue                  = nullptr;
    size_t copySize                        = 0u;

    switch (name)
    {
        case CommandQueueInfo::Context:
            valPointer = mContext->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case CommandQueueInfo::Device:
            valPointer = mDevice->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case CommandQueueInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case CommandQueueInfo::Properties:
            properties = mProperties->get();
            copyValue  = &properties;
            copySize   = sizeof(properties);
            break;
        case CommandQueueInfo::PropertiesArray:
            copyValue = mPropArray.data();
            copySize  = mPropArray.size() * sizeof(decltype(mPropArray)::value_type);
            break;
        case CommandQueueInfo::Size:
            copyValue = &mSize;
            copySize  = sizeof(mSize);
            break;
        case CommandQueueInfo::DeviceDefault:
            valPointer = CommandQueue::CastNative(*mDevice->mDefaultCommandQueue);
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as specified in the Command Queue Parameter table, and param_value is not a NULL value.
        if (valueSize < copySize)
        {
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
        }
        if (copyValue != nullptr)
        {
            std::memcpy(value, copyValue, copySize);
        }
    }
    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }
    return angle::Result::Continue;
}

angle::Result CommandQueue::setProperty(CommandQueueProperties properties,
                                        cl_bool enable,
                                        cl_command_queue_properties *oldProperties)
{
    auto props = mProperties.synchronize();
    if (oldProperties != nullptr)
    {
        *oldProperties = props->get();
    }

    ANGLE_TRY(mImpl->setProperty(properties, enable));

    if (enable == CL_FALSE)
    {
        props->clear(properties);
    }
    else
    {
        props->set(properties);
    }
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueReadBuffer(cl_mem buffer,
                                              cl_bool blockingRead,
                                              size_t offset,
                                              size_t size,
                                              void *ptr,
                                              cl_uint numEventsInWaitList,
                                              const cl_event *eventWaitList,
                                              cl_event *event)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const bool blocking        = blockingRead != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(
        mImpl->enqueueReadBuffer(buf, blocking, offset, size, ptr, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_READ_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueWriteBuffer(cl_mem buffer,
                                               cl_bool blockingWrite,
                                               size_t offset,
                                               size_t size,
                                               const void *ptr,
                                               cl_uint numEventsInWaitList,
                                               const cl_event *eventWaitList,
                                               cl_event *event)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const bool blocking        = blockingWrite != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueWriteBuffer(buf, blocking, offset, size, ptr, waitEvents,
                                        eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_WRITE_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueReadBufferRect(cl_mem buffer,
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
                                                  cl_event *event)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const bool blocking        = blockingRead != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueReadBufferRect(buf, blocking, bufferOrigin, hostOrigin, region,
                                           bufferRowPitch, bufferSlicePitch, hostRowPitch,
                                           hostSlicePitch, ptr, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_READ_BUFFER_RECT, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueWriteBufferRect(cl_mem buffer,
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
                                                   cl_event *event)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const bool blocking        = blockingWrite != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueWriteBufferRect(buf, blocking, bufferOrigin, hostOrigin, region,
                                            bufferRowPitch, bufferSlicePitch, hostRowPitch,
                                            hostSlicePitch, ptr, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_WRITE_BUFFER_RECT, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueCopyBuffer(cl_mem srcBuffer,
                                              cl_mem dstBuffer,
                                              size_t srcOffset,
                                              size_t dstOffset,
                                              size_t size,
                                              cl_uint numEventsInWaitList,
                                              const cl_event *eventWaitList,
                                              cl_event *event)
{
    const Buffer &src          = srcBuffer->cast<Buffer>();
    const Buffer &dst          = dstBuffer->cast<Buffer>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueCopyBuffer(src, dst, srcOffset, dstOffset, size, waitEvents,
                                       eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_COPY_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueCopyBufferRect(cl_mem srcBuffer,
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
                                                  cl_event *event)
{
    const Buffer &src          = srcBuffer->cast<Buffer>();
    const Buffer &dst          = dstBuffer->cast<Buffer>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueCopyBufferRect(src, dst, srcOrigin, dstOrigin, region, srcRowPitch,
                                           srcSlicePitch, dstRowPitch, dstSlicePitch, waitEvents,
                                           eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_COPY_BUFFER_RECT, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueFillBuffer(cl_mem buffer,
                                              const void *pattern,
                                              size_t patternSize,
                                              size_t offset,
                                              size_t size,
                                              cl_uint numEventsInWaitList,
                                              const cl_event *eventWaitList,
                                              cl_event *event)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueFillBuffer(buf, pattern, patternSize, offset, size, waitEvents,
                                       eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_FILL_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueMapBuffer(cl_mem buffer,
                                             cl_bool blockingMap,
                                             MapFlags mapFlags,
                                             size_t offset,
                                             size_t size,
                                             cl_uint numEventsInWaitList,
                                             const cl_event *eventWaitList,
                                             cl_event *event,
                                             void *&mapPtr)
{
    const Buffer &buf          = buffer->cast<Buffer>();
    const bool blocking        = blockingMap != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueMapBuffer(buf, blocking, mapFlags, offset, size, waitEvents,
                                      eventCreateFuncPtr, mapPtr));

    CheckCreateEvent(*this, CL_COMMAND_MAP_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueReadImage(cl_mem image,
                                             cl_bool blockingRead,
                                             const cl::MemOffsets &origin,
                                             const cl::Coordinate &region,
                                             size_t rowPitch,
                                             size_t slicePitch,
                                             void *ptr,
                                             cl_uint numEventsInWaitList,
                                             const cl_event *eventWaitList,
                                             cl_event *event)
{
    const Image &img           = image->cast<Image>();
    const bool blocking        = blockingRead != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueReadImage(img, blocking, origin, region, rowPitch, slicePitch, ptr,
                                      waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_READ_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueWriteImage(cl_mem image,
                                              cl_bool blockingWrite,
                                              const cl::MemOffsets &origin,
                                              const cl::Coordinate &region,
                                              size_t inputRowPitch,
                                              size_t inputSlicePitch,
                                              const void *ptr,
                                              cl_uint numEventsInWaitList,
                                              const cl_event *eventWaitList,
                                              cl_event *event)
{
    const Image &img           = image->cast<Image>();
    const bool blocking        = blockingWrite != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueWriteImage(img, blocking, origin, region, inputRowPitch,
                                       inputSlicePitch, ptr, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_WRITE_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueCopyImage(cl_mem srcImage,
                                             cl_mem dstImage,
                                             const cl::MemOffsets &srcOrigin,
                                             const cl::MemOffsets &dstOrigin,
                                             const cl::Coordinate &region,
                                             cl_uint numEventsInWaitList,
                                             const cl_event *eventWaitList,
                                             cl_event *event)
{
    const Image &src           = srcImage->cast<Image>();
    const Image &dst           = dstImage->cast<Image>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueCopyImage(src, dst, srcOrigin, dstOrigin, region, waitEvents,
                                      eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_COPY_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueFillImage(cl_mem image,
                                             const void *fillColor,
                                             const cl::MemOffsets &origin,
                                             const cl::Coordinate &region,
                                             cl_uint numEventsInWaitList,
                                             const cl_event *eventWaitList,
                                             cl_event *event)
{
    const Image &img           = image->cast<Image>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(
        mImpl->enqueueFillImage(img, fillColor, origin, region, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_FILL_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueCopyImageToBuffer(cl_mem srcImage,
                                                     cl_mem dstBuffer,
                                                     const cl::MemOffsets &srcOrigin,
                                                     const cl::Coordinate &region,
                                                     size_t dstOffset,
                                                     cl_uint numEventsInWaitList,
                                                     const cl_event *eventWaitList,
                                                     cl_event *event)
{
    const Image &src           = srcImage->cast<Image>();
    const Buffer &dst          = dstBuffer->cast<Buffer>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueCopyImageToBuffer(src, dst, srcOrigin, region, dstOffset, waitEvents,
                                              eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_COPY_IMAGE_TO_BUFFER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueCopyBufferToImage(cl_mem srcBuffer,
                                                     cl_mem dstImage,
                                                     size_t srcOffset,
                                                     const cl::MemOffsets &dstOrigin,
                                                     const cl::Coordinate &region,
                                                     cl_uint numEventsInWaitList,
                                                     const cl_event *eventWaitList,
                                                     cl_event *event)
{
    const Buffer &src          = srcBuffer->cast<Buffer>();
    const Image &dst           = dstImage->cast<Image>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueCopyBufferToImage(src, dst, srcOffset, dstOrigin, region, waitEvents,
                                              eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_COPY_BUFFER_TO_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueMapImage(cl_mem image,
                                            cl_bool blockingMap,
                                            MapFlags mapFlags,
                                            const cl::MemOffsets &origin,
                                            const cl::Coordinate &region,
                                            size_t *imageRowPitch,
                                            size_t *imageSlicePitch,
                                            cl_uint numEventsInWaitList,
                                            const cl_event *eventWaitList,
                                            cl_event *event,
                                            void *&mapPtr)
{
    const Image &img           = image->cast<Image>();
    const bool blocking        = blockingMap != CL_FALSE;
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueMapImage(img, blocking, mapFlags, origin, region, imageRowPitch,
                                     imageSlicePitch, waitEvents, eventCreateFuncPtr, mapPtr));

    CheckCreateEvent(*this, CL_COMMAND_MAP_IMAGE, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueUnmapMemObject(cl_mem memobj,
                                                  void *mappedPtr,
                                                  cl_uint numEventsInWaitList,
                                                  const cl_event *eventWaitList,
                                                  cl_event *event)
{
    const Memory &memory       = memobj->cast<Memory>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueUnmapMemObject(memory, mappedPtr, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_UNMAP_MEM_OBJECT, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueMigrateMemObjects(cl_uint numMemObjects,
                                                     const cl_mem *memObjects,
                                                     MemMigrationFlags flags,
                                                     cl_uint numEventsInWaitList,
                                                     const cl_event *eventWaitList,
                                                     cl_event *event)
{
    MemoryPtrs memories;
    memories.reserve(numMemObjects);
    while (numMemObjects-- != 0u)
    {
        memories.emplace_back(&(*memObjects++)->cast<Memory>());
    }
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueMigrateMemObjects(memories, flags, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_MIGRATE_MEM_OBJECTS, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueNDRangeKernel(cl_kernel kernel,
                                                 const NDRange &ndrange,
                                                 cl_uint numEventsInWaitList,
                                                 const cl_event *eventWaitList,
                                                 cl_event *event)
{
    const Kernel &krnl         = kernel->cast<Kernel>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueNDRangeKernel(krnl, ndrange, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_NDRANGE_KERNEL, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueTask(cl_kernel kernel,
                                        cl_uint numEventsInWaitList,
                                        const cl_event *eventWaitList,
                                        cl_event *event)
{
    const Kernel &krnl         = kernel->cast<Kernel>();
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueTask(krnl, waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_TASK, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueNativeKernel(UserFunc userFunc,
                                                void *args,
                                                size_t cbArgs,
                                                cl_uint numMemObjects,
                                                const cl_mem *memList,
                                                const void **argsMemLoc,
                                                cl_uint numEventsInWaitList,
                                                const cl_event *eventWaitList,
                                                cl_event *event)
{
    std::vector<unsigned char> funcArgs;
    BufferPtrs buffers;
    std::vector<size_t> offsets;
    if (numMemObjects != 0u)
    {
        // If argument memory block contains memory objects, make a copy.
        funcArgs.resize(cbArgs);
        std::memcpy(funcArgs.data(), args, cbArgs);
        buffers.reserve(numMemObjects);
        offsets.reserve(numMemObjects);

        while (numMemObjects-- != 0u)
        {
            buffers.emplace_back(&(*memList++)->cast<Buffer>());

            // Calc memory offset of cl_mem object in args.
            offsets.emplace_back(static_cast<const char *>(*argsMemLoc++) -
                                 static_cast<const char *>(args));

            // Fetch location of cl_mem object in copied function argument memory block.
            void *loc = &funcArgs[offsets.back()];

            // Cast cl_mem object to cl::Buffer pointer in place.
            *reinterpret_cast<Buffer **>(loc) = &(*reinterpret_cast<cl_mem *>(loc))->cast<Buffer>();
        }

        // Use copied argument memory block.
        args = funcArgs.data();
    }

    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueNativeKernel(userFunc, args, cbArgs, buffers, offsets, waitEvents,
                                         eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_NATIVE_KERNEL, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueMarkerWithWaitList(cl_uint numEventsInWaitList,
                                                      const cl_event *eventWaitList,
                                                      cl_event *event)
{
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueMarkerWithWaitList(waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_MARKER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueMarker(cl_event *event)
{
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueMarker(eventCreateFunc));

    CheckCreateEvent(*this, CL_COMMAND_MARKER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueWaitForEvents(cl_uint numEvents, const cl_event *eventList)
{
    return mImpl->enqueueWaitForEvents(Event::Cast(numEvents, eventList));
}

angle::Result CommandQueue::enqueueBarrierWithWaitList(cl_uint numEventsInWaitList,
                                                       const cl_event *eventWaitList,
                                                       cl_event *event)
{
    const EventPtrs waitEvents = Event::Cast(numEventsInWaitList, eventWaitList);
    rx::CLEventImpl::CreateFunc eventCreateFunc;
    rx::CLEventImpl::CreateFunc *const eventCreateFuncPtr =
        event != nullptr ? &eventCreateFunc : nullptr;
    InitializeOutputEvent(event);

    ANGLE_TRY(mImpl->enqueueBarrierWithWaitList(waitEvents, eventCreateFuncPtr));

    CheckCreateEvent(*this, CL_COMMAND_BARRIER, eventCreateFunc, event);
    return angle::Result::Continue;
}

angle::Result CommandQueue::enqueueBarrier()
{
    return mImpl->enqueueBarrier();
}

angle::Result CommandQueue::flush()
{
    return mImpl->flush();
}

angle::Result CommandQueue::finish()
{
    return mImpl->finish();
}

CommandQueue::~CommandQueue()
{
    auto queue = mDevice->mDefaultCommandQueue.synchronize();
    if (*queue == this)
    {
        *queue = nullptr;
    }
}

size_t CommandQueue::getDeviceIndex() const
{
    return std::find(mContext->getDevices().cbegin(), mContext->getDevices().cend(), mDevice) -
           mContext->getDevices().cbegin();
}

CommandQueue::CommandQueue(Context &context,
                           Device &device,
                           PropArray &&propArray,
                           CommandQueueProperties properties,
                           cl_uint size)
    : mContext(&context),
      mDevice(&device),
      mPropArray(std::move(propArray)),
      mProperties(properties),
      mSize(size),
      mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createCommandQueue(*this, &mImpl));
    if (mProperties->intersects(CL_QUEUE_ON_DEVICE_DEFAULT))
    {
        *mDevice->mDefaultCommandQueue = this;
    }
}

CommandQueue::CommandQueue(Context &context, Device &device, CommandQueueProperties properties)
    : mContext(&context), mDevice(&device), mProperties(properties), mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createCommandQueue(*this, &mImpl));
}

}  // namespace cl
