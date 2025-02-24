//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryTracking.cpp:
//    Implements the class methods in MemoryTracking.h.
//

#include "libANGLE/renderer/vulkan/MemoryTracking.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

// Consts
namespace
{
// This flag is used for memory allocation tracking using allocation size counters.
constexpr bool kTrackMemoryAllocationSizes = true;
#if defined(ANGLE_ENABLE_MEMORY_ALLOC_LOGGING)
// Flag used for logging memory allocations and deallocations.
constexpr bool kTrackMemoryAllocationDebug = true;
static_assert(kTrackMemoryAllocationSizes,
              "kTrackMemoryAllocationSizes must be enabled to use kTrackMemoryAllocationDebug.");
#else
// Only the allocation size counters are used (if enabled).
constexpr bool kTrackMemoryAllocationDebug = false;
#endif
}  // namespace

namespace rx
{

namespace
{
// Output memory log stream based on level of severity.
void OutputMemoryLogStream(std::stringstream &outStream, vk::MemoryLogSeverity severity)
{
    if (!kTrackMemoryAllocationSizes)
    {
        return;
    }

    switch (severity)
    {
        case vk::MemoryLogSeverity::INFO:
            INFO() << outStream.str();
            break;
        case vk::MemoryLogSeverity::WARN:
            WARN() << outStream.str();
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// Check for currently allocated memory. It is used at the end of the renderer object and when
// there is an allocation error (from ANGLE_VK_TRY()).
void CheckForCurrentMemoryAllocations(vk::Renderer *renderer, vk::MemoryLogSeverity severity)
{
    if (kTrackMemoryAllocationSizes)
    {
        for (uint32_t i = 0; i < vk::kMemoryAllocationTypeCount; i++)
        {
            if (renderer->getMemoryAllocationTracker()->getActiveMemoryAllocationsSize(i) == 0)
            {
                continue;
            }

            std::stringstream outStream;

            outStream << "Currently allocated size for memory allocation type ("
                      << vk::kMemoryAllocationTypeMessage[i] << "): "
                      << renderer->getMemoryAllocationTracker()->getActiveMemoryAllocationsSize(i)
                      << " | Count: "
                      << renderer->getMemoryAllocationTracker()->getActiveMemoryAllocationsCount(i)
                      << std::endl;

            for (uint32_t heapIndex = 0;
                 heapIndex < renderer->getMemoryProperties().getMemoryHeapCount(); heapIndex++)
            {
                outStream
                    << "--> Heap index " << heapIndex << ": "
                    << renderer->getMemoryAllocationTracker()->getActiveHeapMemoryAllocationsSize(
                           i, heapIndex)
                    << " | Count: "
                    << renderer->getMemoryAllocationTracker()->getActiveHeapMemoryAllocationsCount(
                           i, heapIndex)
                    << std::endl;
            }

            // Output the log stream based on the level of severity.
            OutputMemoryLogStream(outStream, severity);
        }
    }
}

// In case of an allocation error, log pending memory allocation if the size in non-zero.
void LogPendingMemoryAllocation(vk::Renderer *renderer, vk::MemoryLogSeverity severity)
{
    if (!kTrackMemoryAllocationSizes)
    {
        return;
    }

    vk::MemoryAllocationType allocInfo =
        renderer->getMemoryAllocationTracker()->getPendingMemoryAllocationType();
    VkDeviceSize allocSize =
        renderer->getMemoryAllocationTracker()->getPendingMemoryAllocationSize();
    uint32_t memoryTypeIndex = renderer->getMemoryAllocationTracker()->getPendingMemoryTypeIndex();
    uint32_t memoryHeapIndex =
        renderer->getMemoryProperties().getHeapIndexForMemoryType(memoryTypeIndex);

    if (allocSize != 0)
    {
        std::stringstream outStream;

        outStream << "Pending allocation size for memory allocation type ("
                  << vk::kMemoryAllocationTypeMessage[ToUnderlying(allocInfo)]
                  << ") for heap index " << memoryHeapIndex << " (type index " << memoryTypeIndex
                  << "): " << allocSize;

        // Output the log stream based on the level of severity.
        OutputMemoryLogStream(outStream, severity);
    }
}

void LogMemoryHeapStats(vk::Renderer *renderer, vk::MemoryLogSeverity severity)
{
    if (!kTrackMemoryAllocationSizes)
    {
        return;
    }

    // Log stream for the heap information.
    std::stringstream outStream;

    // VkPhysicalDeviceMemoryProperties2 enables the use of memory budget properties if
    // supported.
    VkPhysicalDeviceMemoryProperties2KHR memoryProperties;
    memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
    memoryProperties.pNext = nullptr;

    VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudgetProperties;
    memoryBudgetProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
    memoryBudgetProperties.pNext = nullptr;

    if (renderer->getFeatures().supportsMemoryBudget.enabled)
    {
        vk::AddToPNextChain(&memoryProperties, &memoryBudgetProperties);
    }

    vkGetPhysicalDeviceMemoryProperties2(renderer->getPhysicalDevice(), &memoryProperties);

    // Add memory heap information to the stream.
    outStream << "Memory heap info" << std::endl;

    outStream << std::endl << "* Available memory heaps:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryHeapCount; i++)
    {
        outStream << std::dec << i
                  << " | Heap size: " << memoryProperties.memoryProperties.memoryHeaps[i].size
                  << " | Flags: 0x" << std::hex
                  << memoryProperties.memoryProperties.memoryHeaps[i].flags << std::endl;
    }

    if (renderer->getFeatures().supportsMemoryBudget.enabled)
    {
        outStream << std::endl << "* Available memory budget and usage per heap:" << std::endl;
        for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryHeapCount; i++)
        {
            outStream << std::dec << i << " | Heap budget: " << memoryBudgetProperties.heapBudget[i]
                      << " | Heap usage: " << memoryBudgetProperties.heapUsage[i] << std::endl;
        }
    }

    outStream << std::endl << "* Available memory types:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; i++)
    {
        outStream << std::dec << i
                  << " | Heap index: " << memoryProperties.memoryProperties.memoryTypes[i].heapIndex
                  << " | Property flags: 0x" << std::hex
                  << memoryProperties.memoryProperties.memoryTypes[i].propertyFlags << std::endl;
    }

    // Output the log stream based on the level of severity.
    OutputMemoryLogStream(outStream, severity);
}
}  // namespace

MemoryAllocationTracker::MemoryAllocationTracker(vk::Renderer *renderer)
    : mRenderer(renderer), mMemoryAllocationID(0)
{}

void MemoryAllocationTracker::initMemoryTrackers()
{
    // Allocation counters are initialized here to keep track of the size and count of the memory
    // allocations.
    for (size_t allocTypeIndex = 0; allocTypeIndex < mActiveMemoryAllocationsSize.size();
         allocTypeIndex++)
    {
        mActiveMemoryAllocationsSize[allocTypeIndex]  = 0;
        mActiveMemoryAllocationsCount[allocTypeIndex] = 0;

        // Per-heap allocation counters are initialized here.
        for (size_t heapIndex = 0;
             heapIndex < mRenderer->getMemoryProperties().getMemoryHeapCount(); heapIndex++)
        {
            mActivePerHeapMemoryAllocationsSize[allocTypeIndex][heapIndex]  = 0;
            mActivePerHeapMemoryAllocationsCount[allocTypeIndex][heapIndex] = 0;
        }
    }

    resetPendingMemoryAlloc();
}

void MemoryAllocationTracker::onDestroy()
{
    if (kTrackMemoryAllocationDebug)
    {
        CheckForCurrentMemoryAllocations(mRenderer, vk::MemoryLogSeverity::INFO);
    }
}

void MemoryAllocationTracker::onDeviceInit()
{
    if (kTrackMemoryAllocationDebug)
    {
        LogMemoryHeapStats(mRenderer, vk::MemoryLogSeverity::INFO);
    }
}

void MemoryAllocationTracker::logMemoryStatsOnError()
{
    CheckForCurrentMemoryAllocations(mRenderer, vk::MemoryLogSeverity::WARN);
    LogPendingMemoryAllocation(mRenderer, vk::MemoryLogSeverity::WARN);
    LogMemoryHeapStats(mRenderer, vk::MemoryLogSeverity::WARN);
}

void MemoryAllocationTracker::onMemoryAllocImpl(vk::MemoryAllocationType allocType,
                                                VkDeviceSize size,
                                                uint32_t memoryTypeIndex,
                                                void *handle)
{
    ASSERT(allocType != vk::MemoryAllocationType::InvalidEnum && size != 0);

    if (kTrackMemoryAllocationDebug)
    {
        // If enabled (debug layers), we keep more details in the memory tracker, such as handle,
        // and log the action to the output.
        std::unique_lock<angle::SimpleMutex> lock(mMemoryAllocationMutex);

        uint32_t allocTypeIndex = ToUnderlying(allocType);
        uint32_t memoryHeapIndex =
            mRenderer->getMemoryProperties().getHeapIndexForMemoryType(memoryTypeIndex);
        mActiveMemoryAllocationsCount[allocTypeIndex]++;
        mActiveMemoryAllocationsSize[allocTypeIndex] += size;
        mActivePerHeapMemoryAllocationsCount[allocTypeIndex][memoryHeapIndex]++;
        mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex] += size;

        // Add the new allocation to the memory tracker.
        vk::MemoryAllocationInfo memAllocLogInfo;
        memAllocLogInfo.id              = ++mMemoryAllocationID;
        memAllocLogInfo.allocType       = allocType;
        memAllocLogInfo.memoryHeapIndex = memoryHeapIndex;
        memAllocLogInfo.size            = size;
        memAllocLogInfo.handle          = handle;

        vk::MemoryAllocInfoMapKey memoryAllocInfoMapKey(memAllocLogInfo.handle);
        mMemoryAllocationRecord[angle::getBacktraceInfo()].insert(
            std::make_pair(memoryAllocInfoMapKey, memAllocLogInfo));

        INFO() << "Memory allocation: (id " << memAllocLogInfo.id << ") for object "
               << memAllocLogInfo.handle << " | Size: " << memAllocLogInfo.size
               << " | Type: " << vk::kMemoryAllocationTypeMessage[allocTypeIndex]
               << " | Memory type index: " << memoryTypeIndex
               << " | Heap index: " << memAllocLogInfo.memoryHeapIndex;

        resetPendingMemoryAlloc();
    }
    else if (kTrackMemoryAllocationSizes)
    {
        // Add the new allocation size to the allocation counter.
        uint32_t allocTypeIndex = ToUnderlying(allocType);
        mActiveMemoryAllocationsCount[allocTypeIndex]++;
        mActiveMemoryAllocationsSize[allocTypeIndex] += size;

        uint32_t memoryHeapIndex =
            mRenderer->getMemoryProperties().getHeapIndexForMemoryType(memoryTypeIndex);
        mActivePerHeapMemoryAllocationsCount[allocTypeIndex][memoryHeapIndex].fetch_add(
            1, std::memory_order_relaxed);
        mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex].fetch_add(
            size, std::memory_order_relaxed);

        resetPendingMemoryAlloc();
    }
}

void MemoryAllocationTracker::onMemoryDeallocImpl(vk::MemoryAllocationType allocType,
                                                  VkDeviceSize size,
                                                  uint32_t memoryTypeIndex,
                                                  void *handle)
{
    ASSERT(allocType != vk::MemoryAllocationType::InvalidEnum && size != 0);

    if (kTrackMemoryAllocationDebug)
    {
        // If enabled (debug layers), we keep more details in the memory tracker, such as handle,
        // and log the action to the output. The memory allocation tracker uses the backtrace info
        // as key, if available.
        for (auto &memInfoPerBacktrace : mMemoryAllocationRecord)
        {
            vk::MemoryAllocInfoMapKey memoryAllocInfoMapKey(handle);
            MemoryAllocInfoMap &memInfoMap = memInfoPerBacktrace.second;
            std::unique_lock<angle::SimpleMutex> lock(mMemoryAllocationMutex);

            if (memInfoMap.find(memoryAllocInfoMapKey) != memInfoMap.end())
            {
                // Object found; remove it from the allocation tracker.
                vk::MemoryAllocationInfo *memInfoEntry = &memInfoMap[memoryAllocInfoMapKey];
                ASSERT(memInfoEntry->allocType == allocType && memInfoEntry->size == size);

                uint32_t allocTypeIndex = ToUnderlying(memInfoEntry->allocType);
                uint32_t memoryHeapIndex =
                    mRenderer->getMemoryProperties().getHeapIndexForMemoryType(memoryTypeIndex);
                ASSERT(mActiveMemoryAllocationsCount[allocTypeIndex] != 0 &&
                       mActiveMemoryAllocationsSize[allocTypeIndex] >= size);
                ASSERT(memoryHeapIndex == memInfoEntry->memoryHeapIndex &&
                       mActivePerHeapMemoryAllocationsCount[allocTypeIndex][memoryHeapIndex] != 0 &&
                       mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex] >=
                           size);
                mActiveMemoryAllocationsCount[allocTypeIndex]--;
                mActiveMemoryAllocationsSize[allocTypeIndex] -= size;
                mActivePerHeapMemoryAllocationsCount[allocTypeIndex][memoryHeapIndex]--;
                mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex] -= size;

                INFO() << "Memory deallocation: (id " << memInfoEntry->id << ") for object "
                       << memInfoEntry->handle << " | Size: " << memInfoEntry->size
                       << " | Type: " << vk::kMemoryAllocationTypeMessage[allocTypeIndex]
                       << " | Memory type index: " << memoryTypeIndex
                       << " | Heap index: " << memInfoEntry->memoryHeapIndex;

                memInfoMap.erase(memoryAllocInfoMapKey);
            }
        }
    }
    else if (kTrackMemoryAllocationSizes)
    {
        // Remove the allocation size from the allocation counter.
        uint32_t allocTypeIndex = ToUnderlying(allocType);
        ASSERT(mActiveMemoryAllocationsCount[allocTypeIndex] != 0 &&
               mActiveMemoryAllocationsSize[allocTypeIndex] >= size);
        mActiveMemoryAllocationsCount[allocTypeIndex]--;
        mActiveMemoryAllocationsSize[allocTypeIndex] -= size;

        uint32_t memoryHeapIndex =
            mRenderer->getMemoryProperties().getHeapIndexForMemoryType(memoryTypeIndex);
        ASSERT(mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex] >= size);
        mActivePerHeapMemoryAllocationsCount[allocTypeIndex][memoryHeapIndex].fetch_add(
            -1, std::memory_order_relaxed);
        mActivePerHeapMemoryAllocationsSize[allocTypeIndex][memoryHeapIndex].fetch_add(
            -size, std::memory_order_relaxed);
    }
}

VkDeviceSize MemoryAllocationTracker::getActiveMemoryAllocationsSize(uint32_t allocTypeIndex) const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    ASSERT(allocTypeIndex < vk::kMemoryAllocationTypeCount);
    return mActiveMemoryAllocationsSize[allocTypeIndex];
}

VkDeviceSize MemoryAllocationTracker::getActiveHeapMemoryAllocationsSize(uint32_t allocTypeIndex,
                                                                         uint32_t heapIndex) const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    ASSERT(allocTypeIndex < vk::kMemoryAllocationTypeCount &&
           heapIndex < mRenderer->getMemoryProperties().getMemoryHeapCount());
    return mActivePerHeapMemoryAllocationsSize[allocTypeIndex][heapIndex];
}

uint64_t MemoryAllocationTracker::getActiveMemoryAllocationsCount(uint32_t allocTypeIndex) const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    ASSERT(allocTypeIndex < vk::kMemoryAllocationTypeCount);
    return mActiveMemoryAllocationsCount[allocTypeIndex];
}

uint64_t MemoryAllocationTracker::getActiveHeapMemoryAllocationsCount(uint32_t allocTypeIndex,
                                                                      uint32_t heapIndex) const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    ASSERT(allocTypeIndex < vk::kMemoryAllocationTypeCount &&
           heapIndex < mRenderer->getMemoryProperties().getMemoryHeapCount());
    return mActivePerHeapMemoryAllocationsCount[allocTypeIndex][heapIndex];
}

void MemoryAllocationTracker::compareExpectedFlagsWithAllocatedFlags(
    VkMemoryPropertyFlags requiredFlags,
    VkMemoryPropertyFlags preferredFlags,
    VkMemoryPropertyFlags allocatedFlags,
    void *handle)
{
    if (!kTrackMemoryAllocationDebug)
    {
        return;
    }

    ASSERT((requiredFlags & ~allocatedFlags) == 0);
    if (((preferredFlags | requiredFlags) & ~allocatedFlags) != 0)
    {
        INFO() << "Memory type index chosen for object " << handle
               << " lacks some of the preferred property flags.";
    }

    if ((~allocatedFlags & preferredFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
    {
        WARN() << "Device-local memory allocation fallback to system memory.";
    }
}

void MemoryAllocationTracker::onExceedingMaxMemoryAllocationSize(VkDeviceSize size)
{
    VkDeviceSize maxAllocationSize = mRenderer->getMaxMemoryAllocationSize();
    ASSERT(size > maxAllocationSize);

    WARN() << "Attempted allocation size (" << size
           << ") is greater than the maximum allocation size allowed (" << maxAllocationSize
           << ").";
}

VkDeviceSize MemoryAllocationTracker::getPendingMemoryAllocationSize() const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    return mPendingMemoryAllocationSize;
}

vk::MemoryAllocationType MemoryAllocationTracker::getPendingMemoryAllocationType() const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return vk::MemoryAllocationType::Unspecified;
    }

    return mPendingMemoryAllocationType;
}

uint32_t MemoryAllocationTracker::getPendingMemoryTypeIndex() const
{
    if (!kTrackMemoryAllocationSizes)
    {
        return 0;
    }

    return mPendingMemoryTypeIndex;
}

void MemoryAllocationTracker::setPendingMemoryAlloc(vk::MemoryAllocationType allocType,
                                                    VkDeviceSize size,
                                                    uint32_t memoryTypeIndex)
{
    if (!kTrackMemoryAllocationSizes)
    {
        return;
    }

    ASSERT(allocType != vk::MemoryAllocationType::InvalidEnum && size != 0);
    mPendingMemoryAllocationType = allocType;
    mPendingMemoryAllocationSize = size;
    mPendingMemoryTypeIndex      = memoryTypeIndex;
}

void MemoryAllocationTracker::resetPendingMemoryAlloc()
{
    if (!kTrackMemoryAllocationSizes)
    {
        return;
    }

    mPendingMemoryAllocationType = vk::MemoryAllocationType::Unspecified;
    mPendingMemoryAllocationSize = 0;
    mPendingMemoryTypeIndex      = kInvalidMemoryTypeIndex;
}

namespace vk
{
MemoryReport::MemoryReport()
    : mCurrentTotalAllocatedMemory(0),
      mMaxTotalAllocatedMemory(0),
      mCurrentTotalImportedMemory(0),
      mMaxTotalImportedMemory(0)
{}

void MemoryReport::processCallback(const VkDeviceMemoryReportCallbackDataEXT &callbackData,
                                   bool logCallback)
{
    std::unique_lock<angle::SimpleMutex> lock(mMemoryReportMutex);
    VkDeviceSize size = 0;
    std::string reportType;
    switch (callbackData.type)
    {
        case VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATE_EXT:
            reportType = "Allocate";
            if ((mUniqueIDCounts[callbackData.memoryObjectId] += 1) > 1)
            {
                break;
            }
            size = mSizesPerType[callbackData.objectType].allocatedMemory + callbackData.size;
            mSizesPerType[callbackData.objectType].allocatedMemory = size;
            if (mSizesPerType[callbackData.objectType].allocatedMemoryMax < size)
            {
                mSizesPerType[callbackData.objectType].allocatedMemoryMax = size;
            }
            mCurrentTotalAllocatedMemory += callbackData.size;
            if (mMaxTotalAllocatedMemory < mCurrentTotalAllocatedMemory)
            {
                mMaxTotalAllocatedMemory = mCurrentTotalAllocatedMemory;
            }
            break;
        case VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_FREE_EXT:
            reportType = "Free";
            ASSERT(mUniqueIDCounts[callbackData.memoryObjectId] > 0);
            mUniqueIDCounts[callbackData.memoryObjectId] -= 1;
            size = mSizesPerType[callbackData.objectType].allocatedMemory - callbackData.size;
            mSizesPerType[callbackData.objectType].allocatedMemory = size;
            mCurrentTotalAllocatedMemory -= callbackData.size;
            break;
        case VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_IMPORT_EXT:
            reportType = "Import";
            if ((mUniqueIDCounts[callbackData.memoryObjectId] += 1) > 1)
            {
                break;
            }
            size = mSizesPerType[callbackData.objectType].importedMemory + callbackData.size;
            mSizesPerType[callbackData.objectType].importedMemory = size;
            if (mSizesPerType[callbackData.objectType].importedMemoryMax < size)
            {
                mSizesPerType[callbackData.objectType].importedMemoryMax = size;
            }
            mCurrentTotalImportedMemory += callbackData.size;
            if (mMaxTotalImportedMemory < mCurrentTotalImportedMemory)
            {
                mMaxTotalImportedMemory = mCurrentTotalImportedMemory;
            }
            break;
        case VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_UNIMPORT_EXT:
            reportType = "Un-Import";
            ASSERT(mUniqueIDCounts[callbackData.memoryObjectId] > 0);
            mUniqueIDCounts[callbackData.memoryObjectId] -= 1;
            size = mSizesPerType[callbackData.objectType].importedMemory - callbackData.size;
            mSizesPerType[callbackData.objectType].importedMemory = size;
            mCurrentTotalImportedMemory -= callbackData.size;
            break;
        case VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATION_FAILED_EXT:
            reportType = "allocFail";
            break;
        default:
            UNREACHABLE();
            return;
    }
    if (logCallback)
    {
        INFO() << std::right << std::setw(9) << reportType << ": size=" << std::setw(10)
               << callbackData.size << "; type=" << std::setw(15) << std::left
               << Renderer::GetVulkanObjectTypeName(callbackData.objectType)
               << "; heapIdx=" << callbackData.heapIndex << "; id=" << std::hex
               << callbackData.memoryObjectId << "; handle=" << std::hex
               << callbackData.objectHandle << ": Total=" << std::right << std::setw(10) << std::dec
               << size;
    }
}

void MemoryReport::logMemoryReportStats() const
{
    std::unique_lock<angle::SimpleMutex> lock(mMemoryReportMutex);

    INFO() << std::right << "GPU Memory Totals:       Allocated=" << std::setw(10)
           << mCurrentTotalAllocatedMemory << " (max=" << std::setw(10) << mMaxTotalAllocatedMemory
           << ");  Imported=" << std::setw(10) << mCurrentTotalImportedMemory
           << " (max=" << std::setw(10) << mMaxTotalImportedMemory << ")";
    INFO() << "Sub-Totals per type:";
    for (const auto &it : mSizesPerType)
    {
        VkObjectType objectType         = it.first;
        MemorySizes memorySizes         = it.second;
        VkDeviceSize allocatedMemory    = memorySizes.allocatedMemory;
        VkDeviceSize allocatedMemoryMax = memorySizes.allocatedMemoryMax;
        VkDeviceSize importedMemory     = memorySizes.importedMemory;
        VkDeviceSize importedMemoryMax  = memorySizes.importedMemoryMax;
        INFO() << std::right << "- Type=" << std::setw(15)
               << Renderer::GetVulkanObjectTypeName(objectType) << ":  Allocated=" << std::setw(10)
               << allocatedMemory << " (max=" << std::setw(10) << allocatedMemoryMax
               << ");  Imported=" << std::setw(10) << importedMemory << " (max=" << std::setw(10)
               << importedMemoryMax << ")";
    }
}
}  // namespace vk
}  // namespace rx
