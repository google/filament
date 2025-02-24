//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_renderer.h:
//    Defines the class interface for Renderer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "common/PackedEnums.h"
#include "common/SimpleMutex.h"
#include "common/WorkerThread.h"
#include "common/angleutils.h"
#include "common/vulkan/vk_headers.h"
#include "common/vulkan/vulkan_icd.h"
#include "libANGLE/Caps.h"
#include "libANGLE/renderer/vulkan/CommandQueue.h"
#include "libANGLE/renderer/vulkan/DebugAnnotatorVk.h"
#include "libANGLE/renderer/vulkan/MemoryTracking.h"
#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/renderer/vulkan/UtilsVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"
#include "libANGLE/renderer/vulkan/vk_mem_alloc_wrapper.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"

namespace angle
{
class Library;
struct FrontendFeatures;
}  // namespace angle

namespace rx
{
class FramebufferVk;

namespace vk
{
class Format;

static constexpr size_t kMaxExtensionNames = 400;
using ExtensionNameList                    = angle::FixedVector<const char *, kMaxExtensionNames>;

static constexpr size_t kMaxSyncValExtraProperties = 5;
// Information used to accurately skip known synchronization issues in ANGLE.
// TODO: remove messageContents1 and messageContents2 fields after all
// supressions have transitioned to using extraProperties.
struct SkippedSyncvalMessage
{
    const char *messageId;
    const char *messageContents1;
    const char *messageContents2                            = "";
    bool isDueToNonConformantCoherentColorFramebufferFetch  = false;
    const char *extraProperties[kMaxSyncValExtraProperties] = {};
};

class ImageMemorySuballocator : angle::NonCopyable
{
  public:
    ImageMemorySuballocator();
    ~ImageMemorySuballocator();

    void destroy(vk::Renderer *renderer);

    // Allocates memory for the image and binds it.
    VkResult allocateAndBindMemory(ErrorContext *context,
                                   Image *image,
                                   const VkImageCreateInfo *imageCreateInfo,
                                   VkMemoryPropertyFlags requiredFlags,
                                   VkMemoryPropertyFlags preferredFlags,
                                   const VkMemoryRequirements *memoryRequirements,
                                   const bool allocateDedicatedMemory,
                                   MemoryAllocationType memoryAllocationType,
                                   Allocation *allocationOut,
                                   VkMemoryPropertyFlags *memoryFlagsOut,
                                   uint32_t *memoryTypeIndexOut,
                                   VkDeviceSize *sizeOut);

    // Maps the memory to initialize with non-zero value.
    VkResult mapMemoryAndInitWithNonZeroValue(vk::Renderer *renderer,
                                              Allocation *allocation,
                                              VkDeviceSize size,
                                              int value,
                                              VkMemoryPropertyFlags flags);

    // Determines if dedicated memory is required for the allocation.
    bool needsDedicatedMemory(VkDeviceSize size) const;
};

// Supports one semaphore from current surface, and one semaphore passed to
// glSignalSemaphoreEXT.
using SignalSemaphoreVector = angle::FixedVector<VkSemaphore, 2>;

// Recursive function to process variable arguments for garbage collection
inline void CollectGarbage(std::vector<vk::GarbageObject> *garbageOut) {}
template <typename ArgT, typename... ArgsT>
void CollectGarbage(std::vector<vk::GarbageObject> *garbageOut, ArgT object, ArgsT... objectsIn)
{
    if (object->valid())
    {
        garbageOut->emplace_back(vk::GarbageObject::Get(object));
    }
    CollectGarbage(garbageOut, objectsIn...);
}

// Recursive function to process variable arguments for garbage destroy
inline void DestroyGarbage(vk::Renderer *renderer) {}

class OneOffCommandPool : angle::NonCopyable
{
  public:
    OneOffCommandPool();
    void init(vk::ProtectionType protectionType);
    angle::Result getCommandBuffer(vk::ErrorContext *context,
                                   vk::ScopedPrimaryCommandBuffer *commandBufferOut);
    void releaseCommandBuffer(const QueueSerial &submitQueueSerial,
                              vk::PrimaryCommandBuffer &&primary);
    void destroy(VkDevice device);

  private:
    vk::ProtectionType mProtectionType;
    angle::SimpleMutex mMutex;
    vk::CommandPool mCommandPool;
    struct PendingOneOffCommands
    {
        vk::ResourceUse use;
        vk::PrimaryCommandBuffer commandBuffer;
    };
    std::deque<PendingOneOffCommands> mPendingCommands;
};

enum class UseDebugLayers
{
    Yes,
    YesIfAvailable,
    No,
};

enum class UseVulkanSwapchain
{
    Yes,
    No,
};

class Renderer : angle::NonCopyable
{
  public:
    Renderer();
    ~Renderer();

    angle::Result initialize(vk::ErrorContext *context,
                             vk::GlobalOps *globalOps,
                             angle::vk::ICD desiredICD,
                             uint32_t preferredVendorId,
                             uint32_t preferredDeviceId,
                             const uint8_t *preferredDeviceUuid,
                             const uint8_t *preferredDriverUuid,
                             VkDriverId preferredDriverId,
                             UseDebugLayers useDebugLayers,
                             const char *wsiExtension,
                             const char *wsiLayer,
                             angle::NativeWindowSystem nativeWindowSystem,
                             const angle::FeatureOverrides &featureOverrides);

    // Reload volk vk* function ptrs if needed for an already initialized Renderer
    void reloadVolkIfNeeded() const;
    void onDestroy(vk::ErrorContext *context);

    void notifyDeviceLost();
    bool isDeviceLost() const;
    bool hasSharedGarbage();

    std::string getVendorString() const;
    std::string getRendererDescription() const;
    std::string getVersionString(bool includeFullVersion) const;

    gl::Version getMaxSupportedESVersion() const;
    gl::Version getMaxConformantESVersion() const;

    uint32_t getDeviceVersion() const;
    VkInstance getInstance() const { return mInstance; }
    VkPhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; }
    const VkPhysicalDeviceProperties &getPhysicalDeviceProperties() const
    {
        return mPhysicalDeviceProperties;
    }
    const VkPhysicalDeviceDrmPropertiesEXT &getPhysicalDeviceDrmProperties() const
    {
        return mDrmProperties;
    }
    const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT &
    getPhysicalDevicePrimitivesGeneratedQueryFeatures() const
    {
        return mPrimitivesGeneratedQueryFeatures;
    }
    const VkPhysicalDeviceHostImageCopyPropertiesEXT &getPhysicalDeviceHostImageCopyProperties()
        const
    {
        return mHostImageCopyProperties;
    }
    const VkPhysicalDeviceFeatures &getPhysicalDeviceFeatures() const
    {
        return mPhysicalDeviceFeatures;
    }
    const VkPhysicalDeviceFeatures2KHR &getEnabledFeatures() const { return mEnabledFeatures; }
    VkDevice getDevice() const { return mDevice; }

    const vk::Allocator &getAllocator() const { return mAllocator; }
    vk::ImageMemorySuballocator &getImageMemorySuballocator() { return mImageMemorySuballocator; }

    angle::Result checkQueueForSurfacePresent(vk::ErrorContext *context,
                                              VkSurfaceKHR surface,
                                              bool *supportedOut);

    const gl::Caps &getNativeCaps() const;
    const gl::TextureCapsMap &getNativeTextureCaps() const;
    const gl::Extensions &getNativeExtensions() const;
    const gl::Limitations &getNativeLimitations() const;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const;
    void initializeFrontendFeatures(angle::FrontendFeatures *features) const;

    uint32_t getQueueFamilyIndex() const { return mCurrentQueueFamilyIndex; }
    const VkQueueFamilyProperties &getQueueFamilyProperties() const
    {
        return mQueueFamilyProperties[mCurrentQueueFamilyIndex];
    }
    const DeviceQueueIndex getDeviceQueueIndex(egl::ContextPriority priority) const
    {
        return mCommandQueue.getDeviceQueueIndex(priority);
    }
    const DeviceQueueIndex getDefaultDeviceQueueIndex() const
    {
        // By default it will always use medium priority
        return mCommandQueue.getDeviceQueueIndex(egl::ContextPriority::Medium);
    }

    const vk::MemoryProperties &getMemoryProperties() const { return mMemoryProperties; }

    const vk::Format &getFormat(GLenum internalFormat) const
    {
        return mFormatTable[internalFormat];
    }

    const vk::Format &getFormat(angle::FormatID formatID) const { return mFormatTable[formatID]; }

    // Get the pipeline cache data after retrieving the size, but only if the size is increased
    // since last query.  This function should be called with the |mPipelineCacheMutex| lock already
    // held.
    angle::Result getLockedPipelineCacheDataIfNew(vk::ErrorContext *context,
                                                  size_t *pipelineCacheSizeOut,
                                                  size_t lastSyncSize,
                                                  std::vector<uint8_t> *pipelineCacheDataOut);
    angle::Result syncPipelineCacheVk(vk::ErrorContext *context,
                                      vk::GlobalOps *globalOps,
                                      const gl::Context *contextGL);

    const angle::FeaturesVk &getFeatures() const { return mFeatures; }
    uint32_t getMaxVertexAttribDivisor() const { return mMaxVertexAttribDivisor; }
    VkDeviceSize getMaxVertexAttribStride() const { return mMaxVertexAttribStride; }
    uint32_t getMaxColorInputAttachmentCount() const { return mMaxColorInputAttachmentCount; }

    uint32_t getDefaultUniformBufferSize() const { return mDefaultUniformBufferSize; }

    angle::vk::ICD getEnabledICD() const { return mEnabledICD; }
    bool isMockICDEnabled() const { return mEnabledICD == angle::vk::ICD::Mock; }

    // Query the format properties for select bits (linearTilingFeatures, optimalTilingFeatures
    // and bufferFeatures).  Looks through mandatory features first, and falls back to querying
    // the device (first time only).
    bool hasLinearImageFormatFeatureBits(angle::FormatID format,
                                         const VkFormatFeatureFlags featureBits) const;
    VkFormatFeatureFlags getLinearImageFormatFeatureBits(
        angle::FormatID format,
        const VkFormatFeatureFlags featureBits) const;
    VkFormatFeatureFlags getImageFormatFeatureBits(angle::FormatID format,
                                                   const VkFormatFeatureFlags featureBits) const;
    bool hasImageFormatFeatureBits(angle::FormatID format,
                                   const VkFormatFeatureFlags featureBits) const;
    bool hasBufferFormatFeatureBits(angle::FormatID format,
                                    const VkFormatFeatureFlags featureBits) const;

    bool isAsyncCommandBufferResetAndGarbageCleanupEnabled() const
    {
        return mFeatures.asyncCommandBufferResetAndGarbageCleanup.enabled;
    }

    ANGLE_INLINE egl::ContextPriority getDriverPriority(egl::ContextPriority priority)
    {
        return mCommandQueue.getDriverPriority(priority);
    }

    VkQueue getQueue(egl::ContextPriority priority) { return mCommandQueue.getQueue(priority); }
    // Helpers to implement the functionality of EGL_ANGLE_device_vulkan
    void lockVulkanQueueForExternalAccess() { mCommandQueue.lockVulkanQueueForExternalAccess(); }
    void unlockVulkanQueueForExternalAccess()
    {
        mCommandQueue.unlockVulkanQueueForExternalAccess();
    }

    // This command buffer should be submitted immediately via queueSubmitOneOff.
    angle::Result getCommandBufferOneOff(vk::ErrorContext *context,
                                         vk::ProtectionType protectionType,
                                         vk::ScopedPrimaryCommandBuffer *commandBufferOut)
    {
        return mOneOffCommandPoolMap[protectionType].getCommandBuffer(context, commandBufferOut);
    }

    // Fire off a single command buffer immediately with default priority.
    // Command buffer must be allocated with getCommandBufferOneOff and is reclaimed.
    angle::Result queueSubmitOneOff(vk::ErrorContext *context,
                                    vk::ScopedPrimaryCommandBuffer &&scopedCommandBuffer,
                                    vk::ProtectionType protectionType,
                                    egl::ContextPriority priority,
                                    VkSemaphore waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMasks,
                                    QueueSerial *queueSerialOut);

    angle::Result queueSubmitWaitSemaphore(vk::ErrorContext *context,
                                           egl::ContextPriority priority,
                                           const vk::Semaphore &waitSemaphore,
                                           VkPipelineStageFlags waitSemaphoreStageMasks,
                                           QueueSerial submitQueueSerial);

    template <typename... ArgsT>
    void collectGarbage(const vk::ResourceUse &use, ArgsT... garbageIn)
    {
        if (hasResourceUseFinished(use))
        {
            DestroyGarbage(this, garbageIn...);
        }
        else
        {
            std::vector<vk::GarbageObject> sharedGarbage;
            CollectGarbage(&sharedGarbage, garbageIn...);
            if (!sharedGarbage.empty())
            {
                collectGarbage(use, std::move(sharedGarbage));
            }
        }
    }

    void collectGarbage(const vk::ResourceUse &use, vk::GarbageObjects &&sharedGarbage)
    {
        ASSERT(!sharedGarbage.empty());
        vk::SharedGarbage garbage(use, std::move(sharedGarbage));
        mSharedGarbageList.add(this, std::move(garbage));
    }

    void collectSuballocationGarbage(const vk::ResourceUse &use,
                                     vk::BufferSuballocation &&suballocation,
                                     vk::Buffer &&buffer)
    {
        vk::BufferSuballocationGarbage garbage(use, std::move(suballocation), std::move(buffer));
        mSuballocationGarbageList.add(this, std::move(garbage));
    }

    size_t getNextPipelineCacheBlobCacheSlotIndex(size_t *previousSlotIndexOut);
    size_t updatePipelineCacheChunkCount(size_t chunkCount);
    angle::Result getPipelineCache(vk::ErrorContext *context,
                                   vk::PipelineCacheAccess *pipelineCacheOut);
    angle::Result mergeIntoPipelineCache(vk::ErrorContext *context,
                                         const vk::PipelineCache &pipelineCache);

    void onNewValidationMessage(const std::string &message);
    std::string getAndClearLastValidationMessage(uint32_t *countSinceLastClear);

    const std::vector<const char *> &getSkippedValidationMessages() const
    {
        return mSkippedValidationMessages;
    }
    const std::vector<vk::SkippedSyncvalMessage> &getSkippedSyncvalMessages() const
    {
        return mSkippedSyncvalMessages;
    }

    bool isCoherentColorFramebufferFetchEmulated() const
    {
        return mFeatures.supportsShaderFramebufferFetch.enabled &&
               !mIsColorFramebufferFetchCoherent;
    }

    void onColorFramebufferFetchUse() { mIsColorFramebufferFetchUsed = true; }
    bool isColorFramebufferFetchUsed() const { return mIsColorFramebufferFetchUsed; }

    uint64_t getMaxFenceWaitTimeNs() const;

    ANGLE_INLINE bool isCommandQueueBusy() { return mCommandQueue.isBusy(this); }

    angle::VulkanPerfCounters getCommandQueuePerfCounters()
    {
        return mCommandQueue.getPerfCounters();
    }
    void resetCommandQueuePerFrameCounters() { mCommandQueue.resetPerFramePerfCounters(); }

    vk::GlobalOps *getGlobalOps() const { return mGlobalOps; }

    bool enableDebugUtils() const { return mEnableDebugUtils; }
    bool angleDebuggerMode() const { return mAngleDebuggerMode; }

    SamplerCache &getSamplerCache() { return mSamplerCache; }
    SamplerYcbcrConversionCache &getYuvConversionCache() { return mYuvConversionCache; }

    void onAllocateHandle(vk::HandleType handleType);
    void onDeallocateHandle(vk::HandleType handleType, uint32_t count);

    bool getEnableValidationLayers() const { return mEnableValidationLayers; }

    vk::ResourceSerialFactory &getResourceSerialFactory() { return mResourceSerialFactory; }

    void setGlobalDebugAnnotator(bool *installedAnnotatorOut);

    void outputVmaStatString();

    bool haveSameFormatFeatureBits(angle::FormatID formatID1, angle::FormatID formatID2) const;

    void cleanupGarbage(bool *anyGarbageCleanedOut);
    void cleanupPendingSubmissionGarbage();

    angle::Result submitCommands(vk::ErrorContext *context,
                                 vk::ProtectionType protectionType,
                                 egl::ContextPriority contextPriority,
                                 const vk::Semaphore *signalSemaphore,
                                 const vk::SharedExternalFence *externalFence,
                                 std::vector<VkImageMemoryBarrier> &&imagesToTransitionToForeign,
                                 const QueueSerial &submitQueueSerial);

    angle::Result submitPriorityDependency(vk::ErrorContext *context,
                                           vk::ProtectionTypes protectionTypes,
                                           egl::ContextPriority srcContextPriority,
                                           egl::ContextPriority dstContextPriority,
                                           SerialIndex index);

    void handleDeviceLost();
    angle::Result finishResourceUse(vk::ErrorContext *context, const vk::ResourceUse &use);
    angle::Result finishQueueSerial(vk::ErrorContext *context, const QueueSerial &queueSerial);
    angle::Result waitForResourceUseToFinishWithUserTimeout(vk::ErrorContext *context,
                                                            const vk::ResourceUse &use,
                                                            uint64_t timeout,
                                                            VkResult *result);
    angle::Result checkCompletedCommands(vk::ErrorContext *context);

    angle::Result checkCompletedCommandsAndCleanup(vk::ErrorContext *context);
    angle::Result releaseFinishedCommands(vk::ErrorContext *context);

    angle::Result flushWaitSemaphores(vk::ProtectionType protectionType,
                                      egl::ContextPriority priority,
                                      std::vector<VkSemaphore> &&waitSemaphores,
                                      std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks);
    angle::Result flushRenderPassCommands(vk::Context *context,
                                          vk::ProtectionType protectionType,
                                          egl::ContextPriority priority,
                                          const vk::RenderPass &renderPass,
                                          VkFramebuffer framebufferOverride,
                                          vk::RenderPassCommandBufferHelper **renderPassCommands);
    angle::Result flushOutsideRPCommands(
        vk::Context *context,
        vk::ProtectionType protectionType,
        egl::ContextPriority priority,
        vk::OutsideRenderPassCommandBufferHelper **outsideRPCommands);

    VkResult queuePresent(vk::ErrorContext *context,
                          egl::ContextPriority priority,
                          const VkPresentInfoKHR &presentInfo);

    angle::Result getOutsideRenderPassCommandBufferHelper(
        vk::ErrorContext *context,
        vk::SecondaryCommandPool *commandPool,
        vk::SecondaryCommandMemoryAllocator *commandsAllocator,
        vk::OutsideRenderPassCommandBufferHelper **commandBufferHelperOut);
    angle::Result getRenderPassCommandBufferHelper(
        vk::ErrorContext *context,
        vk::SecondaryCommandPool *commandPool,
        vk::SecondaryCommandMemoryAllocator *commandsAllocator,
        vk::RenderPassCommandBufferHelper **commandBufferHelperOut);

    void recycleOutsideRenderPassCommandBufferHelper(
        vk::OutsideRenderPassCommandBufferHelper **commandBuffer);
    void recycleRenderPassCommandBufferHelper(vk::RenderPassCommandBufferHelper **commandBuffer);

    // Process GPU memory reports
    void processMemoryReportCallback(const VkDeviceMemoryReportCallbackDataEXT &callbackData)
    {
        bool logCallback = getFeatures().logMemoryReportCallbacks.enabled;
        mMemoryReport.processCallback(callbackData, logCallback);
    }

    // Accumulate cache stats for a specific cache
    void accumulateCacheStats(VulkanCacheType cache, const CacheStats &stats)
    {
        std::unique_lock<angle::SimpleMutex> localLock(mCacheStatsMutex);
        mVulkanCacheStats[cache].accumulate(stats);
    }
    // Log cache stats for all caches
    void logCacheStats() const;

    VkPipelineStageFlags getSupportedBufferWritePipelineStageMask() const
    {
        return mSupportedBufferWritePipelineStageMask;
    }

    VkPipelineStageFlags getPipelineStageMask(EventStage eventStage) const
    {
        return mEventStageToPipelineStageFlagsMap[eventStage];
    }

    const ImageMemoryBarrierData &getImageMemoryBarrierData(ImageLayout layout) const
    {
        return mImageLayoutAndMemoryBarrierDataMap[layout];
    }

    VkShaderStageFlags getSupportedVulkanShaderStageMask() const
    {
        return mSupportedVulkanShaderStageMask;
    }

    angle::Result getFormatDescriptorCountForVkFormat(vk::ErrorContext *context,
                                                      VkFormat format,
                                                      uint32_t *descriptorCountOut);

    angle::Result getFormatDescriptorCountForExternalFormat(vk::ErrorContext *context,
                                                            uint64_t format,
                                                            uint32_t *descriptorCountOut);

    VkDeviceSize getMaxCopyBytesUsingCPUWhenPreservingBufferData() const
    {
        return mMaxCopyBytesUsingCPUWhenPreservingBufferData;
    }

    const vk::ExtensionNameList &getEnabledInstanceExtensions() const
    {
        return mEnabledInstanceExtensions;
    }

    const vk::ExtensionNameList &getEnabledDeviceExtensions() const
    {
        return mEnabledDeviceExtensions;
    }

    VkDeviceSize getPreferedBufferBlockSize(uint32_t memoryTypeIndex) const;

    size_t getDefaultBufferAlignment() const { return mDefaultBufferAlignment; }

    uint32_t getStagingBufferMemoryTypeIndex(vk::MemoryCoherency coherency) const
    {
        return mStagingBufferMemoryTypeIndex[coherency];
    }
    size_t getStagingBufferAlignment() const { return mStagingBufferAlignment; }

    uint32_t getVertexConversionBufferMemoryTypeIndex(MemoryHostVisibility hostVisibility) const
    {
        return hostVisibility == MemoryHostVisibility::Visible
                   ? mHostVisibleVertexConversionBufferMemoryTypeIndex
                   : mDeviceLocalVertexConversionBufferMemoryTypeIndex;
    }
    size_t getVertexConversionBufferAlignment() const { return mVertexConversionBufferAlignment; }

    uint32_t getDeviceLocalMemoryTypeIndex() const
    {
        return mDeviceLocalVertexConversionBufferMemoryTypeIndex;
    }

    bool isShadingRateSupported(gl::ShadingRate shadingRate) const
    {
        return mSupportedFragmentShadingRates.test(shadingRate);
    }

    VkExtent2D getMaxFragmentShadingRateAttachmentTexelSize() const
    {
        ASSERT(mFeatures.supportsFoveatedRendering.enabled);
        return mFragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize;
    }

    void addBufferBlockToOrphanList(vk::BufferBlock *block) { mOrphanedBufferBlockList.add(block); }

    VkDeviceSize getSuballocationDestroyedSize() const
    {
        return mSuballocationGarbageList.getDestroyedGarbageSize();
    }
    void onBufferPoolPrune() { mSuballocationGarbageList.resetDestroyedGarbageSize(); }
    VkDeviceSize getSuballocationGarbageSize() const
    {
        return mSuballocationGarbageList.getSubmittedGarbageSize();
    }
    VkDeviceSize getPendingSuballocationGarbageSize()
    {
        return mSuballocationGarbageList.getUnsubmittedGarbageSize();
    }

    VkDeviceSize getPendingSubmissionGarbageSize() const
    {
        return mSharedGarbageList.getUnsubmittedGarbageSize();
    }

    ANGLE_INLINE VkFilter getPreferredFilterForYUV(VkFilter defaultFilter)
    {
        return getFeatures().preferLinearFilterForYUV.enabled ? VK_FILTER_LINEAR : defaultFilter;
    }

    angle::Result allocateScopedQueueSerialIndex(vk::ScopedQueueSerialIndex *indexOut);
    angle::Result allocateQueueSerialIndex(SerialIndex *serialIndexOut);
    size_t getLargestQueueSerialIndexEverAllocated() const
    {
        return mQueueSerialIndexAllocator.getLargestIndexEverAllocated();
    }
    void releaseQueueSerialIndex(SerialIndex index);
    Serial generateQueueSerial(SerialIndex index);
    void reserveQueueSerials(SerialIndex index,
                             size_t count,
                             RangedSerialFactory *rangedSerialFactory);

    // Return true if all serials in ResourceUse have been submitted.
    bool hasResourceUseSubmitted(const vk::ResourceUse &use) const;
    bool hasQueueSerialSubmitted(const QueueSerial &queueSerial) const;
    Serial getLastSubmittedSerial(SerialIndex index) const;
    // Return true if all serials in ResourceUse have been finished.
    bool hasResourceUseFinished(const vk::ResourceUse &use) const;
    bool hasQueueSerialFinished(const QueueSerial &queueSerial) const;

    // Memory statistics can be updated on allocation and deallocation.
    template <typename HandleT>
    void onMemoryAlloc(vk::MemoryAllocationType allocType,
                       VkDeviceSize size,
                       uint32_t memoryTypeIndex,
                       HandleT handle)
    {
        mMemoryAllocationTracker.onMemoryAllocImpl(allocType, size, memoryTypeIndex,
                                                   reinterpret_cast<void *>(handle));
    }

    template <typename HandleT>
    void onMemoryDealloc(vk::MemoryAllocationType allocType,
                         VkDeviceSize size,
                         uint32_t memoryTypeIndex,
                         HandleT handle)
    {
        mMemoryAllocationTracker.onMemoryDeallocImpl(allocType, size, memoryTypeIndex,
                                                     reinterpret_cast<void *>(handle));
    }

    MemoryAllocationTracker *getMemoryAllocationTracker() { return &mMemoryAllocationTracker; }

    VkDeviceSize getPendingGarbageSizeLimit() const { return mPendingGarbageSizeLimit; }

    void requestAsyncCommandsAndGarbageCleanup(vk::ErrorContext *context);

    VkDeviceSize getMaxMemoryAllocationSize()
    {
        return mMaintenance3Properties.maxMemoryAllocationSize;
    }

    // Cleanup garbage and finish command batches from the queue if necessary in the event of an OOM
    // error.
    angle::Result cleanupSomeGarbage(ErrorContext *context, bool *anyGarbageCleanedOut);

    // Static function to get Vulkan object type name.
    static const char *GetVulkanObjectTypeName(VkObjectType type);

    bool nullColorAttachmentWithExternalFormatResolve() const
    {
#if defined(ANGLE_PLATFORM_ANDROID)
        ASSERT(mFeatures.supportsExternalFormatResolve.enabled);
        return mExternalFormatResolveProperties.nullColorAttachmentWithExternalFormatResolve;
#else
        return false;
#endif
    }

    vk::ExternalFormatTable *getExternalFormatTable() { return &mExternalFormatTable; }

    std::ostringstream &getPipelineCacheGraphStream() { return mPipelineCacheGraph; }
    bool isPipelineCacheGraphDumpEnabled() const { return mDumpPipelineCacheGraph; }
    const char *getPipelineCacheGraphDumpPath() const
    {
        return mPipelineCacheGraphDumpPath.c_str();
    }

    vk::RefCountedEventRecycler *getRefCountedEventRecycler() { return &mRefCountedEventRecycler; }

    std::thread::id getCleanUpThreadId() const { return mCleanUpThread.getThreadId(); }

    const vk::DescriptorSetLayoutPtr &getEmptyDescriptorLayout() const
    {
        ASSERT(mPlaceHolderDescriptorSetLayout);
        ASSERT(mPlaceHolderDescriptorSetLayout->valid());
        return mPlaceHolderDescriptorSetLayout;
    }

  private:
    angle::Result setupDevice(vk::ErrorContext *context,
                              const angle::FeatureOverrides &featureOverrides,
                              const char *wsiLayer,
                              UseVulkanSwapchain useVulkanSwapchain,
                              angle::NativeWindowSystem nativeWindowSystem);
    angle::Result createDeviceAndQueue(vk::ErrorContext *context, uint32_t queueFamilyIndex);
    void ensureCapsInitialized() const;
    void initializeValidationMessageSuppressions();

    void queryDeviceExtensionFeatures(const vk::ExtensionNameList &deviceExtensionNames);
    void appendDeviceExtensionFeaturesNotPromoted(const vk::ExtensionNameList &deviceExtensionNames,
                                                  VkPhysicalDeviceFeatures2KHR *deviceFeatures,
                                                  VkPhysicalDeviceProperties2 *deviceProperties);
    void appendDeviceExtensionFeaturesPromotedTo11(
        const vk::ExtensionNameList &deviceExtensionNames,
        VkPhysicalDeviceFeatures2KHR *deviceFeatures,
        VkPhysicalDeviceProperties2 *deviceProperties);
    void appendDeviceExtensionFeaturesPromotedTo12(
        const vk::ExtensionNameList &deviceExtensionNames,
        VkPhysicalDeviceFeatures2KHR *deviceFeatures,
        VkPhysicalDeviceProperties2 *deviceProperties);
    void appendDeviceExtensionFeaturesPromotedTo13(
        const vk::ExtensionNameList &deviceExtensionNames,
        VkPhysicalDeviceFeatures2KHR *deviceFeatures,
        VkPhysicalDeviceProperties2 *deviceProperties);

    angle::Result enableInstanceExtensions(vk::ErrorContext *context,
                                           const VulkanLayerVector &enabledInstanceLayerNames,
                                           const char *wsiExtension,
                                           UseVulkanSwapchain useVulkanSwapchain,
                                           bool canLoadDebugUtils);
    angle::Result enableDeviceExtensions(vk::ErrorContext *context,
                                         const angle::FeatureOverrides &featureOverrides,
                                         UseVulkanSwapchain useVulkanSwapchain,
                                         angle::NativeWindowSystem nativeWindowSystem);

    void enableDeviceExtensionsNotPromoted(const vk::ExtensionNameList &deviceExtensionNames);
    void enableDeviceExtensionsPromotedTo11(const vk::ExtensionNameList &deviceExtensionNames);
    void enableDeviceExtensionsPromotedTo12(const vk::ExtensionNameList &deviceExtensionNames);
    void enableDeviceExtensionsPromotedTo13(const vk::ExtensionNameList &deviceExtensionNames);

    void initDeviceExtensionEntryPoints();
    // Initialize extension entry points from core ones if needed
    void initializeInstanceExtensionEntryPointsFromCore() const;
    void initializeDeviceExtensionEntryPointsFromCore() const;

    void initFeatures(const vk::ExtensionNameList &extensions,
                      const angle::FeatureOverrides &featureOverrides,
                      UseVulkanSwapchain useVulkanSwapchain,
                      angle::NativeWindowSystem nativeWindowSystem);
    void appBasedFeatureOverrides(const vk::ExtensionNameList &extensions);
    angle::Result initPipelineCache(vk::ErrorContext *context,
                                    vk::PipelineCache *pipelineCache,
                                    bool *success);
    angle::Result ensurePipelineCacheInitialized(vk::ErrorContext *context);

    template <VkFormatFeatureFlags VkFormatProperties::*features>
    VkFormatFeatureFlags getFormatFeatureBits(angle::FormatID formatID,
                                              const VkFormatFeatureFlags featureBits) const;

    template <VkFormatFeatureFlags VkFormatProperties::*features>
    bool hasFormatFeatureBits(angle::FormatID formatID,
                              const VkFormatFeatureFlags featureBits) const;

    // Initialize VMA allocator and buffer suballocator related data.
    angle::Result initializeMemoryAllocator(vk::ErrorContext *context);

    // Query and cache supported fragment shading rates
    void queryAndCacheFragmentShadingRates();
    // Determine support for shading rate based rendering
    bool canSupportFragmentShadingRate() const;
    // Determine support for foveated rendering
    bool canSupportFoveatedRendering() const;
    // Prefer host visible device local via device local based on device type and heap size.
    bool canPreferDeviceLocalMemoryHostVisible(VkPhysicalDeviceType deviceType);

    // Find the threshold for pending suballocation and image garbage sizes before the context
    // should be flushed.
    void calculatePendingGarbageSizeLimit();

    template <typename CommandBufferHelperT, typename RecyclerT>
    angle::Result getCommandBufferImpl(vk::ErrorContext *context,
                                       vk::SecondaryCommandPool *commandPool,
                                       vk::SecondaryCommandMemoryAllocator *commandsAllocator,
                                       RecyclerT *recycler,
                                       CommandBufferHelperT **commandBufferHelperOut);

    vk::GlobalOps *mGlobalOps;

    void *mLibVulkanLibrary;

    mutable bool mCapsInitialized;
    mutable gl::Caps mNativeCaps;
    mutable gl::TextureCapsMap mNativeTextureCaps;
    mutable gl::Extensions mNativeExtensions;
    mutable gl::Limitations mNativeLimitations;
    mutable ShPixelLocalStorageOptions mNativePLSOptions;
    mutable angle::FeaturesVk mFeatures;

    // The instance and device versions.  The instance version is the one from the Vulkan loader,
    // while the device version comes from VkPhysicalDeviceProperties::apiVersion.  With instance
    // version 1.0, only device version 1.0 can be used.  If instance version is at least 1.1, any
    // device version (even higher than that) can be used.  Some extensions have been promoted to
    // Vulkan 1.1 or higher, but the version check must be done against the instance or device
    // version, depending on whether it's an instance or device extension.
    //
    // Note that mDeviceVersion is technically redundant with mPhysicalDeviceProperties.apiVersion,
    // but ANGLE may use a smaller version with problematic ICDs.
    uint32_t mInstanceVersion;
    uint32_t mDeviceVersion;

    VkInstance mInstance;
    bool mEnableValidationLayers;
    // True if ANGLE is enabling the VK_EXT_debug_utils extension.
    bool mEnableDebugUtils;
    // True if ANGLE should call the vkCmd*DebugUtilsLabelEXT functions in order to communicate
    // to debuggers (e.g. AGI) the OpenGL ES commands that the application uses.  This is
    // independent of mEnableDebugUtils, as an external graphics debugger can enable the
    // VK_EXT_debug_utils extension and cause this to be set true.
    bool mAngleDebuggerMode;
    angle::vk::ICD mEnabledICD;
    VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
    VkPhysicalDevice mPhysicalDevice;

    VkPhysicalDeviceProperties2 mPhysicalDeviceProperties2;
    VkPhysicalDeviceProperties &mPhysicalDeviceProperties;

    VkPhysicalDeviceIDProperties mPhysicalDeviceIDProperties;
    VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
    VkPhysicalDeviceLineRasterizationFeaturesEXT mLineRasterizationFeatures;
    VkPhysicalDeviceProvokingVertexFeaturesEXT mProvokingVertexFeatures;
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT mVertexAttributeDivisorFeatures;
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT mVertexAttributeDivisorProperties;
    VkPhysicalDeviceTransformFeedbackFeaturesEXT mTransformFeedbackFeatures;
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT mIndexTypeUint8Features;
    VkPhysicalDeviceSubgroupProperties mSubgroupProperties;
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR mSubgroupExtendedTypesFeatures;
    VkPhysicalDeviceDeviceMemoryReportFeaturesEXT mMemoryReportFeatures;
    VkDeviceDeviceMemoryReportCreateInfoEXT mMemoryReportCallback;
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR mShaderFloat16Int8Features;
    VkPhysicalDeviceDepthStencilResolvePropertiesKHR mDepthStencilResolveProperties;
    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT
        mMultisampledRenderToSingleSampledFeatures;
    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT mImage2dViewOf3dFeatures;
    VkPhysicalDeviceMultiviewFeatures mMultiviewFeatures;
    VkPhysicalDeviceFeatures2KHR mEnabledFeatures;
    VkPhysicalDeviceMultiviewProperties mMultiviewProperties;
    VkPhysicalDeviceDriverPropertiesKHR mDriverProperties;
    VkPhysicalDeviceCustomBorderColorFeaturesEXT mCustomBorderColorFeatures;
    VkPhysicalDeviceProtectedMemoryFeatures mProtectedMemoryFeatures;
    VkPhysicalDeviceHostQueryResetFeaturesEXT mHostQueryResetFeatures;
    VkPhysicalDeviceDepthClampZeroOneFeaturesEXT mDepthClampZeroOneFeatures;
    VkPhysicalDeviceDepthClipControlFeaturesEXT mDepthClipControlFeatures;
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT mBlendOperationAdvancedFeatures;
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT mPrimitivesGeneratedQueryFeatures;
    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT mPrimitiveTopologyListRestartFeatures;
    VkPhysicalDeviceSamplerYcbcrConversionFeatures mSamplerYcbcrConversionFeatures;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT mExtendedDynamicStateFeatures;
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT mExtendedDynamicState2Features;
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT mGraphicsPipelineLibraryFeatures;
    VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT mGraphicsPipelineLibraryProperties;
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT mVertexInputDynamicStateFeatures;
    VkPhysicalDeviceDynamicRenderingFeaturesKHR mDynamicRenderingFeatures;
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR mDynamicRenderingLocalReadFeatures;
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR mFragmentShadingRateFeatures;
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR mFragmentShadingRateProperties;
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT mFragmentShaderInterlockFeatures;
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR mImagelessFramebufferFeatures;
    VkPhysicalDevicePipelineRobustnessFeaturesEXT mPipelineRobustnessFeatures;
    VkPhysicalDevicePipelineProtectedAccessFeaturesEXT mPipelineProtectedAccessFeatures;
    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
        mRasterizationOrderAttachmentAccessFeatures;
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT mShaderAtomicFloatFeatures;
    VkPhysicalDeviceMaintenance5FeaturesKHR mMaintenance5Features;
    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT mSwapchainMaintenance1Features;
    VkPhysicalDeviceLegacyDitheringFeaturesEXT mDitheringFeatures;
    VkPhysicalDeviceDrmPropertiesEXT mDrmProperties;
    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR mTimelineSemaphoreFeatures;
    VkPhysicalDeviceHostImageCopyFeaturesEXT mHostImageCopyFeatures;
    VkPhysicalDeviceHostImageCopyPropertiesEXT mHostImageCopyProperties;
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT mTextureCompressionASTCHDRFeatures;
    std::vector<VkImageLayout> mHostImageCopySrcLayoutsStorage;
    std::vector<VkImageLayout> mHostImageCopyDstLayoutsStorage;
    VkPhysicalDeviceImageCompressionControlFeaturesEXT mImageCompressionControlFeatures;
    VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT
        mImageCompressionControlSwapchainFeatures;
#if defined(ANGLE_PLATFORM_ANDROID)
    VkPhysicalDeviceExternalFormatResolveFeaturesANDROID mExternalFormatResolveFeatures;
    VkPhysicalDeviceExternalFormatResolvePropertiesANDROID mExternalFormatResolveProperties;
#endif
    VkPhysicalDevice8BitStorageFeatures m8BitStorageFeatures;
    VkPhysicalDevice16BitStorageFeatures m16BitStorageFeatures;
    VkPhysicalDeviceSynchronization2Features mSynchronization2Features;
    VkPhysicalDeviceVariablePointersFeatures mVariablePointersFeatures;
    VkPhysicalDeviceFloatControlsProperties mFloatControlProperties;
    VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR mUniformBufferStandardLayoutFeatures;
    VkPhysicalDeviceMaintenance3Properties mMaintenance3Properties;

    uint32_t mLegacyDitheringVersion = 0;

    angle::PackedEnumBitSet<gl::ShadingRate, uint8_t> mSupportedFragmentShadingRates;
    angle::PackedEnumMap<gl::ShadingRate, VkSampleCountFlags>
        mSupportedFragmentShadingRateSampleCounts;
    std::vector<VkQueueFamilyProperties> mQueueFamilyProperties;
    uint32_t mCurrentQueueFamilyIndex;
    uint32_t mMaxVertexAttribDivisor;
    VkDeviceSize mMaxVertexAttribStride;
    mutable uint32_t mMaxColorInputAttachmentCount;
    uint32_t mDefaultUniformBufferSize;
    VkDevice mDevice;
    VkDeviceSize mMaxCopyBytesUsingCPUWhenPreservingBufferData;

    bool mDeviceLost;

    vk::SharedGarbageList<vk::SharedGarbage> mSharedGarbageList;
    // Suballocations have its own dedicated garbage list for performance optimization since they
    // tend to be the most common garbage objects.
    vk::SharedGarbageList<vk::BufferSuballocationGarbage> mSuballocationGarbageList;
    // Holds orphaned BufferBlocks when ShareGroup gets destroyed
    vk::BufferBlockGarbageList mOrphanedBufferBlockList;
    // Holds RefCountedEvent that are free and ready to reuse
    vk::RefCountedEventRecycler mRefCountedEventRecycler;

    VkDeviceSize mPendingGarbageSizeLimit;

    vk::FormatTable mFormatTable;
    // A cache of VkFormatProperties as queried from the device over time.
    mutable angle::FormatMap<VkFormatProperties> mFormatProperties;

    vk::Allocator mAllocator;

    // Used to allocate memory for images using VMA, utilizing suballocation.
    vk::ImageMemorySuballocator mImageMemorySuballocator;

    vk::MemoryProperties mMemoryProperties;
    VkDeviceSize mPreferredLargeHeapBlockSize;

    // The default alignment for BufferVk object
    size_t mDefaultBufferAlignment;
    // The memory type index for staging buffer that is host visible.
    angle::PackedEnumMap<vk::MemoryCoherency, uint32_t> mStagingBufferMemoryTypeIndex;
    size_t mStagingBufferAlignment;
    // For vertex conversion buffers
    uint32_t mHostVisibleVertexConversionBufferMemoryTypeIndex;
    uint32_t mDeviceLocalVertexConversionBufferMemoryTypeIndex;
    size_t mVertexConversionBufferAlignment;

    // The mutex protects -
    // 1. initialization of the cache
    // 2. Vulkan driver guarantess synchronization for read and write operations but the spec
    //    requires external synchronization when mPipelineCache is the dstCache of
    //    vkMergePipelineCaches. Lock the mutex if mergeProgramPipelineCachesToGlobalCache is
    //    enabled
    angle::SimpleMutex mPipelineCacheMutex;
    vk::PipelineCache mPipelineCache;
    size_t mCurrentPipelineCacheBlobCacheSlotIndex;
    size_t mPipelineCacheChunkCount;
    uint32_t mPipelineCacheVkUpdateTimeout;
    size_t mPipelineCacheSizeAtLastSync;
    std::atomic<bool> mPipelineCacheInitialized;

    // Latest validation data for debug overlay.
    std::string mLastValidationMessage;
    uint32_t mValidationMessageCount;

    // Skipped validation messages.  The exact contents of the list depends on the availability
    // of certain extensions.
    std::vector<const char *> mSkippedValidationMessages;
    // Syncval skipped messages.  The exact contents of the list depends on the availability of
    // certain extensions.
    std::vector<vk::SkippedSyncvalMessage> mSkippedSyncvalMessages;

    // Whether framebuffer fetch is internally coherent.  If framebuffer fetch is not coherent,
    // technically ANGLE could simply not expose EXT_shader_framebuffer_fetch and instead only
    // expose EXT_shader_framebuffer_fetch_non_coherent.  In practice, too many Android apps assume
    // EXT_shader_framebuffer_fetch is available and break without it.  Others use string matching
    // to detect when EXT_shader_framebuffer_fetch is available, and accidentally match
    // EXT_shader_framebuffer_fetch_non_coherent and believe coherent framebuffer fetch is
    // available.
    //
    // For these reasons, ANGLE always exposes EXT_shader_framebuffer_fetch.  To ensure coherence
    // between draw calls, it automatically inserts barriers between draw calls when the program
    // uses framebuffer fetch.  ANGLE does not attempt to guarantee coherence for self-overlapping
    // geometry, which makes this emulation incorrect per spec, but practically harmless.
    //
    // This emulation can also be used to implement coherent advanced blend similarly if needed.
    bool mIsColorFramebufferFetchCoherent;
    // Whether framebuffer fetch has been used, for the purposes of more accurate syncval error
    // filtering.
    bool mIsColorFramebufferFetchUsed;

    // How close to VkPhysicalDeviceLimits::maxMemoryAllocationCount we allow ourselves to get
    static constexpr double kPercentMaxMemoryAllocationCount = 0.3;
    // How many objects to garbage collect before issuing a flush()
    uint32_t mGarbageCollectionFlushThreshold;

    // Only used for "one off" command buffers.
    angle::PackedEnumMap<vk::ProtectionType, OneOffCommandPool> mOneOffCommandPoolMap;

    // Command queue
    vk::CommandQueue mCommandQueue;

    // Async cleanup thread
    vk::CleanUpThread mCleanUpThread;

    // Command buffer pool management.
    vk::CommandBufferRecycler<vk::OutsideRenderPassCommandBufferHelper>
        mOutsideRenderPassCommandBufferRecycler;
    vk::CommandBufferRecycler<vk::RenderPassCommandBufferHelper> mRenderPassCommandBufferRecycler;

    SamplerCache mSamplerCache;
    SamplerYcbcrConversionCache mYuvConversionCache;
    angle::HashMap<VkFormat, uint32_t> mVkFormatDescriptorCountMap;
    vk::ActiveHandleCounter mActiveHandleCounts;
    angle::SimpleMutex mActiveHandleCountsMutex;

    // Tracks resource serials.
    vk::ResourceSerialFactory mResourceSerialFactory;

    // QueueSerial generator
    vk::QueueSerialIndexAllocator mQueueSerialIndexAllocator;
    std::array<AtomicSerialFactory, kMaxQueueSerialIndexCount> mQueueSerialFactory;

    // Application executable information
    VkApplicationInfo mApplicationInfo;
    // Process GPU memory reports
    vk::MemoryReport mMemoryReport;
    // Helpers for adding trace annotations
    DebugAnnotatorVk mAnnotator;

    // Stats about all Vulkan object caches
    VulkanCacheStats mVulkanCacheStats;
    mutable angle::SimpleMutex mCacheStatsMutex;

    // A mask to filter out Vulkan pipeline stages that are not supported, applied in situations
    // where multiple stages are prespecified (for example with image layout transitions):
    //
    // - Excludes GEOMETRY if geometry shaders are not supported.
    // - Excludes TESSELLATION_CONTROL and TESSELLATION_EVALUATION if tessellation shaders are
    // not
    //   supported.
    //
    // Note that this mask can have bits set that don't correspond to valid stages, so it's
    // strictly only useful for masking out unsupported stages in an otherwise valid set of
    // stages.
    VkPipelineStageFlags mSupportedBufferWritePipelineStageMask;
    VkShaderStageFlags mSupportedVulkanShaderStageMask;
    // The 1:1 mapping between EventStage and VkPipelineStageFlags
    EventStageToVkPipelineStageFlagsMap mEventStageToPipelineStageFlagsMap;
    ImageLayoutToMemoryBarrierDataMap mImageLayoutAndMemoryBarrierDataMap;

    // Use thread pool to compress cache data.
    std::shared_ptr<angle::WaitableEvent> mCompressEvent;

    VulkanLayerVector mEnabledDeviceLayerNames;
    vk::ExtensionNameList mEnabledInstanceExtensions;
    vk::ExtensionNameList mEnabledDeviceExtensions;

    // Memory tracker for allocations and deallocations.
    MemoryAllocationTracker mMemoryAllocationTracker;

    vk::ExternalFormatTable mExternalFormatTable;

    // A graph built from pipeline descs and their transitions.  This is not thread-safe, but it's
    // only a debug feature that's disabled by default.
    std::ostringstream mPipelineCacheGraph;
    bool mDumpPipelineCacheGraph;
    std::string mPipelineCacheGraphDumpPath;

    // A placeholder descriptor set layout handle for layouts with no bindings.
    vk::DescriptorSetLayoutPtr mPlaceHolderDescriptorSetLayout;
};

ANGLE_INLINE Serial Renderer::generateQueueSerial(SerialIndex index)
{
    return mQueueSerialFactory[index].generate();
}

ANGLE_INLINE void Renderer::reserveQueueSerials(SerialIndex index,
                                                size_t count,
                                                RangedSerialFactory *rangedSerialFactory)
{
    mQueueSerialFactory[index].reserve(rangedSerialFactory, count);
}

ANGLE_INLINE bool Renderer::hasResourceUseSubmitted(const vk::ResourceUse &use) const
{
    return mCommandQueue.hasResourceUseSubmitted(use);
}

ANGLE_INLINE bool Renderer::hasQueueSerialSubmitted(const QueueSerial &queueSerial) const
{
    return mCommandQueue.hasQueueSerialSubmitted(queueSerial);
}

ANGLE_INLINE Serial Renderer::getLastSubmittedSerial(SerialIndex index) const
{
    return mCommandQueue.getLastSubmittedSerial(index);
}

ANGLE_INLINE bool Renderer::hasResourceUseFinished(const vk::ResourceUse &use) const
{
    return mCommandQueue.hasResourceUseFinished(use);
}

ANGLE_INLINE bool Renderer::hasQueueSerialFinished(const QueueSerial &queueSerial) const
{
    return mCommandQueue.hasQueueSerialFinished(queueSerial);
}

ANGLE_INLINE void Renderer::requestAsyncCommandsAndGarbageCleanup(vk::ErrorContext *context)
{
    mCleanUpThread.requestCleanUp();
}

ANGLE_INLINE angle::Result Renderer::checkCompletedCommands(vk::ErrorContext *context)
{
    return mCommandQueue.checkCompletedCommands(context);
}

ANGLE_INLINE angle::Result Renderer::checkCompletedCommandsAndCleanup(vk::ErrorContext *context)
{
    return mCommandQueue.checkAndCleanupCompletedCommands(context);
}

ANGLE_INLINE angle::Result Renderer::releaseFinishedCommands(vk::ErrorContext *context)
{
    return mCommandQueue.releaseFinishedCommands(context);
}

template <typename ArgT, typename... ArgsT>
void DestroyGarbage(Renderer *renderer, ArgT object, ArgsT... objectsIn)
{
    if (object->valid())
    {
        object->destroy(renderer->getDevice());
    }
    DestroyGarbage(renderer, objectsIn...);
}

template <typename... ArgsT>
void DestroyGarbage(Renderer *renderer, vk::Allocation *object, ArgsT... objectsIn)
{
    if (object->valid())
    {
        object->destroy(renderer->getAllocator());
    }
    DestroyGarbage(renderer, objectsIn...);
}
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_
