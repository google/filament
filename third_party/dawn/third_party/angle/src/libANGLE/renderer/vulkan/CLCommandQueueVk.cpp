//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueVk.cpp: Implements the class methods for CLCommandQueueVk.

#include "common/PackedCLEnums_autogen.h"
#include "common/system_utils.h"

#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLDeviceVk.h"
#include "libANGLE/renderer/vulkan/CLEventVk.h"
#include "libANGLE/renderer/vulkan/CLKernelVk.h"
#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLProgramVk.h"
#include "libANGLE/renderer/vulkan/CLSamplerVk.h"
#include "libANGLE/renderer/vulkan/cl_types.h"
#include "libANGLE/renderer/vulkan/clspv_utils.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_cl_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#include "libANGLE/renderer/serial_utils.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLSampler.h"
#include "libANGLE/Error.h"
#include "libANGLE/cl_types.h"
#include "libANGLE/cl_utils.h"

#include "spirv/unified1/NonSemanticClspvReflection.h"
#include "vulkan/vulkan_core.h"

#include <chrono>

namespace rx
{

namespace
{
static constexpr size_t kTimeoutInMS            = 10000;
static constexpr size_t kSleepInMS              = 500;
static constexpr size_t kTimeoutCheckIterations = kTimeoutInMS / kSleepInMS;

angle::Result SetEventsWithQueueSerialToState(const cl::EventPtrs &eventList,
                                              const QueueSerial &queueSerial,
                                              cl::ExecutionStatus state)
{

    ASSERT(state < cl::ExecutionStatus::EnumCount);

    for (cl::EventPtr event : eventList)
    {
        CLEventVk *eventVk = &event->getImpl<CLEventVk>();
        if (!eventVk->isUserEvent() && eventVk->usedByCommandBuffer(queueSerial))
        {
            ANGLE_TRY(eventVk->setStatusAndExecuteCallback(cl::ToCLenum(state)));
        }
    }
    return angle::Result::Continue;
}

DispatchWorkThread::DispatchWorkThread(CLCommandQueueVk *commandQueue)
    : mCommandQueue(commandQueue),
      mIsTerminating(false),
      mQueueSerials(kFixedQueueLimit),
      mQueueSerialIndex(kInvalidQueueSerialIndex)
{}

DispatchWorkThread::~DispatchWorkThread()
{
    ASSERT(mIsTerminating);
}

angle::Result DispatchWorkThread::init()
{
    mQueueSerialIndex = mCommandQueue->getQueueSerialIndex();
    ASSERT(mQueueSerialIndex != kInvalidQueueSerialIndex);

    mWorkerThread = std::thread(&DispatchWorkThread::finishLoop, this);

    return angle::Result::Continue;
}

void DispatchWorkThread::terminate()
{
    // Terminate the background thread
    {
        std::unique_lock<std::mutex> lock(mThreadMutex);
        mIsTerminating = true;
    }
    mHasWorkSubmitted.notify_all();
    if (mWorkerThread.joinable())
    {
        mWorkerThread.join();
    }
}

angle::Result DispatchWorkThread::notify(QueueSerial queueSerial)
{
    ASSERT(queueSerial.getIndex() == mQueueSerialIndex);

    // QueueSerials are always received in order, its either same or greater than last one
    std::unique_lock<std::mutex> ul(mThreadMutex);
    if (!mQueueSerials.empty())
    {
        QueueSerial &lastSerial = mQueueSerials.back();
        ASSERT(queueSerial >= lastSerial);
        if (queueSerial == lastSerial)
        {
            return angle::Result::Continue;
        }
    }

    // if the queue is full, it might be that device is lost, check for timeout
    size_t numIterations = 0;
    while (mQueueSerials.full() && numIterations < kTimeoutCheckIterations)
    {
        mHasEmptySlot.wait_for(ul, std::chrono::milliseconds(kSleepInMS),
                               [this]() { return !mQueueSerials.full(); });
        numIterations++;
    }
    if (numIterations == kTimeoutCheckIterations)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    mQueueSerials.push(queueSerial);
    mHasWorkSubmitted.notify_one();

    return angle::Result::Continue;
}

angle::Result DispatchWorkThread::finishLoop()
{
    angle::SetCurrentThreadName("ANGLE-CL-CQD");

    while (true)
    {
        std::unique_lock<std::mutex> ul(mThreadMutex);
        mHasWorkSubmitted.wait(ul, [this]() { return !mQueueSerials.empty() || mIsTerminating; });

        while (!mQueueSerials.empty())
        {
            QueueSerial queueSerial = mQueueSerials.front();
            mQueueSerials.pop();
            mHasEmptySlot.notify_one();
            ul.unlock();
            // finish the work associated with the queue serial
            ANGLE_TRY(mCommandQueue->finishQueueSerial(queueSerial));
            ul.lock();
        }

        if (mIsTerminating)
        {
            break;
        }
    }
    return angle::Result::Continue;
}

}  // namespace

CLCommandQueueVk::CLCommandQueueVk(const cl::CommandQueue &commandQueue)
    : CLCommandQueueImpl(commandQueue),
      mContext(&commandQueue.getContext().getImpl<CLContextVk>()),
      mDevice(&commandQueue.getDevice().getImpl<CLDeviceVk>()),
      mPrintfBuffer(nullptr),
      mComputePassCommands(nullptr),
      mQueueSerialIndex(kInvalidQueueSerialIndex),
      mNeedPrintfHandling(false),
      mPrintfInfos(nullptr),
      mFinishHandler(this)
{}

angle::Result CLCommandQueueVk::init()
{
    vk::Renderer *renderer = mContext->getRenderer();
    ASSERT(renderer);

    ANGLE_CL_IMPL_TRY_ERROR(vk::OutsideRenderPassCommandBuffer::InitializeCommandPool(
                                mContext, &mCommandPool.outsideRenderPassPool,
                                renderer->getQueueFamilyIndex(), getProtectionType()),
                            CL_OUT_OF_RESOURCES);

    ANGLE_CL_IMPL_TRY_ERROR(mContext->getRenderer()->getOutsideRenderPassCommandBufferHelper(
                                mContext, &mCommandPool.outsideRenderPassPool,
                                &mOutsideRenderPassCommandsAllocator, &mComputePassCommands),
                            CL_OUT_OF_RESOURCES);

    // Generate initial QueueSerial for command buffer helper
    ANGLE_CL_IMPL_TRY_ERROR(mContext->getRenderer()->allocateQueueSerialIndex(&mQueueSerialIndex),
                            CL_OUT_OF_RESOURCES);
    // and set an initial queue serial for the compute pass commands
    mComputePassCommands->setQueueSerial(
        mQueueSerialIndex, mContext->getRenderer()->generateQueueSerial(mQueueSerialIndex));

    // Initialize serials to be valid but appear submitted and finished.
    mLastFlushedQueueSerial   = QueueSerial(mQueueSerialIndex, Serial());
    mLastSubmittedQueueSerial = mLastFlushedQueueSerial;

    ANGLE_TRY(mFinishHandler.init());

    return angle::Result::Continue;
}

CLCommandQueueVk::~CLCommandQueueVk()
{
    mFinishHandler.terminate();

    ASSERT(mComputePassCommands->empty());
    ASSERT(!mNeedPrintfHandling);

    if (mPrintfBuffer)
    {
        // The lifetime of printf buffer is scoped to command queue, release and destroy.
        const bool wasLastUser = mPrintfBuffer->release();
        ASSERT(wasLastUser);
        delete mPrintfBuffer;
    }

    VkDevice vkDevice = mContext->getDevice();

    if (mQueueSerialIndex != kInvalidQueueSerialIndex)
    {
        mContext->getRenderer()->releaseQueueSerialIndex(mQueueSerialIndex);
        mQueueSerialIndex = kInvalidQueueSerialIndex;
    }

    // Recycle the current command buffers
    mContext->getRenderer()->recycleOutsideRenderPassCommandBufferHelper(&mComputePassCommands);
    mCommandPool.outsideRenderPassPool.destroy(vkDevice);
}

angle::Result CLCommandQueueVk::setProperty(cl::CommandQueueProperties properties, cl_bool enable)
{
    // NOTE: "clSetCommandQueueProperty" has been deprecated as of OpenCL 1.1
    // http://man.opencl.org/deprecated.html
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueReadBuffer(const cl::Buffer &buffer,
                                                  bool blocking,
                                                  size_t offset,
                                                  size_t size,
                                                  void *ptr,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));
    CLBufferVk *bufferVk = &buffer.getImpl<CLBufferVk>();

    if (blocking)
    {
        ANGLE_TRY(finishInternal());
        ANGLE_TRY(bufferVk->copyTo(ptr, offset, size));

        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));
    }
    else
    {
        // Stage a transfer routine
        HostTransferConfig transferConfig;
        transferConfig.type       = CL_COMMAND_READ_BUFFER;
        transferConfig.offset     = offset;
        transferConfig.size       = size;
        transferConfig.dstHostPtr = ptr;
        ANGLE_TRY(addToHostTransferList(bufferVk, transferConfig));

        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueWriteBuffer(const cl::Buffer &buffer,
                                                   bool blocking,
                                                   size_t offset,
                                                   size_t size,
                                                   const void *ptr,
                                                   const cl::EventPtrs &waitEvents,
                                                   CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    auto bufferVk = &buffer.getImpl<CLBufferVk>();

    if (blocking)
    {
        ANGLE_TRY(finishInternal());
        ANGLE_TRY(bufferVk->copyFrom(ptr, offset, size));

        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));
    }
    else
    {
        // Stage a transfer routine
        HostTransferConfig config;
        config.type       = CL_COMMAND_WRITE_BUFFER;
        config.offset     = offset;
        config.size       = size;
        config.srcHostPtr = ptr;
        ANGLE_TRY(addToHostTransferList(bufferVk, config));

        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueReadBufferRect(const cl::Buffer &buffer,
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
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));
    auto bufferVk = &buffer.getImpl<CLBufferVk>();

    cl::BufferRect bufferRect{cl::Offset{bufferOrigin.x, bufferOrigin.y, bufferOrigin.z},
                              cl::Extents{region.x, region.y, region.z}, bufferRowPitch,
                              bufferSlicePitch, 1};

    cl::BufferRect ptrRect{cl::Offset{hostOrigin.x, hostOrigin.y, hostOrigin.z},
                           cl::Extents{region.x, region.y, region.z}, hostRowPitch, hostSlicePitch,
                           1};

    if (blocking)
    {
        ANGLE_TRY(finishInternal());
        ANGLE_TRY(bufferVk->getRect(bufferRect, ptrRect, ptr));
    }
    else
    {
        // Stage a transfer routine
        HostTransferConfig config;
        config.type       = CL_COMMAND_READ_BUFFER_RECT;
        config.srcRect    = bufferRect;
        config.dstRect    = ptrRect;
        config.dstHostPtr = ptr;
        config.size       = bufferVk->getSize();
        ANGLE_TRY(addToHostTransferList(bufferVk, config));
    }

    ANGLE_TRY(createEvent(eventCreateFunc,
                          blocking ? cl::ExecutionStatus::Complete : cl::ExecutionStatus::Queued));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueWriteBufferRect(const cl::Buffer &buffer,
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
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));
    auto bufferVk = &buffer.getImpl<CLBufferVk>();

    cl::BufferRect bufferRect{cl::Offset{bufferOrigin.x, bufferOrigin.y, bufferOrigin.z},
                              cl::Extents{region.x, region.y, region.z}, bufferRowPitch,
                              bufferSlicePitch, 1};

    cl::BufferRect ptrRect{cl::Offset{hostOrigin.x, hostOrigin.y, hostOrigin.z},
                           cl::Extents{region.x, region.y, region.z}, hostRowPitch, hostSlicePitch,
                           1};

    if (blocking)
    {
        ANGLE_TRY(finishInternal());
        ANGLE_TRY(bufferVk->setRect(ptr, ptrRect, bufferRect));
    }
    else
    {
        // Stage a transfer routine
        HostTransferConfig config;
        config.type       = CL_COMMAND_WRITE_BUFFER_RECT;
        config.srcRect    = ptrRect;
        config.dstRect    = bufferRect;
        config.srcHostPtr = ptr;
        config.size       = bufferVk->getSize();
        ANGLE_TRY(addToHostTransferList(bufferVk, config));
    }

    ANGLE_TRY(createEvent(eventCreateFunc,
                          blocking ? cl::ExecutionStatus::Complete : cl::ExecutionStatus::Queued));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                                  const cl::Buffer &dstBuffer,
                                                  size_t srcOffset,
                                                  size_t dstOffset,
                                                  size_t size,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    CLBufferVk *srcBufferVk = &srcBuffer.getImpl<CLBufferVk>();
    CLBufferVk *dstBufferVk = &dstBuffer.getImpl<CLBufferVk>();

    vk::CommandBufferAccess access;
    if (srcBufferVk->isSubBuffer() && dstBufferVk->isSubBuffer() &&
        (srcBufferVk->getParent() == dstBufferVk->getParent()))
    {
        // this is a self copy
        access.onBufferSelfCopy(&srcBufferVk->getBuffer());
    }
    else
    {
        access.onBufferTransferRead(&srcBufferVk->getBuffer());
        access.onBufferTransferWrite(&dstBufferVk->getBuffer());
    }

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(getCommandBuffer(access, &commandBuffer));

    VkBufferCopy copyRegion = {srcOffset, dstOffset, size};
    // update the offset in the case of sub-buffers
    if (srcBufferVk->getOffset())
    {
        copyRegion.srcOffset += srcBufferVk->getOffset();
    }
    if (dstBufferVk->getOffset())
    {
        copyRegion.dstOffset += dstBufferVk->getOffset();
    }
    commandBuffer->copyBuffer(srcBufferVk->getBuffer().getBuffer(),
                              dstBufferVk->getBuffer().getBuffer(), 1, &copyRegion);

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
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
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    ANGLE_TRY(processWaitlist(waitEvents));
    ANGLE_TRY(finishInternal());

    cl::BufferRect srcRect{cl::Offset{srcOrigin.x, srcOrigin.y, srcOrigin.z},
                           cl::Extents{region.x, region.y, region.z}, srcRowPitch, srcSlicePitch,
                           1};

    cl::BufferRect dstRect{cl::Offset{dstOrigin.x, dstOrigin.y, dstOrigin.z},
                           cl::Extents{region.x, region.y, region.z}, dstRowPitch, dstSlicePitch,
                           1};

    auto srcBufferVk    = &srcBuffer.getImpl<CLBufferVk>();
    auto dstBufferVk    = &dstBuffer.getImpl<CLBufferVk>();
    uint8_t *mapPointer = nullptr;
    ANGLE_TRY(srcBufferVk->map(mapPointer));
    ASSERT(mapPointer);
    ANGLE_TRY(dstBufferVk->setRect(static_cast<const void *>(mapPointer), srcRect, dstRect));

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueFillBuffer(const cl::Buffer &buffer,
                                                  const void *pattern,
                                                  size_t patternSize,
                                                  size_t offset,
                                                  size_t size,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    CLBufferVk *bufferVk = &buffer.getImpl<CLBufferVk>();
    if (mComputePassCommands->usesBuffer(bufferVk->getBuffer()))
    {
        ANGLE_TRY(finishInternal());
    }

    ANGLE_TRY(bufferVk->fillWithPattern(pattern, patternSize, offset, size));

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueMapBuffer(const cl::Buffer &buffer,
                                                 bool blocking,
                                                 cl::MapFlags mapFlags,
                                                 size_t offset,
                                                 size_t size,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc,
                                                 void *&mapPtr)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    cl::ExecutionStatus eventComplete = cl::ExecutionStatus::Queued;
    if (blocking || !eventCreateFunc)
    {
        ANGLE_TRY(finishInternal());
        eventComplete = cl::ExecutionStatus::Complete;
    }

    CLBufferVk *bufferVk = &buffer.getImpl<CLBufferVk>();
    uint8_t *mapPointer  = nullptr;
    if (buffer.getFlags().intersects(CL_MEM_USE_HOST_PTR))
    {
        ANGLE_TRY(finishInternal());
        mapPointer = static_cast<uint8_t *>(buffer.getHostPtr()) + offset;
        ANGLE_TRY(bufferVk->copyTo(mapPointer, offset, size));
        eventComplete = cl::ExecutionStatus::Complete;
    }
    else
    {
        ANGLE_TRY(bufferVk->map(mapPointer, offset));
    }
    mapPtr = static_cast<void *>(mapPointer);

    if (bufferVk->isCurrentlyInUse())
    {
        eventComplete = cl::ExecutionStatus::Queued;
    }
    ANGLE_TRY(createEvent(eventCreateFunc, eventComplete));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::copyImageToFromBuffer(CLImageVk &imageVk,
                                                      vk::BufferHelper &buffer,
                                                      const cl::MemOffsets &origin,
                                                      const cl::Coordinate &region,
                                                      size_t bufferOffset,
                                                      ImageBufferCopyDirection direction)
{
    vk::CommandBufferAccess access;
    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    VkImageAspectFlags aspectFlags = imageVk.getImage().getAspectFlags();
    if (direction == ImageBufferCopyDirection::ToBuffer)
    {
        access.onImageTransferRead(aspectFlags, &imageVk.getImage());
        access.onBufferTransferWrite(&buffer);
    }
    else
    {
        access.onImageTransferWrite(gl::LevelIndex(0), 1, 0,
                                    static_cast<uint32_t>(imageVk.getArraySize()), aspectFlags,
                                    &imageVk.getImage());
        access.onBufferTransferRead(&buffer);
    }
    ANGLE_TRY(getCommandBuffer(access, &commandBuffer));

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset      = bufferOffset;
    copyRegion.bufferRowLength   = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageExtent       = cl_vk::GetExtent(imageVk.getExtentForCopy(region));
    copyRegion.imageOffset       = cl_vk::GetOffset(imageVk.getOffsetForCopy(origin));
    copyRegion.imageSubresource  = imageVk.getSubresourceLayersForCopy(
        origin, region, imageVk.getType(), ImageCopyWith::Buffer);
    if (imageVk.isWritable())
    {
        // We need an execution barrier if image can be written to by kernel
        ANGLE_TRY(insertBarrier());
    }

    VkMemoryBarrier memBarrier = {};
    memBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memBarrier.srcAccessMask   = VK_ACCESS_MEMORY_WRITE_BIT;
    memBarrier.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    if (direction == ImageBufferCopyDirection::ToBuffer)
    {
        commandBuffer->copyImageToBuffer(imageVk.getImage().getImage(),
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         buffer.getBuffer().getHandle(), 1, &copyRegion);

        mComputePassCommands->getCommandBuffer().pipelineBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memBarrier, 0,
            nullptr, 0, nullptr);
    }
    else
    {
        commandBuffer->copyBufferToImage(buffer.getBuffer().getHandle(),
                                         imageVk.getImage().getImage(),
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        mComputePassCommands->getCommandBuffer().pipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memBarrier,
            0, nullptr, 0, nullptr);
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::addToHostTransferList(CLBufferVk *srcBuffer,
                                                      HostTransferConfig transferConfig)
{
    // TODO(aannestrand): Flush here if we reach some max-transfer-buffer heuristic
    // http://anglebug.com/377545840

    cl::Memory *transferBufferHandle =
        cl::Buffer::Cast(this->mContext->getFrontendObject().createBuffer(
            nullptr, cl::MemFlags{CL_MEM_READ_WRITE}, srcBuffer->getSize(), nullptr));
    if (transferBufferHandle == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }
    HostTransferEntry transferEntry{transferConfig, cl::MemoryPtr{transferBufferHandle}};
    mCommandsStateMap[mComputePassCommands->getQueueSerial()].hostTransferList.emplace_back(
        transferEntry);

    // Release initialization reference, lifetime controlled by RefPointer.
    transferBufferHandle->release();

    // We need an execution barrier if buffer can be written to by kernel
    if (!mComputePassCommands->getCommandBuffer().empty() && srcBuffer->isWritable())
    {
        // TODO(aannestrand): Look into combining these kernel execution barriers
        // http://anglebug.com/377545840
        VkMemoryBarrier memoryBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                         VK_ACCESS_SHADER_WRITE_BIT,
                                         VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
        mComputePassCommands->getCommandBuffer().pipelineBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
            &memoryBarrier, 0, nullptr, 0, nullptr);
    }

    // Enqueue blit/transfer cmd
    VkPipelineStageFlags srcStageMask  = {};
    VkPipelineStageFlags dstStageMask  = {};
    VkMemoryBarrier memBarrier         = {};
    memBarrier.sType                   = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    CLBufferVk &transferBufferHandleVk = transferBufferHandle->getImpl<CLBufferVk>();
    switch (transferConfig.type)
    {
        case CL_COMMAND_WRITE_BUFFER:
        {
            VkBufferCopy copyRegion = {transferConfig.offset, transferConfig.offset,
                                       transferConfig.size};
            ANGLE_TRY(transferBufferHandleVk.copyFrom(transferConfig.srcHostPtr,
                                                      transferConfig.offset, transferConfig.size));
            copyRegion.srcOffset += transferBufferHandleVk.getOffset();
            copyRegion.dstOffset += srcBuffer->getOffset();
            mComputePassCommands->getCommandBuffer().copyBuffer(
                transferBufferHandleVk.getBuffer().getBuffer(), srcBuffer->getBuffer().getBuffer(),
                1, &copyRegion);

            srcStageMask             = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStageMask             = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        }
        case CL_COMMAND_WRITE_BUFFER_RECT:
        {
            ANGLE_TRY(transferBufferHandleVk.setRect(
                transferConfig.srcHostPtr, transferConfig.srcRect, transferConfig.dstRect));
            for (VkBufferCopy &copyRegion :
                 transferBufferHandleVk.rectCopyRegions(transferConfig.dstRect))
            {
                copyRegion.srcOffset += transferBufferHandleVk.getOffset();
                copyRegion.dstOffset += srcBuffer->getOffset();
                mComputePassCommands->getCommandBuffer().copyBuffer(
                    transferBufferHandleVk.getBuffer().getBuffer(),
                    srcBuffer->getBuffer().getBuffer(), 1, &copyRegion);
            }

            // Config transfer barrier
            srcStageMask             = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStageMask             = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        }
        case CL_COMMAND_READ_BUFFER:
        {
            VkBufferCopy copyRegion = {transferConfig.offset, transferConfig.offset,
                                       transferConfig.size};
            copyRegion.srcOffset += srcBuffer->getOffset();
            copyRegion.dstOffset += transferBufferHandleVk.getOffset();
            mComputePassCommands->getCommandBuffer().copyBuffer(
                srcBuffer->getBuffer().getBuffer(), transferBufferHandleVk.getBuffer().getBuffer(),
                1, &copyRegion);

            srcStageMask             = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dstStageMask             = VK_PIPELINE_STAGE_HOST_BIT;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        }
        case CL_COMMAND_READ_BUFFER_RECT:
        {
            for (VkBufferCopy &copyRegion :
                 transferBufferHandleVk.rectCopyRegions(transferConfig.srcRect))
            {
                copyRegion.srcOffset += srcBuffer->getOffset();
                copyRegion.dstOffset += transferBufferHandleVk.getOffset();
                mComputePassCommands->getCommandBuffer().copyBuffer(
                    srcBuffer->getBuffer().getBuffer(),
                    transferBufferHandleVk.getBuffer().getBuffer(), 1, &copyRegion);
            }

            // Config transfer barrier
            srcStageMask             = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dstStageMask             = VK_PIPELINE_STAGE_HOST_BIT;
            memBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }

    // TODO(aannestrand): Look into combining these transfer barriers
    // http://anglebug.com/377545840
    mComputePassCommands->getCommandBuffer().pipelineBarrier(srcStageMask, dstStageMask, 0, 1,
                                                             &memBarrier, 0, nullptr, 0, nullptr);

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::addToHostTransferList(CLImageVk *srcImage,
                                                      HostTransferConfig transferConfig)
{
    // TODO(aannestrand): Flush here if we reach some max-transfer-buffer heuristic
    // http://anglebug.com/377545840
    CommandsState &commandsState = mCommandsStateMap[mComputePassCommands->getQueueSerial()];

    cl::Memory *transferBufferHandle =
        cl::Buffer::Cast(this->mContext->getFrontendObject().createBuffer(
            nullptr, cl::MemFlags{CL_MEM_READ_WRITE}, srcImage->getSize(), nullptr));
    if (transferBufferHandle == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    HostTransferEntry transferEntry{transferConfig, cl::MemoryPtr{transferBufferHandle}};
    commandsState.hostTransferList.emplace_back(transferEntry);

    // Release initialization reference, lifetime controlled by RefPointer.
    transferBufferHandle->release();

    // Enqueue blit
    CLBufferVk &transferBufferHandleVk = transferBufferHandle->getImpl<CLBufferVk>();
    ANGLE_TRY(copyImageToFromBuffer(*srcImage, transferBufferHandleVk.getBuffer(),
                                    transferConfig.origin, transferConfig.region, 0,
                                    ImageBufferCopyDirection::ToBuffer));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueReadImage(const cl::Image &image,
                                                 bool blocking,
                                                 const cl::MemOffsets &origin,
                                                 const cl::Coordinate &region,
                                                 size_t rowPitch,
                                                 size_t slicePitch,
                                                 void *ptr,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    CLImageVk &imageVk = image.getImpl<CLImageVk>();
    size_t size        = (region.x * region.y * region.z * imageVk.getElementSize());

    ANGLE_TRY(processWaitlist(waitEvents));

    if (imageVk.isStagingBufferInitialized() == false)
    {
        ANGLE_TRY(imageVk.createStagingBuffer(imageVk.getSize()));
    }

    if (blocking)
    {
        ANGLE_TRY(copyImageToFromBuffer(imageVk, imageVk.getStagingBuffer(), origin, region, 0,
                                        ImageBufferCopyDirection::ToBuffer));
        ANGLE_TRY(finishInternal());
        if (rowPitch == 0 && slicePitch == 0)
        {
            ANGLE_TRY(imageVk.copyStagingTo(ptr, 0, size));
        }
        else
        {
            ANGLE_TRY(imageVk.copyStagingToFromWithPitch(ptr, region, rowPitch, slicePitch,
                                                         StagingBufferCopyDirection::ToHost));
        }
        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));
    }
    else
    {
        // Create a transfer buffer and push it in update list
        HostTransferConfig transferConfig;
        transferConfig.type        = CL_COMMAND_READ_IMAGE;
        transferConfig.size        = size;
        transferConfig.dstHostPtr  = ptr;
        transferConfig.origin      = origin;
        transferConfig.region      = region;
        transferConfig.rowPitch    = rowPitch;
        transferConfig.slicePitch  = slicePitch;
        transferConfig.elementSize = imageVk.getElementSize();
        ANGLE_TRY(addToHostTransferList(&imageVk, transferConfig));

        ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueWriteImage(const cl::Image &image,
                                                  bool blocking,
                                                  const cl::MemOffsets &origin,
                                                  const cl::Coordinate &region,
                                                  size_t inputRowPitch,
                                                  size_t inputSlicePitch,
                                                  const void *ptr,
                                                  const cl::EventPtrs &waitEvents,
                                                  CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    ANGLE_TRY(processWaitlist(waitEvents));

    CLImageVk &imageVk = image.getImpl<CLImageVk>();
    size_t size        = (region.x * region.y * region.z * imageVk.getElementSize());
    cl::ExecutionStatus eventInitialState = cl::ExecutionStatus::Queued;
    if (imageVk.isStagingBufferInitialized() == false)
    {
        ANGLE_TRY(imageVk.createStagingBuffer(imageVk.getSize()));
    }

    if (inputRowPitch == 0 && inputSlicePitch == 0)
    {
        ANGLE_TRY(imageVk.copyStagingFrom((void *)ptr, 0, size));
    }
    else
    {
        ANGLE_TRY(imageVk.copyStagingToFromWithPitch((void *)ptr, region, inputRowPitch,
                                                     inputSlicePitch,
                                                     StagingBufferCopyDirection::ToStagingBuffer));
    }

    ANGLE_TRY(copyImageToFromBuffer(imageVk, imageVk.getStagingBuffer(), origin, region, 0,
                                    ImageBufferCopyDirection::ToImage));

    if (blocking)
    {
        ANGLE_TRY(finishInternal());
        eventInitialState = cl::ExecutionStatus::Complete;
    }

    ANGLE_TRY(createEvent(eventCreateFunc, eventInitialState));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueCopyImage(const cl::Image &srcImage,
                                                 const cl::Image &dstImage,
                                                 const cl::MemOffsets &srcOrigin,
                                                 const cl::MemOffsets &dstOrigin,
                                                 const cl::Coordinate &region,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    ANGLE_TRY(processWaitlist(waitEvents));

    auto srcImageVk = &srcImage.getImpl<CLImageVk>();
    auto dstImageVk = &dstImage.getImpl<CLImageVk>();

    vk::CommandBufferAccess access;
    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    VkImageAspectFlags dstAspectFlags = srcImageVk->getImage().getAspectFlags();
    VkImageAspectFlags srcAspectFlags = dstImageVk->getImage().getAspectFlags();
    access.onImageTransferWrite(gl::LevelIndex(0), 1, 0, 1, dstAspectFlags,
                                &dstImageVk->getImage());
    access.onImageTransferRead(srcAspectFlags, &srcImageVk->getImage());
    ANGLE_TRY(getCommandBuffer(access, &commandBuffer));

    VkImageCopy copyRegion    = {};
    copyRegion.extent         = cl_vk::GetExtent(srcImageVk->getExtentForCopy(region));
    copyRegion.srcOffset      = cl_vk::GetOffset(srcImageVk->getOffsetForCopy(srcOrigin));
    copyRegion.dstOffset      = cl_vk::GetOffset(dstImageVk->getOffsetForCopy(dstOrigin));
    copyRegion.srcSubresource = srcImageVk->getSubresourceLayersForCopy(
        srcOrigin, region, dstImageVk->getType(), ImageCopyWith::Image);
    copyRegion.dstSubresource = dstImageVk->getSubresourceLayersForCopy(
        dstOrigin, region, srcImageVk->getType(), ImageCopyWith::Image);
    if (srcImageVk->isWritable() || dstImageVk->isWritable())
    {
        // We need an execution barrier if buffer can be written to by kernel
        ANGLE_TRY(insertBarrier());
    }

    commandBuffer->copyImage(
        srcImageVk->getImage().getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImageVk->getImage().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueFillImage(const cl::Image &image,
                                                 const void *fillColor,
                                                 const cl::MemOffsets &origin,
                                                 const cl::Coordinate &region,
                                                 const cl::EventPtrs &waitEvents,
                                                 CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    CLImageVk &imageVk = image.getImpl<CLImageVk>();
    PixelColor packedColor;
    cl::Extents extent = imageVk.getImageExtent();

    imageVk.packPixels(fillColor, &packedColor);

    if (imageVk.isStagingBufferInitialized() == false)
    {
        ANGLE_TRY(imageVk.createStagingBuffer(imageVk.getSize()));
    }

    ANGLE_TRY(copyImageToFromBuffer(imageVk, imageVk.getStagingBuffer(), cl::kMemOffsetsZero,
                                    {extent.width, extent.height, extent.depth}, 0,
                                    ImageBufferCopyDirection::ToBuffer));
    ANGLE_TRY(finishInternal());

    uint8_t *mapPointer = nullptr;
    ANGLE_TRY(imageVk.map(mapPointer, 0));
    imageVk.fillImageWithColor(origin, region, mapPointer, &packedColor);
    imageVk.unmap();
    mapPointer = nullptr;
    ANGLE_TRY(copyImageToFromBuffer(imageVk, imageVk.getStagingBuffer(), cl::kMemOffsetsZero,
                                    {extent.width, extent.height, extent.depth}, 0,
                                    ImageBufferCopyDirection::ToImage));

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueCopyImageToBuffer(const cl::Image &srcImage,
                                                         const cl::Buffer &dstBuffer,
                                                         const cl::MemOffsets &srcOrigin,
                                                         const cl::Coordinate &region,
                                                         size_t dstOffset,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    CLImageVk &srcImageVk   = srcImage.getImpl<CLImageVk>();
    CLBufferVk &dstBufferVk = dstBuffer.getImpl<CLBufferVk>();

    ANGLE_TRY(processWaitlist(waitEvents));

    ANGLE_TRY(copyImageToFromBuffer(srcImageVk, dstBufferVk.getBuffer(), srcOrigin, region,
                                    dstOffset, ImageBufferCopyDirection::ToBuffer));

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueCopyBufferToImage(const cl::Buffer &srcBuffer,
                                                         const cl::Image &dstImage,
                                                         size_t srcOffset,
                                                         const cl::MemOffsets &dstOrigin,
                                                         const cl::Coordinate &region,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);
    CLBufferVk &srcBufferVk = srcBuffer.getImpl<CLBufferVk>();
    CLImageVk &dstImageVk   = dstImage.getImpl<CLImageVk>();

    ANGLE_TRY(processWaitlist(waitEvents));

    ANGLE_TRY(copyImageToFromBuffer(dstImageVk, srcBufferVk.getBuffer(), dstOrigin, region,
                                    srcOffset, ImageBufferCopyDirection::ToImage));

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueMapImage(const cl::Image &image,
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
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    // TODO: Look into better enqueue handling of this map-op if non-blocking
    // https://anglebug.com/376722715
    CLImageVk *imageVk = &image.getImpl<CLImageVk>();
    cl::Extents extent = imageVk->getImageExtent();
    if (blocking)
    {
        ANGLE_TRY(finishInternal());
    }

    mComputePassCommands->imageRead(mContext, imageVk->getImage().getAspectFlags(),
                                    vk::ImageLayout::TransferSrc, &imageVk->getImage());

    if (imageVk->isStagingBufferInitialized() == false)
    {
        ANGLE_TRY(imageVk->createStagingBuffer(imageVk->getSize()));
    }

    ANGLE_TRY(copyImageToFromBuffer(*imageVk, imageVk->getStagingBuffer(), cl::kMemOffsetsZero,
                                    {extent.width, extent.height, extent.depth}, 0,
                                    ImageBufferCopyDirection::ToBuffer));
    if (blocking)
    {
        ANGLE_TRY(finishInternal());
    }

    uint8_t *mapPointer = nullptr;
    size_t elementSize  = imageVk->getElementSize();
    size_t rowPitch     = (extent.width * elementSize);
    size_t offset =
        (origin.x * elementSize) + (origin.y * rowPitch) + (origin.z * extent.height * rowPitch);
    size_t size = (region.x * region.y * region.z * elementSize);

    if (image.getFlags().intersects(CL_MEM_USE_HOST_PTR))
    {
        mapPointer = static_cast<uint8_t *>(image.getHostPtr()) + offset;
        ANGLE_TRY(imageVk->copyTo(mapPointer, offset, size));
    }
    else
    {
        ANGLE_TRY(imageVk->map(mapPointer, offset));
    }
    mapPtr = static_cast<void *>(mapPointer);

    *imageRowPitch = rowPitch;

    switch (imageVk->getDescriptor().type)
    {
        case cl::MemObjectType::Image1D:
        case cl::MemObjectType::Image1D_Buffer:
        case cl::MemObjectType::Image2D:
            if (imageSlicePitch != nullptr)
            {
                *imageSlicePitch = 0;
            }
            break;
        case cl::MemObjectType::Image2D_Array:
        case cl::MemObjectType::Image3D:
            *imageSlicePitch = (extent.height * (*imageRowPitch));
            break;
        case cl::MemObjectType::Image1D_Array:
            *imageSlicePitch = *imageRowPitch;
            break;
        default:
            UNREACHABLE();
            break;
    }

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueUnmapMemObject(const cl::Memory &memory,
                                                      void *mappedPtr,
                                                      const cl::EventPtrs &waitEvents,
                                                      CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    cl::ExecutionStatus eventComplete = cl::ExecutionStatus::Queued;
    if (!eventCreateFunc)
    {
        ANGLE_TRY(finishInternal());
        eventComplete = cl::ExecutionStatus::Complete;
    }

    if (memory.getType() == cl::MemObjectType::Buffer)
    {
        CLBufferVk &bufferVk = memory.getImpl<CLBufferVk>();
        if (memory.getFlags().intersects(CL_MEM_USE_HOST_PTR))
        {
            ANGLE_TRY(finishInternal());
            ANGLE_TRY(bufferVk.copyFrom(memory.getHostPtr(), 0, bufferVk.getSize()));
            eventComplete = cl::ExecutionStatus::Complete;
        }
    }
    else if (memory.getType() != cl::MemObjectType::Pipe)
    {
        // of image type
        CLImageVk &imageVk = memory.getImpl<CLImageVk>();
        if (memory.getFlags().intersects(CL_MEM_USE_HOST_PTR))
        {
            uint8_t *mapPointer = static_cast<uint8_t *>(memory.getHostPtr());
            ANGLE_TRY(imageVk.copyStagingFrom(mapPointer, 0, imageVk.getSize()));
        }
        cl::Extents extent = imageVk.getImageExtent();
        ANGLE_TRY(copyImageToFromBuffer(imageVk, imageVk.getStagingBuffer(), cl::kMemOffsetsZero,
                                        {extent.width, extent.height, extent.depth}, 0,
                                        ImageBufferCopyDirection::ToImage));
        ANGLE_TRY(finishInternal());
        eventComplete = cl::ExecutionStatus::Complete;
    }
    else
    {
        // mem object type pipe is not supported and creation of such an object should have
        // failed
        UNREACHABLE();
    }

    memory.getImpl<CLMemoryVk>().unmap();
    ANGLE_TRY(createEvent(eventCreateFunc, eventComplete));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueMigrateMemObjects(const cl::MemoryPtrs &memObjects,
                                                         cl::MemMigrationFlags flags,
                                                         const cl::EventPtrs &waitEvents,
                                                         CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    if (mCommandQueue.getContext().getDevices().size() > 1)
    {
        // TODO(aannestrand): Later implement support to allow migration of mem objects across
        // different devices. http://anglebug.com/377942759
        UNIMPLEMENTED();
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Complete));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueNDRangeKernel(const cl::Kernel &kernel,
                                                     const cl::NDRange &ndrange,
                                                     const cl::EventPtrs &waitEvents,
                                                     CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));

    vk::PipelineCacheAccess pipelineCache;
    vk::PipelineHelper *pipelineHelper = nullptr;
    CLKernelVk &kernelImpl             = kernel.getImpl<CLKernelVk>();
    const CLProgramVk::DeviceProgramData *devProgramData =
        kernelImpl.getProgram()->getDeviceProgramData(mCommandQueue.getDevice().getNative());
    ASSERT(devProgramData != nullptr);
    cl::NDRange enqueueNDRange(ndrange);

    // Start with Workgroup size (WGS) from kernel attribute (if available)
    cl::WorkgroupSize workgroupSize =
        devProgramData->getCompiledWorkgroupSize(kernelImpl.getKernelName());
    if (workgroupSize != cl::WorkgroupSize{0, 0, 0})
    {
        // Local work size (LWS) was valid, use that as WGS
        enqueueNDRange.localWorkSize = workgroupSize;
    }
    else
    {
        if (enqueueNDRange.nullLocalWorkSize)
        {
            // NULL value was passed, in which case the OpenCL implementation will determine
            // how to be break the global work-items into appropriate work-group instances.
            enqueueNDRange.localWorkSize =
                mCommandQueue.getDevice().getImpl<CLDeviceVk>().selectWorkGroupSize(enqueueNDRange);
        }
        // At this point, we should have a non-zero Workgroup size
        ASSERT((enqueueNDRange.localWorkSize != cl::WorkgroupSize{0, 0, 0}));
    }

    // Printf storage is setup for single time usage. So drive any existing usage to completion if
    // the kernel uses printf.
    if (kernelImpl.usesPrintf() && mNeedPrintfHandling)
    {
        ANGLE_TRY(finishInternal());
    }

    // Fetch or create compute pipeline (if we miss in cache)
    ANGLE_CL_IMPL_TRY_ERROR(mContext->getRenderer()->getPipelineCache(mContext, &pipelineCache),
                            CL_OUT_OF_RESOURCES);

    ANGLE_TRY(processKernelResources(kernelImpl));
    ANGLE_TRY(processGlobalPushConstants(kernelImpl, enqueueNDRange));

    // Create uniform dispatch region(s) based on VkLimits for WorkgroupCount
    const uint32_t *maxComputeWorkGroupCount =
        mContext->getRenderer()->getPhysicalDeviceProperties().limits.maxComputeWorkGroupCount;
    for (cl::NDRange &uniformRegion : enqueueNDRange.createUniformRegions(
             {maxComputeWorkGroupCount[0], maxComputeWorkGroupCount[1],
              maxComputeWorkGroupCount[2]}))
    {
        cl::WorkgroupCount uniformRegionWorkgroupCount = uniformRegion.getWorkgroupCount();
        const VkPushConstantRange *pushConstantRegionOffset =
            devProgramData->getRegionOffsetRange();
        if (pushConstantRegionOffset != nullptr)
        {
            // The sum of the global ID offset into the NDRange for this uniform region and
            // the global offset of the NDRange
            // https://github.com/google/clspv/blob/main/docs/OpenCLCOnVulkan.md#module-scope-push-constants
            uint32_t regionOffsets[3] = {
                enqueueNDRange.globalWorkOffset[0] + uniformRegion.globalWorkOffset[0],
                enqueueNDRange.globalWorkOffset[1] + uniformRegion.globalWorkOffset[1],
                enqueueNDRange.globalWorkOffset[2] + uniformRegion.globalWorkOffset[2]};
            mComputePassCommands->getCommandBuffer().pushConstants(
                kernelImpl.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                pushConstantRegionOffset->offset, pushConstantRegionOffset->size, &regionOffsets);
        }
        const VkPushConstantRange *pushConstantRegionGroupOffset =
            devProgramData->getRegionGroupOffsetRange();
        if (pushConstantRegionGroupOffset != nullptr)
        {
            // The 3D group ID offset into the NDRange for this region
            // https://github.com/google/clspv/blob/main/docs/OpenCLCOnVulkan.md#module-scope-push-constants
            ASSERT(enqueueNDRange.localWorkSize[0] > 0 && enqueueNDRange.localWorkSize[1] > 0 &&
                   enqueueNDRange.localWorkSize[2] > 0);
            ASSERT(uniformRegion.globalWorkOffset[0] % enqueueNDRange.localWorkSize[0] == 0 &&
                   uniformRegion.globalWorkOffset[1] % enqueueNDRange.localWorkSize[1] == 0 &&
                   uniformRegion.globalWorkOffset[2] % enqueueNDRange.localWorkSize[2] == 0);
            uint32_t regionGroupOffsets[3] = {
                uniformRegion.globalWorkOffset[0] / enqueueNDRange.localWorkSize[0],
                uniformRegion.globalWorkOffset[1] / enqueueNDRange.localWorkSize[1],
                uniformRegion.globalWorkOffset[2] / enqueueNDRange.localWorkSize[2]};
            mComputePassCommands->getCommandBuffer().pushConstants(
                kernelImpl.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                pushConstantRegionGroupOffset->offset, pushConstantRegionGroupOffset->size,
                &regionGroupOffsets);
        }

        ANGLE_TRY(kernelImpl.getOrCreateComputePipeline(
            &pipelineCache, uniformRegion, mCommandQueue.getDevice(), &pipelineHelper));
        mComputePassCommands->retainResource(pipelineHelper);
        mComputePassCommands->getCommandBuffer().bindComputePipeline(pipelineHelper->getPipeline());
        mComputePassCommands->getCommandBuffer().dispatch(uniformRegionWorkgroupCount[0],
                                                          uniformRegionWorkgroupCount[1],
                                                          uniformRegionWorkgroupCount[2]);
    }

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueTask(const cl::Kernel &kernel,
                                            const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc)
{
    constexpr size_t globalWorkSize[3] = {1, 0, 0};
    constexpr size_t localWorkSize[3]  = {1, 0, 0};
    cl::NDRange ndrange(1, nullptr, globalWorkSize, localWorkSize);
    return enqueueNDRangeKernel(kernel, ndrange, waitEvents, eventCreateFunc);
}

angle::Result CLCommandQueueVk::enqueueNativeKernel(cl::UserFunc userFunc,
                                                    void *args,
                                                    size_t cbArgs,
                                                    const cl::BufferPtrs &buffers,
                                                    const std::vector<size_t> bufferPtrOffsets,
                                                    const cl::EventPtrs &waitEvents,
                                                    CLEventImpl::CreateFunc *eventCreateFunc)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLCommandQueueVk::enqueueMarkerWithWaitList(const cl::EventPtrs &waitEvents,
                                                          CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(processWaitlist(waitEvents));
    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueMarker(CLEventImpl::CreateFunc &eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    // This deprecated API is essentially a super-set of clEnqueueBarrier, where we also return
    // an event object (i.e. marker) since clEnqueueBarrier does not provide this
    ANGLE_TRY(insertBarrier());

    ANGLE_TRY(createEvent(&eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueWaitForEvents(const cl::EventPtrs &events)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    // Unlike clWaitForEvents, this routine is non-blocking
    ANGLE_TRY(processWaitlist(events));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueBarrierWithWaitList(const cl::EventPtrs &waitEvents,
                                                           CLEventImpl::CreateFunc *eventCreateFunc)
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    // The barrier command either waits for a list of events to complete, or if the list is
    // empty it waits for all commands previously enqueued in command_queue to complete before
    // it completes
    if (waitEvents.empty())
    {
        ANGLE_TRY(insertBarrier());
    }
    else
    {
        ANGLE_TRY(processWaitlist(waitEvents));
    }

    ANGLE_TRY(createEvent(eventCreateFunc, cl::ExecutionStatus::Queued));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::insertBarrier()
{
    VkMemoryBarrier memoryBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
                                     VK_ACCESS_SHADER_WRITE_BIT,
                                     VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
    mComputePassCommands->getCommandBuffer().pipelineBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
        &memoryBarrier, 0, nullptr, 0, nullptr);

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::enqueueBarrier()
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRY(insertBarrier());

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::flush()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CLCommandQueueVk::flush");

    QueueSerial lastSubmittedQueueSerial;
    {
        std::unique_lock<std::mutex> ul(mCommandQueueMutex);

        ANGLE_TRY(flushInternal());
        lastSubmittedQueueSerial = mLastSubmittedQueueSerial;
    }

    return mFinishHandler.notify(lastSubmittedQueueSerial);
}

angle::Result CLCommandQueueVk::finish()
{
    std::scoped_lock<std::mutex> sl(mCommandQueueMutex);

    ANGLE_TRACE_EVENT0("gpu.angle", "CLCommandQueueVk::finish");

    // Blocking finish
    return finishInternal();
}

angle::Result CLCommandQueueVk::syncHostBuffers(HostTransferEntries &hostTransferList)
{
    if (!hostTransferList.empty())
    {
        for (const HostTransferEntry &hostTransferEntry : hostTransferList)
        {
            const HostTransferConfig &transferConfig = hostTransferEntry.transferConfig;
            CLBufferVk &transferBufferVk =
                hostTransferEntry.transferBufferHandle->getImpl<CLBufferVk>();
            switch (hostTransferEntry.transferConfig.type)
            {
                case CL_COMMAND_WRITE_BUFFER:
                case CL_COMMAND_WRITE_BUFFER_RECT:
                    // Nothing left to do here
                    break;
                case CL_COMMAND_READ_BUFFER:
                case CL_COMMAND_READ_IMAGE:
                    if (transferConfig.rowPitch == 0 && transferConfig.slicePitch == 0)
                    {
                        ANGLE_TRY(transferBufferVk.copyTo(
                            transferConfig.dstHostPtr, transferConfig.offset, transferConfig.size));
                    }
                    else
                    {
                        ANGLE_TRY(transferBufferVk.copyToWithPitch(
                            transferConfig.dstHostPtr, transferConfig.offset, transferConfig.size,
                            transferConfig.rowPitch, transferConfig.slicePitch,
                            transferConfig.region, transferConfig.elementSize));
                    }
                    break;
                case CL_COMMAND_READ_BUFFER_RECT:
                    ANGLE_TRY(transferBufferVk.getRect(
                        transferConfig.srcRect, transferConfig.dstRect, transferConfig.dstHostPtr));
                    break;
                default:
                    UNIMPLEMENTED();
                    break;
            }
        }
    }
    hostTransferList.clear();

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::addMemoryDependencies(cl::Memory *clMem)
{
    cl::Memory *parentMem = clMem->getParent() ? clMem->getParent().get() : nullptr;

    // Take an usage count
    mCommandsStateMap[mComputePassCommands->getQueueSerial()].memories.emplace_back(clMem);

    // Handle possible resource RAW hazard
    bool insertBarrier = false;
    if (clMem->getFlags().intersects(CL_MEM_READ_WRITE))
    {
        // Texel buffers have backing buffer objects
        if (mDependencyTracker.contains(clMem) || mDependencyTracker.contains(parentMem) ||
            mDependencyTracker.size() == kMaxDependencyTrackerSize)
        {
            insertBarrier = true;
            mDependencyTracker.clear();
        }
        mDependencyTracker.insert(clMem);
        if (parentMem)
        {
            mDependencyTracker.insert(parentMem);
        }
    }

    // Insert a layout transition for images
    if (cl::IsImageType(clMem->getType()))
    {
        CLImageVk &vkMem = clMem->getImpl<CLImageVk>();
        mComputePassCommands->imageWrite(mContext, gl::LevelIndex(0), 0, 1,
                                         vkMem.getImage().getAspectFlags(),
                                         vk::ImageLayout::ComputeShaderWrite, &vkMem.getImage());
    }
    else if (insertBarrier && cl::IsBufferType(clMem->getType()))
    {
        CLBufferVk &vkMem = clMem->getImpl<CLBufferVk>();

        mComputePassCommands->bufferWrite(mContext, VK_ACCESS_SHADER_WRITE_BIT,
                                          vk::PipelineStage::ComputeShader, &vkMem.getBuffer());
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::processKernelResources(CLKernelVk &kernelVk)
{
    bool podBufferPresent              = false;
    uint32_t podBinding                = 0;
    VkDescriptorType podDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bool needsBarrier = false;
    const CLProgramVk::DeviceProgramData *devProgramData =
        kernelVk.getProgram()->getDeviceProgramData(mCommandQueue.getDevice().getNative());
    ASSERT(devProgramData != nullptr);

    // Set the descriptor set layouts and allocate descriptor sets
    // The descriptor set layouts are setup in the order of their appearance, as Vulkan requires
    // them to point to valid handles.
    angle::EnumIterator<DescriptorSetIndex> layoutIndex(DescriptorSetIndex::LiteralSampler);
    for (DescriptorSetIndex index : angle::AllEnums<DescriptorSetIndex>())
    {
        if (!kernelVk.getDescriptorSetLayoutDesc(index).empty())
        {
            // Setup the descriptor layout
            ANGLE_CL_IMPL_TRY_ERROR(mContext->getDescriptorSetLayoutCache()->getDescriptorSetLayout(
                                        mContext, kernelVk.getDescriptorSetLayoutDesc(index),
                                        &kernelVk.getDescriptorSetLayouts()[*layoutIndex]),
                                    CL_INVALID_OPERATION);
            ASSERT(kernelVk.getDescriptorSetLayouts()[*layoutIndex]->valid());

            // Allocate descriptor set
            ANGLE_TRY(mContext->allocateDescriptorSet(&kernelVk, index, layoutIndex,
                                                      mComputePassCommands));
            ++layoutIndex;
        }
    }

    // Setup the pipeline layout
    ANGLE_CL_IMPL_TRY_ERROR(kernelVk.initPipelineLayout(), CL_INVALID_OPERATION);

    // Retain kernel object until we finish executing it later
    mCommandsStateMap[mComputePassCommands->getQueueSerial()].kernels.emplace_back(
        &kernelVk.getFrontendObject());

    // Process each kernel argument/resource
    vk::DescriptorSetArray<UpdateDescriptorSetsBuilder> updateDescriptorSetsBuilders;
    CLKernelArguments args = kernelVk.getArgs();
    UpdateDescriptorSetsBuilder &kernelArgDescSetBuilder =
        updateDescriptorSetsBuilders[DescriptorSetIndex::KernelArguments];
    for (size_t index = 0; index < args.size(); index++)
    {
        const auto &arg = args.at(index);
        switch (arg.type)
        {
            case NonSemanticClspvReflectionArgumentUniform:
            case NonSemanticClspvReflectionArgumentStorageBuffer:
            {
                cl::Memory *clMem = cl::Buffer::Cast(static_cast<const cl_mem>(arg.handle));
                CLBufferVk &vkMem = clMem->getImpl<CLBufferVk>();

                ANGLE_TRY(addMemoryDependencies(clMem));

                // Update buffer/descriptor info
                VkDescriptorBufferInfo &bufferInfo =
                    kernelArgDescSetBuilder.allocDescriptorBufferInfo();
                bufferInfo.range  = clMem->getSize();
                bufferInfo.offset = clMem->getOffset();
                bufferInfo.buffer = vkMem.getBuffer().getBuffer().getHandle();
                VkWriteDescriptorSet &writeDescriptorSet =
                    kernelArgDescSetBuilder.allocWriteDescriptorSet();
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType =
                    arg.type == NonSemanticClspvReflectionArgumentUniform
                        ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                        : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writeDescriptorSet.pBufferInfo = &bufferInfo;
                writeDescriptorSet.sType       = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet =
                    kernelVk.getDescriptorSet(DescriptorSetIndex::KernelArguments);
                writeDescriptorSet.dstBinding = arg.descriptorBinding;
                break;
            }
            case NonSemanticClspvReflectionArgumentPodPushConstant:
            {
                ASSERT(!podBufferPresent);

                // Spec requires the size and offset to be multiple of 4, round up for size and
                // round down for offset to ensure this
                uint32_t offset = roundDownPow2(arg.pushConstOffset, 4u);
                uint32_t size =
                    roundUpPow2(arg.pushConstOffset + arg.pushConstantSize, 4u) - offset;
                ASSERT(offset + size <= kernelVk.getPodArgumentPushConstantsData().size());
                mComputePassCommands->getCommandBuffer().pushConstants(
                    kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, offset, size,
                    &kernelVk.getPodArgumentPushConstantsData()[offset]);
                break;
            }
            case NonSemanticClspvReflectionArgumentWorkgroup:
            {
                // Nothing to do here (this is already taken care of during clSetKernelArg)
                break;
            }
            case NonSemanticClspvReflectionArgumentSampler:
            {
                cl::Sampler *clSampler =
                    cl::Sampler::Cast(*static_cast<const cl_sampler *>(arg.handle));
                CLSamplerVk &vkSampler = clSampler->getImpl<CLSamplerVk>();
                VkDescriptorImageInfo &samplerInfo =
                    kernelArgDescSetBuilder.allocDescriptorImageInfo();
                samplerInfo.sampler = vkSampler.getSamplerHelper().get().getHandle();
                VkWriteDescriptorSet &writeDescriptorSet =
                    kernelArgDescSetBuilder.allocWriteDescriptorSet();
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
                writeDescriptorSet.pImageInfo      = &samplerInfo;
                writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet =
                    kernelVk.getDescriptorSet(DescriptorSetIndex::KernelArguments);
                writeDescriptorSet.dstBinding = arg.descriptorBinding;

                const VkPushConstantRange *samplerMaskRange =
                    devProgramData->getNormalizedSamplerMaskRange(index);
                if (samplerMaskRange != nullptr)
                {
                    if (clSampler->getNormalizedCoords() == false)
                    {
                        ANGLE_TRY(vkSampler.createNormalized());
                        samplerInfo.sampler =
                            vkSampler.getSamplerHelperNormalized().get().getHandle();
                    }
                    uint32_t mask = vkSampler.getSamplerMask();
                    mComputePassCommands->getCommandBuffer().pushConstants(
                        kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                        samplerMaskRange->offset, samplerMaskRange->size, &mask);
                }
                break;
            }
            case NonSemanticClspvReflectionArgumentStorageImage:
            case NonSemanticClspvReflectionArgumentSampledImage:
            {
                cl::Memory *clMem = cl::Image::Cast(static_cast<const cl_mem>(arg.handle));
                CLImageVk &vkMem  = clMem->getImpl<CLImageVk>();

                ANGLE_TRY(addMemoryDependencies(clMem));

                cl_image_format imageFormat = vkMem.getFormat();
                const VkPushConstantRange *imageDataChannelOrderRange =
                    devProgramData->getImageDataChannelOrderRange(index);
                if (imageDataChannelOrderRange != nullptr)
                {
                    mComputePassCommands->getCommandBuffer().pushConstants(
                        kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                        imageDataChannelOrderRange->offset, imageDataChannelOrderRange->size,
                        &imageFormat.image_channel_order);
                }

                const VkPushConstantRange *imageDataChannelDataTypeRange =
                    devProgramData->getImageDataChannelDataTypeRange(index);
                if (imageDataChannelDataTypeRange != nullptr)
                {
                    mComputePassCommands->getCommandBuffer().pushConstants(
                        kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                        imageDataChannelDataTypeRange->offset, imageDataChannelDataTypeRange->size,
                        &imageFormat.image_channel_data_type);
                }

                // Update image/descriptor info
                VkDescriptorImageInfo &imageInfo =
                    kernelArgDescSetBuilder.allocDescriptorImageInfo();
                imageInfo.imageLayout =
                    arg.type == NonSemanticClspvReflectionArgumentStorageImage
                        ? VK_IMAGE_LAYOUT_GENERAL
                        : vkMem.getImage().getCurrentLayout(mContext->getRenderer());
                imageInfo.imageView = vkMem.getImageView().getHandle();
                imageInfo.sampler   = VK_NULL_HANDLE;
                VkWriteDescriptorSet &writeDescriptorSet =
                    kernelArgDescSetBuilder.allocWriteDescriptorSet();
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType =
                    arg.type == NonSemanticClspvReflectionArgumentStorageImage
                        ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                        : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writeDescriptorSet.pImageInfo = &imageInfo;
                writeDescriptorSet.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet =
                    kernelVk.getDescriptorSet(DescriptorSetIndex::KernelArguments);
                writeDescriptorSet.dstBinding = arg.descriptorBinding;
                break;
            }
            case NonSemanticClspvReflectionArgumentUniformTexelBuffer:
            case NonSemanticClspvReflectionArgumentStorageTexelBuffer:
            {
                cl::Memory *clMem = cl::Image::Cast(static_cast<const cl_mem>(arg.handle));
                CLImageVk &vkMem  = clMem->getImpl<CLImageVk>();

                ANGLE_TRY(addMemoryDependencies(clMem));

                VkBufferView &bufferView           = kernelArgDescSetBuilder.allocBufferView();
                const vk::BufferView *vkBufferView = nullptr;
                ANGLE_TRY(vkMem.getBufferView(&vkBufferView));
                bufferView = vkBufferView->getHandle();

                VkWriteDescriptorSet &writeDescriptorSet =
                    kernelArgDescSetBuilder.allocWriteDescriptorSet();
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType =
                    arg.type == NonSemanticClspvReflectionArgumentStorageTexelBuffer
                        ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                        : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                writeDescriptorSet.pImageInfo = nullptr;
                writeDescriptorSet.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet =
                    kernelVk.getDescriptorSet(DescriptorSetIndex::KernelArguments);
                writeDescriptorSet.dstBinding       = arg.descriptorBinding;
                writeDescriptorSet.pTexelBufferView = &bufferView;

                break;
            }
            case NonSemanticClspvReflectionArgumentPodUniform:
            case NonSemanticClspvReflectionArgumentPodStorageBuffer:
            {
                if (!podBufferPresent)
                {
                    podBufferPresent  = true;
                    podBinding        = arg.descriptorBinding;
                    podDescriptorType = arg.type == NonSemanticClspvReflectionArgumentPodUniform
                                            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                                            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                break;
            }
            case NonSemanticClspvReflectionArgumentPointerUniform:
            case NonSemanticClspvReflectionArgumentPointerPushConstant:
            default:
            {
                UNIMPLEMENTED();
                break;
            }
        }
    }
    if (podBufferPresent)
    {
        cl::MemoryPtr clMem = kernelVk.getPodBuffer();
        ASSERT(clMem != nullptr);
        CLBufferVk &vkMem = clMem->getImpl<CLBufferVk>();

        VkDescriptorBufferInfo &bufferInfo = kernelArgDescSetBuilder.allocDescriptorBufferInfo();
        bufferInfo.range                   = clMem->getSize();
        bufferInfo.offset                  = clMem->getOffset();
        bufferInfo.buffer                  = vkMem.getBuffer().getBuffer().getHandle();

        ANGLE_TRY(addMemoryDependencies(clMem.get()));

        VkWriteDescriptorSet &writeDescriptorSet =
            kernelArgDescSetBuilder.allocWriteDescriptorSet();
        writeDescriptorSet.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext  = nullptr;
        writeDescriptorSet.dstSet = kernelVk.getDescriptorSet(DescriptorSetIndex::KernelArguments);
        writeDescriptorSet.dstBinding      = podBinding;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = podDescriptorType;
        writeDescriptorSet.pImageInfo      = nullptr;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
    }

    // process the printf storage buffer
    if (kernelVk.usesPrintf())
    {
        UpdateDescriptorSetsBuilder &printfDescSetBuilder =
            updateDescriptorSetsBuilders[DescriptorSetIndex::Printf];

        cl::MemoryPtr clMem = getOrCreatePrintfBuffer();
        CLBufferVk &vkMem   = clMem->getImpl<CLBufferVk>();
        uint8_t *mapPointer = nullptr;
        ANGLE_TRY(vkMem.map(mapPointer, 0));
        // The spec calls out *The first 4 bytes of the buffer should be zero-initialized.*
        memset(mapPointer, 0, 4);

        auto &bufferInfo  = printfDescSetBuilder.allocDescriptorBufferInfo();
        bufferInfo.range  = clMem->getSize();
        bufferInfo.offset = clMem->getOffset();
        bufferInfo.buffer = vkMem.getBuffer().getBuffer().getHandle();

        auto &writeDescriptorSet           = printfDescSetBuilder.allocWriteDescriptorSet();
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = kernelVk.getDescriptorSet(DescriptorSetIndex::Printf);
        writeDescriptorSet.dstBinding      = kernelVk.getProgram()
                                            ->getDeviceProgramData(kernelVk.getKernelName().c_str())
                                            ->reflectionData.printfBufferStorage.binding;

        mNeedPrintfHandling = true;
        mPrintfInfos        = kernelVk.getProgram()->getPrintfDescriptors(kernelVk.getKernelName());
    }

    angle::EnumIterator<DescriptorSetIndex> descriptorSetIndex(DescriptorSetIndex::LiteralSampler);
    for (DescriptorSetIndex index : angle::AllEnums<DescriptorSetIndex>())
    {
        if (!kernelVk.getDescriptorSetLayoutDesc(index).empty())
        {
            mContext->getPerfCounters().writeDescriptorSets =
                updateDescriptorSetsBuilders[index].flushDescriptorSetUpdates(
                    mContext->getRenderer()->getDevice());

            VkDescriptorSet descriptorSet = kernelVk.getDescriptorSet(index);
            mComputePassCommands->getCommandBuffer().bindDescriptorSets(
                kernelVk.getPipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE, *descriptorSetIndex,
                1, &descriptorSet, 0, nullptr);

            ++descriptorSetIndex;
        }
    }

    if (needsBarrier)
    {
        ANGLE_TRY(insertBarrier());
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::processGlobalPushConstants(CLKernelVk &kernelVk,
                                                           const cl::NDRange &ndrange)
{
    const CLProgramVk::DeviceProgramData *devProgramData =
        kernelVk.getProgram()->getDeviceProgramData(mCommandQueue.getDevice().getNative());
    ASSERT(devProgramData != nullptr);

    const VkPushConstantRange *globalOffsetRange = devProgramData->getGlobalOffsetRange();
    if (globalOffsetRange != nullptr)
    {
        mComputePassCommands->getCommandBuffer().pushConstants(
            kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, globalOffsetRange->offset,
            globalOffsetRange->size, ndrange.globalWorkOffset.data());
    }

    const VkPushConstantRange *globalSizeRange = devProgramData->getGlobalSizeRange();
    if (globalSizeRange != nullptr)
    {
        mComputePassCommands->getCommandBuffer().pushConstants(
            kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, globalSizeRange->offset,
            globalSizeRange->size, ndrange.globalWorkSize.data());
    }

    const VkPushConstantRange *enqueuedLocalSizeRange = devProgramData->getEnqueuedLocalSizeRange();
    if (enqueuedLocalSizeRange != nullptr)
    {
        mComputePassCommands->getCommandBuffer().pushConstants(
            kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
            enqueuedLocalSizeRange->offset, enqueuedLocalSizeRange->size,
            ndrange.localWorkSize.data());
    }

    const VkPushConstantRange *numWorkgroupsRange = devProgramData->getNumWorkgroupsRange();
    if (devProgramData->reflectionData.pushConstants.contains(
            NonSemanticClspvReflectionPushConstantNumWorkgroups))
    {
        // We support non-uniform workgroups, thus take the ceil of the quotient
        uint32_t numWorkgroups[3] = {
            UnsignedCeilDivide(ndrange.globalWorkSize[0], ndrange.localWorkSize[0]),
            UnsignedCeilDivide(ndrange.globalWorkSize[1], ndrange.localWorkSize[1]),
            UnsignedCeilDivide(ndrange.globalWorkSize[2], ndrange.localWorkSize[2])};
        mComputePassCommands->getCommandBuffer().pushConstants(
            kernelVk.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, numWorkgroupsRange->offset,
            numWorkgroupsRange->size, &numWorkgroups);
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::flushComputePassCommands()
{
    if (mComputePassCommands->empty())
    {
        return angle::Result::Continue;
    }

    // Flush any host visible buffers by adding appropriate barriers
    if (mComputePassCommands->getAndResetHasHostVisibleBufferWrite())
    {
        // Make sure all writes to host-visible buffers are flushed.
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask   = VK_ACCESS_MEMORY_WRITE_BIT;
        memoryBarrier.dstAccessMask   = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;

        mComputePassCommands->getCommandBuffer().memoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT, memoryBarrier);
    }

    // get hold of the queue serial that is flushed, post the flush the command buffer will be reset
    mLastFlushedQueueSerial = mComputePassCommands->getQueueSerial();
    // Here, we flush our compute cmds to RendererVk's primary command buffer
    ANGLE_TRY(mContext->getRenderer()->flushOutsideRPCommands(
        mContext, getProtectionType(), egl::ContextPriority::Medium, &mComputePassCommands));

    mContext->getPerfCounters().flushedOutsideRenderPassCommandBuffers++;

    // Generate new serial for next batch of cmds
    mComputePassCommands->setQueueSerial(
        mQueueSerialIndex, mContext->getRenderer()->generateQueueSerial(mQueueSerialIndex));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::processWaitlist(const cl::EventPtrs &waitEvents)
{
    if (!waitEvents.empty())
    {
        bool insertedBarrier = false;
        for (const cl::EventPtr &event : waitEvents)
        {
            if (event->getImpl<CLEventVk>().isUserEvent() ||
                event->getCommandQueue() != &mCommandQueue)
            {
                // We cannot use a barrier in these cases, therefore defer the event
                // handling till submission time
                // TODO: Perhaps we could utilize VkEvents here instead and have GPU wait(s)
                // https://anglebug.com/42267109
                mExternalEvents.push_back(event);
            }
            else if (event->getCommandQueue() == &mCommandQueue && !insertedBarrier)
            {
                // As long as there is at least one dependant command in same queue,
                // we just need to insert one execution barrier
                ANGLE_TRY(insertBarrier());

                insertedBarrier = true;
            }
        }
    }
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::submitCommands()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CLCommandQueueVk::submitCommands()");

    ASSERT(hasCommandsPendingSubmission());

    // Kick off renderer submit
    ANGLE_TRY(mContext->getRenderer()->submitCommands(mContext, getProtectionType(),
                                                      egl::ContextPriority::Medium, nullptr,
                                                      nullptr, {}, mLastFlushedQueueSerial));

    mLastSubmittedQueueSerial = mLastFlushedQueueSerial;

    // Now that we have submitted commands, some of pending garbage may no longer pending
    // and should be moved to garbage list.
    mContext->getRenderer()->cleanupPendingSubmissionGarbage();

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::createEvent(CLEventImpl::CreateFunc *createFunc,
                                            cl::ExecutionStatus initialStatus)
{
    if (createFunc != nullptr)
    {
        *createFunc = [this, initialStatus, queueSerial = mComputePassCommands->getQueueSerial()](
                          const cl::Event &event) {
            auto eventVk = new (std::nothrow) CLEventVk(event);
            if (eventVk == nullptr)
            {
                ERR() << "Failed to create event obj!";
                ANGLE_CL_SET_ERROR(CL_OUT_OF_HOST_MEMORY);
                return CLEventImpl::Ptr(nullptr);
            }

            if (initialStatus == cl::ExecutionStatus::Complete)
            {
                // Submission finished at this point, just set event to complete
                if (IsError(eventVk->setStatusAndExecuteCallback(cl::ToCLenum(initialStatus))))
                {
                    ANGLE_CL_SET_ERROR(CL_OUT_OF_RESOURCES);
                }
            }
            else if (mCommandQueue.getProperties().intersects(CL_QUEUE_PROFILING_ENABLE))
            {
                // We also block for profiling so that we get timestamps per-command
                if (IsError(mCommandQueue.getImpl<CLCommandQueueVk>().finish()))
                {
                    ANGLE_CL_SET_ERROR(CL_OUT_OF_RESOURCES);
                }
                // Submission finished at this point, just set event to complete
                if (IsError(eventVk->setStatusAndExecuteCallback(CL_COMPLETE)))
                {
                    ANGLE_CL_SET_ERROR(CL_OUT_OF_RESOURCES);
                }
            }
            else
            {
                eventVk->setQueueSerial(queueSerial);
                // Save a reference to this event to set event transitions
                std::lock_guard<std::mutex> lock(mCommandQueueMutex);
                mCommandsStateMap[queueSerial].events.emplace_back(&eventVk->getFrontendObject());
            }

            return CLEventImpl::Ptr(eventVk);
        };
    }
    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::submitEmptyCommand()
{
    // This will be called as part of resetting the command buffer and command buffer has to be
    // empty.
    ASSERT(mComputePassCommands->empty());

    // There is nothing to be flushed, mark it flushed and do a submit to signal the queue serial
    mLastFlushedQueueSerial = mComputePassCommands->getQueueSerial();
    ANGLE_TRY(submitCommands());
    ANGLE_TRY(finishQueueSerialInternal(mLastSubmittedQueueSerial));

    // increment the queue serial for the next command batch
    mComputePassCommands->setQueueSerial(
        mQueueSerialIndex, mContext->getRenderer()->generateQueueSerial(mQueueSerialIndex));

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::resetCommandBufferWithError(cl_int errorCode)
{
    // Got an error so reset the command buffer and report back error to all the associated
    // events
    ASSERT(errorCode != CL_SUCCESS);

    QueueSerial currentSerial = mComputePassCommands->getQueueSerial();
    mComputePassCommands->getCommandBuffer().reset();

    for (cl::EventPtr event : mCommandsStateMap[currentSerial].events)
    {
        CLEventVk *eventVk = &event->getImpl<CLEventVk>();
        if (!eventVk->isUserEvent())
        {
            ANGLE_TRY(
                eventVk->setStatusAndExecuteCallback(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST));
        }
    }
    mCommandsStateMap.erase(currentSerial);
    mExternalEvents.clear();

    // Command buffer has been reset and as such the associated queue serial will not get signaled
    // leading to causality issues. So submit an empty command to keep the queue serials timelines
    // intact.
    ANGLE_TRY(submitEmptyCommand());

    ANGLE_CL_RETURN_ERROR(errorCode);
}

angle::Result CLCommandQueueVk::finishQueueSerialInternal(const QueueSerial queueSerial)
{
    // Queue serial must belong to this queue and work must have been submitted.
    ASSERT(queueSerial.getIndex() == mQueueSerialIndex);
    ASSERT(mContext->getRenderer()->hasQueueSerialSubmitted(queueSerial));

    ANGLE_TRY(mContext->getRenderer()->finishQueueSerial(mContext, queueSerial));

    // Ensure memory  objects are synced back to host CPU
    ANGLE_TRY(syncHostBuffers(mCommandsStateMap[queueSerial].hostTransferList));

    if (mNeedPrintfHandling)
    {
        ANGLE_TRY(processPrintfBuffer());
        mNeedPrintfHandling = false;
    }

    // Events associated with this queue serial and ready to be marked complete
    ANGLE_TRY(SetEventsWithQueueSerialToState(mCommandsStateMap[queueSerial].events, queueSerial,
                                              cl::ExecutionStatus::Complete));

    mExternalEvents.clear();
    mCommandsStateMap.erase(queueSerial);

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::finishQueueSerial(const QueueSerial queueSerial)
{
    ASSERT(queueSerial.getIndex() == getQueueSerialIndex());
    ASSERT(mContext->getRenderer()->hasQueueSerialSubmitted(queueSerial));

    ANGLE_TRY(mContext->getRenderer()->finishQueueSerial(mContext, queueSerial));

    std::lock_guard<std::mutex> sl(mCommandQueueMutex);

    return finishQueueSerialInternal(queueSerial);
}

angle::Result CLCommandQueueVk::flushInternal()
{
    if (!mComputePassCommands->empty())
    {
        // If we still have dependant events, handle them now
        if (!mExternalEvents.empty())
        {
            for (const auto &depEvent : mExternalEvents)
            {
                if (depEvent->getImpl<CLEventVk>().isUserEvent())
                {
                    // We just wait here for user to set the event object
                    cl_int status = CL_QUEUED;
                    ANGLE_TRY(depEvent->getImpl<CLEventVk>().waitForUserEventStatus());
                    ANGLE_TRY(depEvent->getImpl<CLEventVk>().getCommandExecutionStatus(status));
                    if (status < 0)
                    {
                        ERR() << "Invalid dependant user-event (" << depEvent.get()
                              << ") status encountered!";
                        ANGLE_TRY(resetCommandBufferWithError(
                            CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST));
                    }
                }
                else
                {
                    // Otherwise, we just need to submit/finish for dependant event queues
                    // here that are not associated with this queue
                    ANGLE_TRY(depEvent->getCommandQueue()->finish());
                }
            }
            mExternalEvents.clear();
        }

        ANGLE_TRY(flushComputePassCommands());
        CommandsState commandsState = mCommandsStateMap[mLastFlushedQueueSerial];
        ANGLE_TRY(SetEventsWithQueueSerialToState(commandsState.events, mLastFlushedQueueSerial,
                                                  cl::ExecutionStatus::Submitted));

        ANGLE_TRY(submitCommands());
        ASSERT(!hasCommandsPendingSubmission());
        ANGLE_TRY(SetEventsWithQueueSerialToState(commandsState.events, mLastSubmittedQueueSerial,
                                                  cl::ExecutionStatus::Running));
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::finishInternal()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CLCommandQueueVk::finish");
    ANGLE_TRY(flushInternal());

    return finishQueueSerialInternal(mLastSubmittedQueueSerial);
}

// Helper function to insert appropriate memory barriers before accessing the resources in the
// command buffer.
angle::Result CLCommandQueueVk::onResourceAccess(const vk::CommandBufferAccess &access)
{
    // Buffers
    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getReadBuffers())
    {
        if (mComputePassCommands->usesBufferForWrite(*bufferAccess.buffer))
        {
            // read buffers only need a new command buffer if previously used for write
            ANGLE_TRY(flushInternal());
        }

        mComputePassCommands->bufferRead(mContext, bufferAccess.accessType, bufferAccess.stage,
                                         bufferAccess.buffer);
    }

    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getWriteBuffers())
    {
        if (mComputePassCommands->usesBuffer(*bufferAccess.buffer))
        {
            // write buffers always need a new command buffer
            ANGLE_TRY(flushInternal());
        }

        mComputePassCommands->bufferWrite(mContext, bufferAccess.accessType, bufferAccess.stage,
                                          bufferAccess.buffer);
        if (bufferAccess.buffer->isHostVisible())
        {
            // currently all are host visible so nothing to do
        }
    }

    for (const vk::CommandBufferBufferExternalAcquireRelease &bufferAcquireRelease :
         access.getExternalAcquireReleaseBuffers())
    {
        mComputePassCommands->retainResourceForWrite(bufferAcquireRelease.buffer);
    }

    for (const vk::CommandBufferResourceAccess &resourceAccess : access.getAccessResources())
    {
        mComputePassCommands->retainResource(resourceAccess.resource);
    }

    return angle::Result::Continue;
}

angle::Result CLCommandQueueVk::processPrintfBuffer()
{
    ASSERT(mPrintfBuffer);
    ASSERT(mNeedPrintfHandling);
    ASSERT(mPrintfInfos);

    cl::MemoryPtr clMem = getOrCreatePrintfBuffer();
    CLBufferVk &vkMem = clMem->getImpl<CLBufferVk>();

    unsigned char *data = nullptr;
    ANGLE_TRY(vkMem.map(data, 0));
    ANGLE_TRY(ClspvProcessPrintfBuffer(data, vkMem.getSize(), mPrintfInfos));
    vkMem.unmap();

    return angle::Result::Continue;
}

// A single CL buffer is setup for every command queue of size kPrintfBufferSize. This can be
// expanded later, if more storage is needed.
cl::MemoryPtr CLCommandQueueVk::getOrCreatePrintfBuffer()
{
    if (!mPrintfBuffer)
    {
        mPrintfBuffer = cl::Buffer::Cast(mContext->getFrontendObject().createBuffer(
            nullptr, cl::MemFlags(CL_MEM_READ_WRITE), kPrintfBufferSize, nullptr));
    }
    return cl::MemoryPtr(mPrintfBuffer);
}

bool CLCommandQueueVk::hasUserEventDependency() const
{
    return std::any_of(mExternalEvents.begin(), mExternalEvents.end(),
                       [](const cl::EventPtr event) { return event->isUserEvent(); });
}

}  // namespace rx
