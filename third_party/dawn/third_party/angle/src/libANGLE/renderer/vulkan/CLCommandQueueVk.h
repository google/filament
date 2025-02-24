//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueVk.h: Defines the class interface for CLCommandQueueVk,
// implementing CLCommandQueueImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLCOMMANDQUEUEVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLCOMMANDQUEUEVK_H_

#include <condition_variable>
#include <vector>

#include "common/PackedCLEnums_autogen.h"
#include "common/hash_containers.h"

#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLKernelVk.h"
#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/cl_types.h"
#include "libANGLE/renderer/vulkan/clspv_utils.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/renderer/CLCommandQueueImpl.h"
#include "libANGLE/renderer/serial_utils.h"

#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/cl_types.h"

namespace std
{
// Hash function for QueueSerial so that it can serve as a key for angle::HashMap
template <>
struct hash<rx::QueueSerial>
{
    size_t operator()(const rx::QueueSerial &queueSerial) const
    {
        size_t hash = 0;
        angle::HashCombine(hash, queueSerial.getSerial().getValue());
        angle::HashCombine(hash, queueSerial.getIndex());
        return hash;
    }
};
}  // namespace std

namespace rx
{

static constexpr size_t kPrintfBufferSize = 1024 * 1024;
class CLCommandQueueVk;

namespace
{

struct HostTransferConfig
{
    HostTransferConfig()
        : srcRect(cl::Offset{}, cl::Extents{}, 0, 0, 0), dstRect(cl::Offset{}, cl::Extents{}, 0, 0)
    {}
    cl_command_type type{0};
    size_t size            = 0;
    size_t offset          = 0;
    void *dstHostPtr       = nullptr;
    const void *srcHostPtr = nullptr;
    size_t rowPitch        = 0;
    size_t slicePitch      = 0;
    size_t elementSize     = 0;
    cl::MemOffsets origin;
    cl::Coordinate region;
    cl::BufferRect srcRect;
    cl::BufferRect dstRect;
};
struct HostTransferEntry
{
    HostTransferConfig transferConfig;
    cl::MemoryPtr transferBufferHandle;
};
using HostTransferEntries = std::vector<HostTransferEntry>;

// DispatchWorkThread setups a background thread to wait on the work submitted to Vulkan renderer.
class DispatchWorkThread
{
  public:
    DispatchWorkThread(CLCommandQueueVk *commandQueue);
    ~DispatchWorkThread();

    angle::Result init();
    void terminate();

    angle::Result notify(QueueSerial queueSerial);

  private:
    static constexpr size_t kFixedQueueLimit = 4u;

    angle::Result finishLoop();

    CLCommandQueueVk *const mCommandQueue;

    std::mutex mThreadMutex;
    std::condition_variable mHasWorkSubmitted;
    std::condition_variable mHasEmptySlot;
    bool mIsTerminating;
    std::thread mWorkerThread;

    angle::FixedQueue<QueueSerial> mQueueSerials;
    // Queue serial index associated with the CLCommandQueueVk
    SerialIndex mQueueSerialIndex;
};

struct CommandsState
{
    cl::EventPtrs events;
    cl::MemoryPtrs memories;
    cl::KernelPtrs kernels;
    HostTransferEntries hostTransferList;
};
using CommandsStateMap = angle::HashMap<QueueSerial, CommandsState>;

}  // namespace

class CLCommandQueueVk : public CLCommandQueueImpl
{
  public:
    CLCommandQueueVk(const cl::CommandQueue &commandQueue);
    ~CLCommandQueueVk() override;

    angle::Result init();

    angle::Result setProperty(cl::CommandQueueProperties properties, cl_bool enable) override;

    angle::Result enqueueReadBuffer(const cl::Buffer &buffer,
                                    bool blocking,
                                    size_t offset,
                                    size_t size,
                                    void *ptr,
                                    const cl::EventPtrs &waitEvents,
                                    CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueWriteBuffer(const cl::Buffer &buffer,
                                     bool blocking,
                                     size_t offset,
                                     size_t size,
                                     const void *ptr,
                                     const cl::EventPtrs &waitEvents,
                                     CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueReadBufferRect(const cl::Buffer &buffer,
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
                                        CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueWriteBufferRect(const cl::Buffer &buffer,
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
                                         CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueCopyBuffer(const cl::Buffer &srcBuffer,
                                    const cl::Buffer &dstBuffer,
                                    size_t srcOffset,
                                    size_t dstOffset,
                                    size_t size,
                                    const cl::EventPtrs &waitEvents,
                                    CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueCopyBufferRect(const cl::Buffer &srcBuffer,
                                        const cl::Buffer &dstBuffer,
                                        const cl::MemOffsets &srcOrigin,
                                        const cl::MemOffsets &dstOrigin,
                                        const cl::Coordinate &region,
                                        size_t srcRowPitch,
                                        size_t srcSlicePitch,
                                        size_t dstRowPitch,
                                        size_t dstSlicePitch,
                                        const cl::EventPtrs &waitEvents,
                                        CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueFillBuffer(const cl::Buffer &buffer,
                                    const void *pattern,
                                    size_t patternSize,
                                    size_t offset,
                                    size_t size,
                                    const cl::EventPtrs &waitEvents,
                                    CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueMapBuffer(const cl::Buffer &buffer,
                                   bool blocking,
                                   cl::MapFlags mapFlags,
                                   size_t offset,
                                   size_t size,
                                   const cl::EventPtrs &waitEvents,
                                   CLEventImpl::CreateFunc *eventCreateFunc,
                                   void *&mapPtr) override;

    angle::Result enqueueReadImage(const cl::Image &image,
                                   bool blocking,
                                   const cl::MemOffsets &origin,
                                   const cl::Coordinate &region,
                                   size_t rowPitch,
                                   size_t slicePitch,
                                   void *ptr,
                                   const cl::EventPtrs &waitEvents,
                                   CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueWriteImage(const cl::Image &image,
                                    bool blocking,
                                    const cl::MemOffsets &origin,
                                    const cl::Coordinate &region,
                                    size_t inputRowPitch,
                                    size_t inputSlicePitch,
                                    const void *ptr,
                                    const cl::EventPtrs &waitEvents,
                                    CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueCopyImage(const cl::Image &srcImage,
                                   const cl::Image &dstImage,
                                   const cl::MemOffsets &srcOrigin,
                                   const cl::MemOffsets &dstOrigin,
                                   const cl::Coordinate &region,
                                   const cl::EventPtrs &waitEvents,
                                   CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueFillImage(const cl::Image &image,
                                   const void *fillColor,
                                   const cl::MemOffsets &origin,
                                   const cl::Coordinate &region,
                                   const cl::EventPtrs &waitEvents,
                                   CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueCopyImageToBuffer(const cl::Image &srcImage,
                                           const cl::Buffer &dstBuffer,
                                           const cl::MemOffsets &srcOrigin,
                                           const cl::Coordinate &region,
                                           size_t dstOffset,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueCopyBufferToImage(const cl::Buffer &srcBuffer,
                                           const cl::Image &dstImage,
                                           size_t srcOffset,
                                           const cl::MemOffsets &dstOrigin,
                                           const cl::Coordinate &region,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueMapImage(const cl::Image &image,
                                  bool blocking,
                                  cl::MapFlags mapFlags,
                                  const cl::MemOffsets &origin,
                                  const cl::Coordinate &region,
                                  size_t *imageRowPitch,
                                  size_t *imageSlicePitch,
                                  const cl::EventPtrs &waitEvents,
                                  CLEventImpl::CreateFunc *eventCreateFunc,
                                  void *&mapPtr) override;

    angle::Result enqueueUnmapMemObject(const cl::Memory &memory,
                                        void *mappedPtr,
                                        const cl::EventPtrs &waitEvents,
                                        CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueMigrateMemObjects(const cl::MemoryPtrs &memObjects,
                                           cl::MemMigrationFlags flags,
                                           const cl::EventPtrs &waitEvents,
                                           CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueNDRangeKernel(const cl::Kernel &kernel,
                                       const cl::NDRange &ndrange,
                                       const cl::EventPtrs &waitEvents,
                                       CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueTask(const cl::Kernel &kernel,
                              const cl::EventPtrs &waitEvents,
                              CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueNativeKernel(cl::UserFunc userFunc,
                                      void *args,
                                      size_t cbArgs,
                                      const cl::BufferPtrs &buffers,
                                      const std::vector<size_t> bufferPtrOffsets,
                                      const cl::EventPtrs &waitEvents,
                                      CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueMarkerWithWaitList(const cl::EventPtrs &waitEvents,
                                            CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueMarker(CLEventImpl::CreateFunc &eventCreateFunc) override;

    angle::Result enqueueWaitForEvents(const cl::EventPtrs &events) override;

    angle::Result enqueueBarrierWithWaitList(const cl::EventPtrs &waitEvents,
                                             CLEventImpl::CreateFunc *eventCreateFunc) override;

    angle::Result enqueueBarrier() override;

    angle::Result flush() override;

    angle::Result finish() override;

    CLPlatformVk *getPlatform() { return mContext->getPlatform(); }
    CLContextVk *getContext() { return mContext; }

    cl::MemoryPtr getOrCreatePrintfBuffer();

    angle::Result finishQueueSerial(const QueueSerial queueSerial);

    SerialIndex getQueueSerialIndex() const { return mQueueSerialIndex; }

    bool hasCommandsPendingSubmission() const
    {
        return mLastFlushedQueueSerial != mLastSubmittedQueueSerial;
    }

  private:
    static constexpr size_t kMaxDependencyTrackerSize    = 64;
    static constexpr size_t kMaxHostBufferUpdateListSize = 16;

    angle::Result resetCommandBufferWithError(cl_int errorCode);

    vk::ProtectionType getProtectionType() const { return vk::ProtectionType::Unprotected; }

    // Create-update-bind the kernel's descriptor set, put push-constants in cmd buffer, capture
    // kernel resources, and handle kernel execution dependencies
    angle::Result processKernelResources(CLKernelVk &kernelVk);
    // Updates global push constants for a given CL kernel
    angle::Result processGlobalPushConstants(CLKernelVk &kernelVk, const cl::NDRange &ndrange);

    angle::Result submitCommands();
    angle::Result finishInternal();
    angle::Result flushInternal();
    // Wait for the submitted work to the renderer to finish and perform post-processing such as
    // event status updates etc. This is a blocking call.
    angle::Result finishQueueSerialInternal(const QueueSerial queueSerial);

    angle::Result syncHostBuffers(HostTransferEntries &hostTransferList);
    angle::Result flushComputePassCommands();
    angle::Result processWaitlist(const cl::EventPtrs &waitEvents);
    angle::Result createEvent(CLEventImpl::CreateFunc *createFunc,
                              cl::ExecutionStatus initialStatus);

    angle::Result onResourceAccess(const vk::CommandBufferAccess &access);
    angle::Result getCommandBuffer(const vk::CommandBufferAccess &access,
                                   vk::OutsideRenderPassCommandBuffer **commandBufferOut)
    {
        ANGLE_TRY(onResourceAccess(access));
        *commandBufferOut = &mComputePassCommands->getCommandBuffer();
        return angle::Result::Continue;
    }

    angle::Result processPrintfBuffer();
    angle::Result copyImageToFromBuffer(CLImageVk &imageVk,
                                        vk::BufferHelper &buffer,
                                        const cl::MemOffsets &origin,
                                        const cl::Coordinate &region,
                                        size_t bufferOffset,
                                        ImageBufferCopyDirection writeToBuffer);

    bool hasUserEventDependency() const;

    angle::Result insertBarrier();
    angle::Result addMemoryDependencies(cl::Memory *clMem);

    angle::Result submitEmptyCommand();

    CLContextVk *mContext;
    const CLDeviceVk *mDevice;
    cl::Memory *mPrintfBuffer;

    vk::SecondaryCommandPools mCommandPool;
    vk::OutsideRenderPassCommandBufferHelper *mComputePassCommands;
    vk::SecondaryCommandMemoryAllocator mOutsideRenderPassCommandsAllocator;

    // Queue Serials for this command queue
    SerialIndex mQueueSerialIndex;
    QueueSerial mLastSubmittedQueueSerial;
    QueueSerial mLastFlushedQueueSerial;

    std::mutex mCommandQueueMutex;

    // External dependent events that this queue has to wait on
    cl::EventPtrs mExternalEvents;

    // Keep track of kernel resources on prior kernel enqueues
    angle::HashSet<cl::Object *> mDependencyTracker;

    CommandsStateMap mCommandsStateMap;

    // printf handling
    bool mNeedPrintfHandling;
    const angle::HashMap<uint32_t, ClspvPrintfInfo> *mPrintfInfos;

    // Host buffer transferring routines
    angle::Result addToHostTransferList(CLBufferVk *srcBuffer, HostTransferConfig transferEntry);
    angle::Result addToHostTransferList(CLImageVk *srcImage, HostTransferConfig transferEntry);

    DispatchWorkThread mFinishHandler;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLCOMMANDQUEUEVK_H_
