//
// Copyright (c) 2017-2021 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Tests.h"
#include "VmaUsage.h"
#include "Common.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

#ifdef _WIN32

static const char* CODE_DESCRIPTION = "Foo";

extern VkCommandBuffer g_hTemporaryCommandBuffer;
extern const VkAllocationCallbacks* g_Allocs;
extern bool VK_KHR_buffer_device_address_enabled;
extern bool VK_EXT_memory_priority_enabled;
extern PFN_vkGetBufferDeviceAddressKHR g_vkGetBufferDeviceAddressKHR;
void BeginSingleTimeCommands();
void EndSingleTimeCommands();
void SetDebugUtilsObjectName(VkObjectType type, uint64_t handle, const char* name);

#ifndef VMA_DEBUG_MARGIN
    #define VMA_DEBUG_MARGIN 0
#endif

enum CONFIG_TYPE {
    CONFIG_TYPE_MINIMUM,
    CONFIG_TYPE_SMALL,
    CONFIG_TYPE_AVERAGE,
    CONFIG_TYPE_LARGE,
    CONFIG_TYPE_MAXIMUM,
    CONFIG_TYPE_COUNT
};

static constexpr CONFIG_TYPE ConfigType = CONFIG_TYPE_SMALL;
//static constexpr CONFIG_TYPE ConfigType = CONFIG_TYPE_LARGE;

enum class FREE_ORDER { FORWARD, BACKWARD, RANDOM, COUNT };

static const char* FREE_ORDER_NAMES[] = {
    "FORWARD",
    "BACKWARD",
    "RANDOM",
};

// Copy of internal VmaAlgorithmToStr.
static const char* AlgorithmToStr(uint32_t algorithm)
{
    switch(algorithm)
    {
    case VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT:
        return "Linear";
    case VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT:
        return "Buddy";
    case 0:
        return "Default";
    default:
        assert(0);
        return "";
    }
}

struct AllocationSize
{
    uint32_t Probability;
    VkDeviceSize BufferSizeMin, BufferSizeMax;
    uint32_t ImageSizeMin, ImageSizeMax;
};

struct Config
{
    uint32_t RandSeed;
    VkDeviceSize BeginBytesToAllocate;
    uint32_t AdditionalOperationCount;
    VkDeviceSize MaxBytesToAllocate;
    uint32_t MemUsageProbability[4]; // For VMA_MEMORY_USAGE_*
    std::vector<AllocationSize> AllocationSizes;
    uint32_t ThreadCount;
    uint32_t ThreadsUsingCommonAllocationsProbabilityPercent;
    FREE_ORDER FreeOrder;
    VmaAllocationCreateFlags AllocationStrategy; // For VMA_ALLOCATION_CREATE_STRATEGY_*
};

struct Result
{
    duration TotalTime;
    duration AllocationTimeMin, AllocationTimeAvg, AllocationTimeMax;
    duration DeallocationTimeMin, DeallocationTimeAvg, DeallocationTimeMax;
    VkDeviceSize TotalMemoryAllocated;
    VkDeviceSize FreeRangeSizeAvg, FreeRangeSizeMax;
};

void TestDefragmentationSimple();
void TestDefragmentationFull();

struct PoolTestConfig
{
    uint32_t RandSeed;
    uint32_t ThreadCount;
    VkDeviceSize PoolSize;
    uint32_t FrameCount;
    uint32_t TotalItemCount;
    // Range for number of items used in each frame.
    uint32_t UsedItemCountMin, UsedItemCountMax;
    // Percent of items to make unused, and possibly make some others used in each frame.
    uint32_t ItemsToMakeUnusedPercent;
    std::vector<AllocationSize> AllocationSizes;

    VkDeviceSize CalcAvgResourceSize() const
    {
        uint32_t probabilitySum = 0;
        VkDeviceSize sizeSum = 0;
        for(size_t i = 0; i < AllocationSizes.size(); ++i)
        {
            const AllocationSize& allocSize = AllocationSizes[i];
            if(allocSize.BufferSizeMax > 0)
                sizeSum += (allocSize.BufferSizeMin + allocSize.BufferSizeMax) / 2 * allocSize.Probability;
            else
            {
                const VkDeviceSize avgDimension = (allocSize.ImageSizeMin + allocSize.ImageSizeMax) / 2;
                sizeSum += avgDimension * avgDimension * 4 * allocSize.Probability;
            }
            probabilitySum += allocSize.Probability;
        }
        return sizeSum / probabilitySum;
    }

    bool UsesBuffers() const
    {
        for(size_t i = 0; i < AllocationSizes.size(); ++i)
            if(AllocationSizes[i].BufferSizeMax > 0)
                return true;
        return false;
    }

    bool UsesImages() const
    {
        for(size_t i = 0; i < AllocationSizes.size(); ++i)
            if(AllocationSizes[i].ImageSizeMax > 0)
                return true;
        return false;
    }
};

struct PoolTestResult
{
    duration TotalTime;
    duration AllocationTimeMin, AllocationTimeAvg, AllocationTimeMax;
    duration DeallocationTimeMin, DeallocationTimeAvg, DeallocationTimeMax;
    size_t LostAllocationCount, LostAllocationTotalSize;
    size_t FailedAllocationCount, FailedAllocationTotalSize;
};

static const uint32_t IMAGE_BYTES_PER_PIXEL = 1;

uint32_t g_FrameIndex = 0;

struct BufferInfo
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
};

static uint32_t MemoryTypeToHeap(uint32_t memoryTypeIndex)
{
    const VkPhysicalDeviceMemoryProperties* props;
    vmaGetMemoryProperties(g_hAllocator, &props);
    return props->memoryTypes[memoryTypeIndex].heapIndex;
}

static uint32_t GetAllocationStrategyCount()
{
    uint32_t strategyCount = 0;
    switch(ConfigType)
    {
    case CONFIG_TYPE_MINIMUM: strategyCount = 1; break;
    case CONFIG_TYPE_SMALL:   strategyCount = 1; break;
    case CONFIG_TYPE_AVERAGE: strategyCount = 2; break;
    case CONFIG_TYPE_LARGE:   strategyCount = 2; break;
    case CONFIG_TYPE_MAXIMUM: strategyCount = 3; break;
    default: assert(0);
    }
    return strategyCount;
}

static const char* GetAllocationStrategyName(VmaAllocationCreateFlags allocStrategy)
{
    switch(allocStrategy)
    {
    case VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT: return "BEST_FIT"; break;
    case VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT: return "WORST_FIT"; break;
    case VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT: return "FIRST_FIT"; break;
    case 0: return "Default"; break;
    default: assert(0); return "";   
    }
}

static void InitResult(Result& outResult)
{
    outResult.TotalTime = duration::zero();
    outResult.AllocationTimeMin = duration::max();
    outResult.AllocationTimeAvg = duration::zero();
    outResult.AllocationTimeMax = duration::min();
    outResult.DeallocationTimeMin = duration::max();
    outResult.DeallocationTimeAvg = duration::zero();
    outResult.DeallocationTimeMax = duration::min();
    outResult.TotalMemoryAllocated = 0;
    outResult.FreeRangeSizeAvg = 0;
    outResult.FreeRangeSizeMax = 0;
}

class TimeRegisterObj
{
public:
    TimeRegisterObj(duration& min, duration& sum, duration& max) :
        m_Min(min),
        m_Sum(sum),
        m_Max(max),
        m_TimeBeg(std::chrono::high_resolution_clock::now())
    {
    }

    ~TimeRegisterObj()
    {
        duration d = std::chrono::high_resolution_clock::now() - m_TimeBeg;
        m_Sum += d;
        if(d < m_Min) m_Min = d;
        if(d > m_Max) m_Max = d;
    }

private:
    duration& m_Min;
    duration& m_Sum;
    duration& m_Max;
    time_point m_TimeBeg;
};

struct PoolTestThreadResult
{
    duration AllocationTimeMin, AllocationTimeSum, AllocationTimeMax;
    duration DeallocationTimeMin, DeallocationTimeSum, DeallocationTimeMax;
    size_t AllocationCount, DeallocationCount;
    size_t LostAllocationCount, LostAllocationTotalSize;
    size_t FailedAllocationCount, FailedAllocationTotalSize;
};

class AllocationTimeRegisterObj : public TimeRegisterObj
{
public:
    AllocationTimeRegisterObj(Result& result) :
        TimeRegisterObj(result.AllocationTimeMin, result.AllocationTimeAvg, result.AllocationTimeMax)
    {
    }
};

class DeallocationTimeRegisterObj : public TimeRegisterObj
{
public:
    DeallocationTimeRegisterObj(Result& result) :
        TimeRegisterObj(result.DeallocationTimeMin, result.DeallocationTimeAvg, result.DeallocationTimeMax)
    {
    }
};

class PoolAllocationTimeRegisterObj : public TimeRegisterObj
{
public:
    PoolAllocationTimeRegisterObj(PoolTestThreadResult& result) :
        TimeRegisterObj(result.AllocationTimeMin, result.AllocationTimeSum, result.AllocationTimeMax)
    {
    }
};

class PoolDeallocationTimeRegisterObj : public TimeRegisterObj
{
public:
    PoolDeallocationTimeRegisterObj(PoolTestThreadResult& result) :
        TimeRegisterObj(result.DeallocationTimeMin, result.DeallocationTimeSum, result.DeallocationTimeMax)
    {
    }
};

static void CurrentTimeToStr(std::string& out)
{
    time_t rawTime; time(&rawTime);
    struct tm timeInfo; localtime_s(&timeInfo, &rawTime);
    char timeStr[128];
    strftime(timeStr, _countof(timeStr), "%c", &timeInfo);
    out = timeStr;
}

VkResult MainTest(Result& outResult, const Config& config)
{
    assert(config.ThreadCount > 0);

    InitResult(outResult);

    RandomNumberGenerator mainRand{config.RandSeed};

    time_point timeBeg = std::chrono::high_resolution_clock::now();

    std::atomic<size_t> allocationCount = 0;
    VkResult res = VK_SUCCESS;

    uint32_t memUsageProbabilitySum =
        config.MemUsageProbability[0] + config.MemUsageProbability[1] +
        config.MemUsageProbability[2] + config.MemUsageProbability[3];
    assert(memUsageProbabilitySum > 0);

    uint32_t allocationSizeProbabilitySum = std::accumulate(
        config.AllocationSizes.begin(),
        config.AllocationSizes.end(),
        0u,
        [](uint32_t sum, const AllocationSize& allocSize) {
            return sum + allocSize.Probability;
        });

    struct Allocation
    {
        VkBuffer Buffer;
        VkImage Image;
        VmaAllocation Alloc;
    };

    std::vector<Allocation> commonAllocations;
    std::mutex commonAllocationsMutex;

    auto Allocate = [&](
        VkDeviceSize bufferSize,
        const VkExtent2D imageExtent,
        RandomNumberGenerator& localRand,
        VkDeviceSize& totalAllocatedBytes,
        std::vector<Allocation>& allocations) -> VkResult
    {
        assert((bufferSize == 0) != (imageExtent.width == 0 && imageExtent.height == 0));

        uint32_t memUsageIndex = 0;
        uint32_t memUsageRand = localRand.Generate() % memUsageProbabilitySum;
        while(memUsageRand >= config.MemUsageProbability[memUsageIndex])
            memUsageRand -= config.MemUsageProbability[memUsageIndex++];

        VmaAllocationCreateInfo memReq = {};
        memReq.usage = (VmaMemoryUsage)(VMA_MEMORY_USAGE_GPU_ONLY + memUsageIndex);
        memReq.flags |= config.AllocationStrategy;

        Allocation allocation = {};
        VmaAllocationInfo allocationInfo;

        // Buffer
        if(bufferSize > 0)
        {
            assert(imageExtent.width == 0);
            VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

            {
                AllocationTimeRegisterObj timeRegisterObj{outResult};
                res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &memReq, &allocation.Buffer, &allocation.Alloc, &allocationInfo);
            }
        }
        // Image
        else
        {
            VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = imageExtent.width;
            imageInfo.extent.height = imageExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.tiling = memReq.usage == VMA_MEMORY_USAGE_GPU_ONLY ?
                VK_IMAGE_TILING_OPTIMAL :
                VK_IMAGE_TILING_LINEAR;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            switch(memReq.usage)
            {
            case VMA_MEMORY_USAGE_GPU_ONLY:
                switch(localRand.Generate() % 3)
                {
                case 0:
                    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    break;
                case 1:
                    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    break;
                case 2:
                    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                    break;
                }
                break;
            case VMA_MEMORY_USAGE_CPU_ONLY:
            case VMA_MEMORY_USAGE_CPU_TO_GPU:
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                break;
            case VMA_MEMORY_USAGE_GPU_TO_CPU:
                imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                break;
            }
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.flags = 0;

            {
                AllocationTimeRegisterObj timeRegisterObj{outResult};
                res = vmaCreateImage(g_hAllocator, &imageInfo, &memReq, &allocation.Image, &allocation.Alloc, &allocationInfo);
            }
        }

        if(res == VK_SUCCESS)
        {
            ++allocationCount;
            totalAllocatedBytes += allocationInfo.size;
            bool useCommonAllocations = localRand.Generate() % 100 < config.ThreadsUsingCommonAllocationsProbabilityPercent;
            if(useCommonAllocations)
            {
                std::unique_lock<std::mutex> lock(commonAllocationsMutex);
                commonAllocations.push_back(allocation);
            }
            else
                allocations.push_back(allocation);
        }
        else
        {
            TEST(0);
        }
        return res;
    };

    auto GetNextAllocationSize = [&](
        VkDeviceSize& outBufSize,
        VkExtent2D& outImageSize,
        RandomNumberGenerator& localRand)
    {
        outBufSize = 0;
        outImageSize = {0, 0};

        uint32_t allocSizeIndex = 0;
        uint32_t r = localRand.Generate() % allocationSizeProbabilitySum;
        while(r >= config.AllocationSizes[allocSizeIndex].Probability)
            r -= config.AllocationSizes[allocSizeIndex++].Probability;

        const AllocationSize& allocSize = config.AllocationSizes[allocSizeIndex];
        if(allocSize.BufferSizeMax > 0)
        {
            assert(allocSize.ImageSizeMax == 0);
            if(allocSize.BufferSizeMax == allocSize.BufferSizeMin)
                outBufSize = allocSize.BufferSizeMin;
            else
            {
                outBufSize = allocSize.BufferSizeMin + localRand.Generate() % (allocSize.BufferSizeMax - allocSize.BufferSizeMin);
                outBufSize = outBufSize / 16 * 16;
            }
        }
        else
        {
            if(allocSize.ImageSizeMax == allocSize.ImageSizeMin)
                outImageSize.width = outImageSize.height = allocSize.ImageSizeMax;
            else
            {
                outImageSize.width  = allocSize.ImageSizeMin + localRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
                outImageSize.height = allocSize.ImageSizeMin + localRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
            }
        }
    };

    std::atomic<uint32_t> numThreadsReachedMaxAllocations = 0;
    HANDLE threadsFinishEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    auto ThreadProc = [&](uint32_t randSeed) -> void
    {
        RandomNumberGenerator threadRand(randSeed);
        VkDeviceSize threadTotalAllocatedBytes = 0;
        std::vector<Allocation> threadAllocations;
        VkDeviceSize threadBeginBytesToAllocate = config.BeginBytesToAllocate / config.ThreadCount;
        VkDeviceSize threadMaxBytesToAllocate = config.MaxBytesToAllocate / config.ThreadCount;
        uint32_t threadAdditionalOperationCount = config.AdditionalOperationCount / config.ThreadCount;

        // BEGIN ALLOCATIONS
        for(;;)
        {
            VkDeviceSize bufferSize = 0;
            VkExtent2D imageExtent = {};
            GetNextAllocationSize(bufferSize, imageExtent, threadRand);
            if(threadTotalAllocatedBytes + bufferSize + imageExtent.width * imageExtent.height * IMAGE_BYTES_PER_PIXEL <
                threadBeginBytesToAllocate)
            {
                if(Allocate(bufferSize, imageExtent, threadRand, threadTotalAllocatedBytes, threadAllocations) != VK_SUCCESS)
                    break;
            }
            else
                break;
        }

        // ADDITIONAL ALLOCATIONS AND FREES
        for(size_t i = 0; i < threadAdditionalOperationCount; ++i)
        {
            VkDeviceSize bufferSize = 0;
            VkExtent2D imageExtent = {};
            GetNextAllocationSize(bufferSize, imageExtent, threadRand);

            // true = allocate, false = free
            bool allocate = threadRand.Generate() % 2 != 0;

            if(allocate)
            {
                if(threadTotalAllocatedBytes +
                    bufferSize +
                    imageExtent.width * imageExtent.height * IMAGE_BYTES_PER_PIXEL <
                    threadMaxBytesToAllocate)
                {
                    if(Allocate(bufferSize, imageExtent, threadRand, threadTotalAllocatedBytes, threadAllocations) != VK_SUCCESS)
                        break;
                }
            }
            else
            {
                bool useCommonAllocations = threadRand.Generate() % 100 < config.ThreadsUsingCommonAllocationsProbabilityPercent;
                if(useCommonAllocations)
                {
                    std::unique_lock<std::mutex> lock(commonAllocationsMutex);
                    if(!commonAllocations.empty())
                    {
                        size_t indexToFree = threadRand.Generate() % commonAllocations.size();
                        VmaAllocationInfo allocationInfo;
                        vmaGetAllocationInfo(g_hAllocator, commonAllocations[indexToFree].Alloc, &allocationInfo);
                        if(threadTotalAllocatedBytes >= allocationInfo.size)
                        {
                            DeallocationTimeRegisterObj timeRegisterObj{outResult};
                            if(commonAllocations[indexToFree].Buffer != VK_NULL_HANDLE)
                                vmaDestroyBuffer(g_hAllocator, commonAllocations[indexToFree].Buffer, commonAllocations[indexToFree].Alloc);
                            else
                                vmaDestroyImage(g_hAllocator, commonAllocations[indexToFree].Image, commonAllocations[indexToFree].Alloc);
                            threadTotalAllocatedBytes -= allocationInfo.size;
                            commonAllocations.erase(commonAllocations.begin() + indexToFree);
                        }
                    }
                }
                else
                {
                    if(!threadAllocations.empty())
                    {
                        size_t indexToFree = threadRand.Generate() % threadAllocations.size();
                        VmaAllocationInfo allocationInfo;
                        vmaGetAllocationInfo(g_hAllocator, threadAllocations[indexToFree].Alloc, &allocationInfo);
                        if(threadTotalAllocatedBytes >= allocationInfo.size)
                        {
                            DeallocationTimeRegisterObj timeRegisterObj{outResult};
                            if(threadAllocations[indexToFree].Buffer != VK_NULL_HANDLE)
                                vmaDestroyBuffer(g_hAllocator, threadAllocations[indexToFree].Buffer, threadAllocations[indexToFree].Alloc);
                            else
                                vmaDestroyImage(g_hAllocator, threadAllocations[indexToFree].Image, threadAllocations[indexToFree].Alloc);
                            threadTotalAllocatedBytes -= allocationInfo.size;
                            threadAllocations.erase(threadAllocations.begin() + indexToFree);
                        }
                    }
                }
            }
        }

        ++numThreadsReachedMaxAllocations;

        WaitForSingleObject(threadsFinishEvent, INFINITE);

        // DEALLOCATION
        while(!threadAllocations.empty())
        {
            size_t indexToFree = 0;
            switch(config.FreeOrder)
            {
            case FREE_ORDER::FORWARD:
                indexToFree = 0;
                break;
            case FREE_ORDER::BACKWARD:
                indexToFree = threadAllocations.size() - 1;
                break;
            case FREE_ORDER::RANDOM:
                indexToFree = mainRand.Generate() % threadAllocations.size();
                break;
            }

            {
                DeallocationTimeRegisterObj timeRegisterObj{outResult};
                if(threadAllocations[indexToFree].Buffer != VK_NULL_HANDLE)
                    vmaDestroyBuffer(g_hAllocator, threadAllocations[indexToFree].Buffer, threadAllocations[indexToFree].Alloc);
                else
                    vmaDestroyImage(g_hAllocator, threadAllocations[indexToFree].Image, threadAllocations[indexToFree].Alloc);
            }
            threadAllocations.erase(threadAllocations.begin() + indexToFree);
        }
    };

    uint32_t threadRandSeed = mainRand.Generate();
    std::vector<std::thread> bkgThreads;
    for(size_t i = 0; i < config.ThreadCount; ++i)
    {
        bkgThreads.emplace_back(std::bind(ThreadProc, threadRandSeed + (uint32_t)i));
    }
    
    // Wait for threads reached max allocations
    while(numThreadsReachedMaxAllocations < config.ThreadCount)
        Sleep(0);

    // CALCULATE MEMORY STATISTICS ON FINAL USAGE
    VmaStats vmaStats = {};
    vmaCalculateStats(g_hAllocator, &vmaStats);
    outResult.TotalMemoryAllocated = vmaStats.total.usedBytes + vmaStats.total.unusedBytes;
    outResult.FreeRangeSizeMax = vmaStats.total.unusedRangeSizeMax;
    outResult.FreeRangeSizeAvg = vmaStats.total.unusedRangeSizeAvg;

    // Signal threads to deallocate
    SetEvent(threadsFinishEvent);

    // Wait for threads finished
    for(size_t i = 0; i < bkgThreads.size(); ++i)
        bkgThreads[i].join();
    bkgThreads.clear();

    CloseHandle(threadsFinishEvent);

    // Deallocate remaining common resources
    while(!commonAllocations.empty())
    {
        size_t indexToFree = 0;
        switch(config.FreeOrder)
        {
        case FREE_ORDER::FORWARD:
            indexToFree = 0;
            break;
        case FREE_ORDER::BACKWARD:
            indexToFree = commonAllocations.size() - 1;
            break;
        case FREE_ORDER::RANDOM:
            indexToFree = mainRand.Generate() % commonAllocations.size();
            break;
        }

        {
            DeallocationTimeRegisterObj timeRegisterObj{outResult};
            if(commonAllocations[indexToFree].Buffer != VK_NULL_HANDLE)
                vmaDestroyBuffer(g_hAllocator, commonAllocations[indexToFree].Buffer, commonAllocations[indexToFree].Alloc);
            else
                vmaDestroyImage(g_hAllocator, commonAllocations[indexToFree].Image, commonAllocations[indexToFree].Alloc);
        }
        commonAllocations.erase(commonAllocations.begin() + indexToFree);
    }

    if(allocationCount)
    {
        outResult.AllocationTimeAvg /= allocationCount;
        outResult.DeallocationTimeAvg /= allocationCount;
    }

    outResult.TotalTime = std::chrono::high_resolution_clock::now() - timeBeg;

    return res;
}

void SaveAllocatorStatsToFile(const wchar_t* filePath)
{
    wprintf(L"Saving JSON dump to file \"%s\"\n", filePath);
    char* stats;
    vmaBuildStatsString(g_hAllocator, &stats, VK_TRUE);
    SaveFile(filePath, stats, strlen(stats));
    vmaFreeStatsString(g_hAllocator, stats);
}

struct AllocInfo
{
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkImage m_Image = VK_NULL_HANDLE;
    VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t m_StartValue = 0;
    union
    {
        VkBufferCreateInfo m_BufferInfo;
        VkImageCreateInfo m_ImageInfo;
    };

    // After defragmentation.
    VkBuffer m_NewBuffer = VK_NULL_HANDLE;
    VkImage m_NewImage = VK_NULL_HANDLE;

    void CreateBuffer(
        const VkBufferCreateInfo& bufCreateInfo,
        const VmaAllocationCreateInfo& allocCreateInfo);
    void CreateImage(
        const VkImageCreateInfo& imageCreateInfo,
        const VmaAllocationCreateInfo& allocCreateInfo,
        VkImageLayout layout);
    void Destroy();
};

void AllocInfo::CreateBuffer(
    const VkBufferCreateInfo& bufCreateInfo,
    const VmaAllocationCreateInfo& allocCreateInfo)
{
    m_BufferInfo = bufCreateInfo;
    VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &m_Buffer, &m_Allocation, nullptr);
    TEST(res == VK_SUCCESS);
}
void AllocInfo::CreateImage(
    const VkImageCreateInfo& imageCreateInfo,
    const VmaAllocationCreateInfo& allocCreateInfo,
    VkImageLayout layout)
{
    m_ImageInfo = imageCreateInfo;
    m_ImageLayout = layout;
    VkResult res = vmaCreateImage(g_hAllocator, &imageCreateInfo, &allocCreateInfo, &m_Image, &m_Allocation, nullptr);
    TEST(res == VK_SUCCESS);
}

void AllocInfo::Destroy()
{
    if(m_Image)
    {
        assert(!m_Buffer);
        vkDestroyImage(g_hDevice, m_Image, g_Allocs);
        m_Image = VK_NULL_HANDLE;
    }
    if(m_Buffer)
    {
        assert(!m_Image);
        vkDestroyBuffer(g_hDevice, m_Buffer, g_Allocs);
        m_Buffer = VK_NULL_HANDLE;
    }
    if(m_Allocation)
    {
        vmaFreeMemory(g_hAllocator, m_Allocation);
        m_Allocation = VK_NULL_HANDLE;
    }
}

class StagingBufferCollection
{
public:
    StagingBufferCollection() { }
    ~StagingBufferCollection();
    // Returns false if maximum total size of buffers would be exceeded.
    bool AcquireBuffer(VkDeviceSize size, VkBuffer& outBuffer, void*& outMappedPtr);
    void ReleaseAllBuffers();

private:
    static const VkDeviceSize MAX_TOTAL_SIZE = 256ull * 1024 * 1024;
    struct BufInfo
    {
        VmaAllocation Allocation = VK_NULL_HANDLE;
        VkBuffer Buffer = VK_NULL_HANDLE;
        VkDeviceSize Size = VK_WHOLE_SIZE;
        void* MappedPtr = nullptr;
        bool Used = false;
    };
    std::vector<BufInfo> m_Bufs;
    // Including both used and unused.
    VkDeviceSize m_TotalSize = 0;
};

StagingBufferCollection::~StagingBufferCollection()
{
    for(size_t i = m_Bufs.size(); i--; )
    {
        vmaDestroyBuffer(g_hAllocator, m_Bufs[i].Buffer, m_Bufs[i].Allocation);
    }
}

bool StagingBufferCollection::AcquireBuffer(VkDeviceSize size, VkBuffer& outBuffer, void*& outMappedPtr)
{
    assert(size <= MAX_TOTAL_SIZE);

    // Try to find existing unused buffer with best size.
    size_t bestIndex = SIZE_MAX;
    for(size_t i = 0, count = m_Bufs.size(); i < count; ++i)
    {
        BufInfo& currBufInfo = m_Bufs[i];
        if(!currBufInfo.Used && currBufInfo.Size >= size &&
            (bestIndex == SIZE_MAX || currBufInfo.Size < m_Bufs[bestIndex].Size))
        {
            bestIndex = i;
        }
    }

    if(bestIndex != SIZE_MAX)
    {
        m_Bufs[bestIndex].Used = true;
        outBuffer = m_Bufs[bestIndex].Buffer;
        outMappedPtr = m_Bufs[bestIndex].MappedPtr;
        return true;
    }
    
    // Allocate new buffer with requested size.
    if(m_TotalSize + size <= MAX_TOTAL_SIZE)
    {
        BufInfo bufInfo;
        bufInfo.Size = size;
        bufInfo.Used = true;

        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.size = size;
        bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo allocInfo;
        VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &bufInfo.Buffer, &bufInfo.Allocation, &allocInfo);
        bufInfo.MappedPtr = allocInfo.pMappedData;
        TEST(res == VK_SUCCESS && bufInfo.MappedPtr);

        outBuffer = bufInfo.Buffer;
        outMappedPtr = bufInfo.MappedPtr;

        m_Bufs.push_back(std::move(bufInfo));

        m_TotalSize += size;

        return true;
    }

    // There are some unused but smaller buffers: Free them and try again.
    bool hasUnused = false;
    for(size_t i = 0, count = m_Bufs.size(); i < count; ++i)
    {
        if(!m_Bufs[i].Used)
        {
            hasUnused = true;
            break;
        }
    }
    if(hasUnused)
    {
        for(size_t i = m_Bufs.size(); i--; )
        {
            if(!m_Bufs[i].Used)
            {
                m_TotalSize -= m_Bufs[i].Size;
                vmaDestroyBuffer(g_hAllocator, m_Bufs[i].Buffer, m_Bufs[i].Allocation);
                m_Bufs.erase(m_Bufs.begin() + i);
            }
        }

        return AcquireBuffer(size, outBuffer, outMappedPtr);
   }

    return false;
}

void StagingBufferCollection::ReleaseAllBuffers()
{
    for(size_t i = 0, count = m_Bufs.size(); i < count; ++i)
    {
        m_Bufs[i].Used = false;
    }
}

static void UploadGpuData(const AllocInfo* allocInfo, size_t allocInfoCount)
{
    StagingBufferCollection stagingBufs;

    bool cmdBufferStarted = false;
    for(size_t allocInfoIndex = 0; allocInfoIndex < allocInfoCount; ++allocInfoIndex)
    {
        const AllocInfo& currAllocInfo = allocInfo[allocInfoIndex];
        if(currAllocInfo.m_Buffer)
        {
            const VkDeviceSize size = currAllocInfo.m_BufferInfo.size;

            VkBuffer stagingBuf = VK_NULL_HANDLE;
            void* stagingBufMappedPtr = nullptr;
            if(!stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr))
            {
                TEST(cmdBufferStarted);
                EndSingleTimeCommands();
                stagingBufs.ReleaseAllBuffers();
                cmdBufferStarted = false;

                bool ok = stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr);
                TEST(ok);
            }

            // Fill staging buffer.
            {
                assert(size % sizeof(uint32_t) == 0);
                uint32_t* stagingValPtr = (uint32_t*)stagingBufMappedPtr;
                uint32_t val = currAllocInfo.m_StartValue;
                for(size_t i = 0; i < size / sizeof(uint32_t); ++i)
                {
                    *stagingValPtr = val;
                    ++stagingValPtr;
                    ++val;
                }
            }

            // Issue copy command from staging buffer to destination buffer.
            if(!cmdBufferStarted)
            {
                cmdBufferStarted = true;
                BeginSingleTimeCommands();
            }

            VkBufferCopy copy = {};
            copy.srcOffset = 0;
            copy.dstOffset = 0;
            copy.size = size;
            vkCmdCopyBuffer(g_hTemporaryCommandBuffer, stagingBuf, currAllocInfo.m_Buffer, 1, &copy);
        }
        else
        {
            TEST(currAllocInfo.m_ImageInfo.format == VK_FORMAT_R8G8B8A8_UNORM && "Only RGBA8 images are currently supported.");
            TEST(currAllocInfo.m_ImageInfo.mipLevels == 1 && "Only single mip images are currently supported.");

            const VkDeviceSize size = (VkDeviceSize)currAllocInfo.m_ImageInfo.extent.width * currAllocInfo.m_ImageInfo.extent.height * sizeof(uint32_t);

            VkBuffer stagingBuf = VK_NULL_HANDLE;
            void* stagingBufMappedPtr = nullptr;
            if(!stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr))
            {
                TEST(cmdBufferStarted);
                EndSingleTimeCommands();
                stagingBufs.ReleaseAllBuffers();
                cmdBufferStarted = false;

                bool ok = stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr);
                TEST(ok);
            }

            // Fill staging buffer.
            {
                assert(size % sizeof(uint32_t) == 0);
                uint32_t *stagingValPtr = (uint32_t *)stagingBufMappedPtr;
                uint32_t val = currAllocInfo.m_StartValue;
                for(size_t i = 0; i < size / sizeof(uint32_t); ++i)
                {
                    *stagingValPtr = val;
                    ++stagingValPtr;
                    ++val;
                }
            }
            
            // Issue copy command from staging buffer to destination buffer.
            if(!cmdBufferStarted)
            {
                cmdBufferStarted = true;
                BeginSingleTimeCommands();
            }

            
            // Transfer to transfer dst layout
            VkImageSubresourceRange subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0, VK_REMAINING_MIP_LEVELS,
                0, VK_REMAINING_ARRAY_LAYERS
            };
            
            VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = currAllocInfo.m_Image;
            barrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(g_hTemporaryCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            // Copy image date
            VkBufferImageCopy copy = {};
            copy.bufferOffset = 0;
            copy.bufferRowLength = 0;
            copy.bufferImageHeight = 0;
            copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy.imageSubresource.layerCount = 1;
            copy.imageExtent = currAllocInfo.m_ImageInfo.extent;

            vkCmdCopyBufferToImage(g_hTemporaryCommandBuffer, stagingBuf, currAllocInfo.m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

            // Transfer to desired layout
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = currAllocInfo.m_ImageLayout;

            vkCmdPipelineBarrier(g_hTemporaryCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }
    }

    if(cmdBufferStarted)
    {
        EndSingleTimeCommands();
        stagingBufs.ReleaseAllBuffers();
    }
}

static void ValidateGpuData(const AllocInfo* allocInfo, size_t allocInfoCount)
{
    StagingBufferCollection stagingBufs;

    bool cmdBufferStarted = false;
    size_t validateAllocIndexOffset = 0;
    std::vector<void*> validateStagingBuffers;
    for(size_t allocInfoIndex = 0; allocInfoIndex < allocInfoCount; ++allocInfoIndex)
    {
        const AllocInfo& currAllocInfo = allocInfo[allocInfoIndex];
        if(currAllocInfo.m_Buffer)
        {
            const VkDeviceSize size = currAllocInfo.m_BufferInfo.size;

            VkBuffer stagingBuf = VK_NULL_HANDLE;
            void* stagingBufMappedPtr = nullptr;
            if(!stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr))
            {
                TEST(cmdBufferStarted);
                EndSingleTimeCommands();
                cmdBufferStarted = false;

                for(size_t validateIndex = 0;
                    validateIndex < validateStagingBuffers.size();
                    ++validateIndex)
                {
                    const size_t validateAllocIndex = validateIndex + validateAllocIndexOffset;
                    const VkDeviceSize validateSize = allocInfo[validateAllocIndex].m_BufferInfo.size;
                    TEST(validateSize % sizeof(uint32_t) == 0);
                    const uint32_t* stagingValPtr = (const uint32_t*)validateStagingBuffers[validateIndex];
                    uint32_t val = allocInfo[validateAllocIndex].m_StartValue;
                    bool valid = true;
                    for(size_t i = 0; i < validateSize / sizeof(uint32_t); ++i)
                    {
                        if(*stagingValPtr != val)
                        {
                            valid = false;
                            break;
                        }
                        ++stagingValPtr;
                        ++val;
                    }
                    TEST(valid);
                }

                stagingBufs.ReleaseAllBuffers();

                validateAllocIndexOffset = allocInfoIndex;
                validateStagingBuffers.clear();

                bool ok = stagingBufs.AcquireBuffer(size, stagingBuf, stagingBufMappedPtr);
                TEST(ok);
            }

            // Issue copy command from staging buffer to destination buffer.
            if(!cmdBufferStarted)
            {
                cmdBufferStarted = true;
                BeginSingleTimeCommands();
            }

            VkBufferCopy copy = {};
            copy.srcOffset = 0;
            copy.dstOffset = 0;
            copy.size = size;
            vkCmdCopyBuffer(g_hTemporaryCommandBuffer, currAllocInfo.m_Buffer, stagingBuf, 1, &copy);

            // Sava mapped pointer for later validation.
            validateStagingBuffers.push_back(stagingBufMappedPtr);
        }
        else
        {
            TEST(0 && "Images not currently supported.");
        }
    }

    if(cmdBufferStarted)
    {
        EndSingleTimeCommands();

        for(size_t validateIndex = 0;
            validateIndex < validateStagingBuffers.size();
            ++validateIndex)
        {
            const size_t validateAllocIndex = validateIndex + validateAllocIndexOffset;
            const VkDeviceSize validateSize = allocInfo[validateAllocIndex].m_BufferInfo.size;
            TEST(validateSize % sizeof(uint32_t) == 0);
            const uint32_t* stagingValPtr = (const uint32_t*)validateStagingBuffers[validateIndex];
            uint32_t val = allocInfo[validateAllocIndex].m_StartValue;
            bool valid = true;
            for(size_t i = 0; i < validateSize / sizeof(uint32_t); ++i)
            {
                if(*stagingValPtr != val)
                {
                    valid = false;
                    break;
                }
                ++stagingValPtr;
                ++val;
            }
            TEST(valid);
        }

        stagingBufs.ReleaseAllBuffers();
    }
}

static void GetMemReq(VmaAllocationCreateInfo& outMemReq)
{
    outMemReq = {};
    outMemReq.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    //outMemReq.flags = VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT;
}

static void CreateBuffer(
    VmaPool pool,
    const VkBufferCreateInfo& bufCreateInfo,
    bool persistentlyMapped,
    AllocInfo& outAllocInfo)
{
    outAllocInfo = {};
    outAllocInfo.m_BufferInfo = bufCreateInfo;
    
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;
    if(persistentlyMapped)
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo vmaAllocInfo = {};
    ERR_GUARD_VULKAN( vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &outAllocInfo.m_Buffer, &outAllocInfo.m_Allocation, &vmaAllocInfo) );

    // Setup StartValue and fill.
    {
        outAllocInfo.m_StartValue = (uint32_t)rand();
        uint32_t* data = (uint32_t*)vmaAllocInfo.pMappedData;
        TEST((data != nullptr) == persistentlyMapped);
        if(!persistentlyMapped)
        {
            ERR_GUARD_VULKAN( vmaMapMemory(g_hAllocator, outAllocInfo.m_Allocation, (void**)&data) );
        }

        uint32_t value = outAllocInfo.m_StartValue;
        TEST(bufCreateInfo.size % 4 == 0);
        for(size_t i = 0; i < bufCreateInfo.size / sizeof(uint32_t); ++i)
            data[i] = value++;

        if(!persistentlyMapped)
            vmaUnmapMemory(g_hAllocator, outAllocInfo.m_Allocation);
    }
}

static void CreateAllocation(AllocInfo& outAllocation)
{
    outAllocation.m_Allocation = nullptr;
    outAllocation.m_Buffer = nullptr;
    outAllocation.m_Image = nullptr;
    outAllocation.m_StartValue = (uint32_t)rand();

    VmaAllocationCreateInfo vmaMemReq;
    GetMemReq(vmaMemReq);

    VmaAllocationInfo allocInfo;

    const bool isBuffer = true;//(rand() & 0x1) != 0;
    const bool isLarge = (rand() % 16) == 0;
    if(isBuffer)
    {
        const uint32_t bufferSize = isLarge ?
            (rand() % 10 + 1) * (1024 * 1024) : // 1 MB ... 10 MB
            (rand() % 1024 + 1) * 1024; // 1 KB ... 1 MB

        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VkResult res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &vmaMemReq, &outAllocation.m_Buffer, &outAllocation.m_Allocation, &allocInfo);
        outAllocation.m_BufferInfo = bufferInfo;
        TEST(res == VK_SUCCESS);
    }
    else
    {
        const uint32_t imageSizeX = isLarge ?
            1024 + rand() % (4096 - 1024) : // 1024 ... 4096
            rand() % 1024 + 1; // 1 ... 1024
        const uint32_t imageSizeY = isLarge ?
            1024 + rand() % (4096 - 1024) : // 1024 ... 4096
            rand() % 1024 + 1; // 1 ... 1024

        VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.extent.width = imageSizeX;
        imageInfo.extent.height = imageSizeY;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        VkResult res = vmaCreateImage(g_hAllocator, &imageInfo, &vmaMemReq, &outAllocation.m_Image, &outAllocation.m_Allocation, &allocInfo);
        outAllocation.m_ImageInfo = imageInfo;
        TEST(res == VK_SUCCESS);
    }

    uint32_t* data = (uint32_t*)allocInfo.pMappedData;
    if(allocInfo.pMappedData == nullptr)
    {
        VkResult res = vmaMapMemory(g_hAllocator, outAllocation.m_Allocation, (void**)&data);
        TEST(res == VK_SUCCESS);
    }

    uint32_t value = outAllocation.m_StartValue;
    TEST(allocInfo.size % 4 == 0);
    for(size_t i = 0; i < allocInfo.size / sizeof(uint32_t); ++i)
        data[i] = value++;

    if(allocInfo.pMappedData == nullptr)
        vmaUnmapMemory(g_hAllocator, outAllocation.m_Allocation);
}

static void DestroyAllocation(const AllocInfo& allocation)
{
    if(allocation.m_Buffer)
        vmaDestroyBuffer(g_hAllocator, allocation.m_Buffer, allocation.m_Allocation);
    else
        vmaDestroyImage(g_hAllocator, allocation.m_Image, allocation.m_Allocation);
}

static void DestroyAllAllocations(std::vector<AllocInfo>& allocations)
{
    for(size_t i = allocations.size(); i--; )
        DestroyAllocation(allocations[i]);
    allocations.clear();
}

static void ValidateAllocationData(const AllocInfo& allocation)
{
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(g_hAllocator, allocation.m_Allocation, &allocInfo);

    uint32_t* data = (uint32_t*)allocInfo.pMappedData;
    if(allocInfo.pMappedData == nullptr)
    {
        VkResult res = vmaMapMemory(g_hAllocator, allocation.m_Allocation, (void**)&data);
        TEST(res == VK_SUCCESS);
    }

    uint32_t value = allocation.m_StartValue;
    bool ok = true;
    size_t i;
    TEST(allocInfo.size % 4 == 0);
    for(i = 0; i < allocInfo.size / sizeof(uint32_t); ++i)
    {
        if(data[i] != value++)
        {
            ok = false;
            break;
        }
    }
    TEST(ok);

    if(allocInfo.pMappedData == nullptr)
        vmaUnmapMemory(g_hAllocator, allocation.m_Allocation);
}

static void RecreateAllocationResource(AllocInfo& allocation)
{
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(g_hAllocator, allocation.m_Allocation, &allocInfo);

    if(allocation.m_Buffer)
    {
        vkDestroyBuffer(g_hDevice, allocation.m_Buffer, g_Allocs);

        VkResult res = vkCreateBuffer(g_hDevice, &allocation.m_BufferInfo, g_Allocs, &allocation.m_Buffer);
        TEST(res == VK_SUCCESS);

        // Just to silence validation layer warnings.
        VkMemoryRequirements vkMemReq;
        vkGetBufferMemoryRequirements(g_hDevice, allocation.m_Buffer, &vkMemReq);
        TEST(vkMemReq.size >= allocation.m_BufferInfo.size);

        res = vmaBindBufferMemory(g_hAllocator, allocation.m_Allocation, allocation.m_Buffer);
        TEST(res == VK_SUCCESS);
    }
    else
    {
        vkDestroyImage(g_hDevice, allocation.m_Image, g_Allocs);

        VkResult res = vkCreateImage(g_hDevice, &allocation.m_ImageInfo, g_Allocs, &allocation.m_Image);
        TEST(res == VK_SUCCESS);

        // Just to silence validation layer warnings.
        VkMemoryRequirements vkMemReq;
        vkGetImageMemoryRequirements(g_hDevice, allocation.m_Image, &vkMemReq);

        res = vmaBindImageMemory(g_hAllocator, allocation.m_Allocation, allocation.m_Image);
        TEST(res == VK_SUCCESS);
    }
}

static void Defragment(AllocInfo* allocs, size_t allocCount,
    const VmaDefragmentationInfo* defragmentationInfo = nullptr,
    VmaDefragmentationStats* defragmentationStats = nullptr)
{
    std::vector<VmaAllocation> vmaAllocs(allocCount);
    for(size_t i = 0; i < allocCount; ++i)
        vmaAllocs[i] = allocs[i].m_Allocation;

    std::vector<VkBool32> allocChanged(allocCount);
    
    ERR_GUARD_VULKAN( vmaDefragment(g_hAllocator, vmaAllocs.data(), allocCount, allocChanged.data(),
        defragmentationInfo, defragmentationStats) );

    for(size_t i = 0; i < allocCount; ++i)
    {
        if(allocChanged[i])
        {
            RecreateAllocationResource(allocs[i]);
        }
    }
}

static void ValidateAllocationsData(const AllocInfo* allocs, size_t allocCount)
{
    std::for_each(allocs, allocs + allocCount, [](const AllocInfo& allocInfo) {
        ValidateAllocationData(allocInfo);
    });
}

void TestDefragmentationSimple()
{
    wprintf(L"Test defragmentation simple\n");

    RandomNumberGenerator rand(667);

    const VkDeviceSize BUF_SIZE = 0x10000;
    const VkDeviceSize BLOCK_SIZE = BUF_SIZE * 8;
    
    const VkDeviceSize MIN_BUF_SIZE = 32;
    const VkDeviceSize MAX_BUF_SIZE = BUF_SIZE * 4;
    auto RandomBufSize = [&]() -> VkDeviceSize {
        return align_up<VkDeviceSize>(rand.Generate() % (MAX_BUF_SIZE - MIN_BUF_SIZE + 1) + MIN_BUF_SIZE, 32);
    };

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = BUF_SIZE;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo exampleAllocCreateInfo = {};
    exampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    uint32_t memTypeIndex = UINT32_MAX;
    vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufCreateInfo, &exampleAllocCreateInfo, &memTypeIndex);

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.blockSize = BLOCK_SIZE;
    poolCreateInfo.memoryTypeIndex = memTypeIndex;

    VmaPool pool;
    ERR_GUARD_VULKAN( vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool) );

    // Defragmentation of empty pool.
    {
        VmaDefragmentationInfo2 defragInfo = {};
        defragInfo.maxCpuBytesToMove = VK_WHOLE_SIZE;
        defragInfo.maxCpuAllocationsToMove = UINT32_MAX;
        defragInfo.poolCount = 1;
        defragInfo.pPools = &pool;

        VmaDefragmentationStats defragStats = {};
        VmaDefragmentationContext defragCtx = nullptr;
        VkResult res = vmaDefragmentationBegin(g_hAllocator, &defragInfo, &defragStats, &defragCtx);
        TEST(res >= VK_SUCCESS);
        vmaDefragmentationEnd(g_hAllocator, defragCtx);
        TEST(defragStats.allocationsMoved == 0 && defragStats.bytesFreed == 0 &&
            defragStats.bytesMoved == 0 && defragStats.deviceMemoryBlocksFreed == 0);
    }

    std::vector<AllocInfo> allocations;

    // persistentlyMappedOption = 0 - not persistently mapped.
    // persistentlyMappedOption = 1 - persistently mapped.
    for(uint32_t persistentlyMappedOption = 0; persistentlyMappedOption < 2; ++persistentlyMappedOption)
    {
        wprintf(L"  Persistently mapped option = %u\n", persistentlyMappedOption);
        const bool persistentlyMapped = persistentlyMappedOption != 0;

        // # Test 1
        // Buffers of fixed size.
        // Fill 2 blocks. Remove odd buffers. Defragment everything.
        // Expected result: at least 1 block freed.
        {
            for(size_t i = 0; i < BLOCK_SIZE / BUF_SIZE * 2; ++i)
            {
                AllocInfo allocInfo;
                CreateBuffer(pool, bufCreateInfo, persistentlyMapped, allocInfo);
                allocations.push_back(allocInfo);
            }

            for(size_t i = 1; i < allocations.size(); ++i)
            {
                DestroyAllocation(allocations[i]);
                allocations.erase(allocations.begin() + i);
            }

            VmaDefragmentationStats defragStats;
            Defragment(allocations.data(), allocations.size(), nullptr, &defragStats);
            TEST(defragStats.allocationsMoved > 0 && defragStats.bytesMoved > 0);
            TEST(defragStats.deviceMemoryBlocksFreed >= 1);

            ValidateAllocationsData(allocations.data(), allocations.size());

            DestroyAllAllocations(allocations);
        }

        // # Test 2
        // Buffers of fixed size.
        // Fill 2 blocks. Remove odd buffers. Defragment one buffer at time.
        // Expected result: Each of 4 interations makes some progress.
        {
            for(size_t i = 0; i < BLOCK_SIZE / BUF_SIZE * 2; ++i)
            {
                AllocInfo allocInfo;
                CreateBuffer(pool, bufCreateInfo, persistentlyMapped, allocInfo);
                allocations.push_back(allocInfo);
            }

            for(size_t i = 1; i < allocations.size(); ++i)
            {
                DestroyAllocation(allocations[i]);
                allocations.erase(allocations.begin() + i);
            }

            VmaDefragmentationInfo defragInfo = {};
            defragInfo.maxAllocationsToMove = 1;
            defragInfo.maxBytesToMove = BUF_SIZE;

            for(size_t i = 0; i < BLOCK_SIZE / BUF_SIZE / 2; ++i)
            {
                VmaDefragmentationStats defragStats;
                Defragment(allocations.data(), allocations.size(), &defragInfo, &defragStats);
                TEST(defragStats.allocationsMoved > 0 && defragStats.bytesMoved > 0);
            }

            ValidateAllocationsData(allocations.data(), allocations.size());

            DestroyAllAllocations(allocations);
        }

        // # Test 3
        // Buffers of variable size.
        // Create a number of buffers. Remove some percent of them.
        // Defragment while having some percent of them unmovable.
        // Expected result: Just simple validation.
        {
            for(size_t i = 0; i < 100; ++i)
            {
                VkBufferCreateInfo localBufCreateInfo = bufCreateInfo;
                localBufCreateInfo.size = RandomBufSize();

                AllocInfo allocInfo;
                CreateBuffer(pool, bufCreateInfo, persistentlyMapped, allocInfo);
                allocations.push_back(allocInfo);
            }

            const uint32_t percentToDelete = 60;
            const size_t numberToDelete = allocations.size() * percentToDelete / 100;
            for(size_t i = 0; i < numberToDelete; ++i)
            {
                size_t indexToDelete = rand.Generate() % (uint32_t)allocations.size();
                DestroyAllocation(allocations[indexToDelete]);
                allocations.erase(allocations.begin() + indexToDelete);
            }

            // Non-movable allocations will be at the beginning of allocations array.
            const uint32_t percentNonMovable = 20;
            const size_t numberNonMovable = allocations.size() * percentNonMovable / 100;
            for(size_t i = 0; i < numberNonMovable; ++i)
            {
                size_t indexNonMovable = i + rand.Generate() % (uint32_t)(allocations.size() - i);
                if(indexNonMovable != i)
                    std::swap(allocations[i], allocations[indexNonMovable]);
            }

            VmaDefragmentationStats defragStats;
            Defragment(
                allocations.data() + numberNonMovable,
                allocations.size() - numberNonMovable,
                nullptr, &defragStats);

            ValidateAllocationsData(allocations.data(), allocations.size());

            DestroyAllAllocations(allocations);
        }
    }

    /*
    Allocation that must be move to an overlapping place using memmove().
    Create 2 buffers, second slightly bigger than the first. Delete first. Then defragment.
    */
    if(VMA_DEBUG_MARGIN == 0) // FAST algorithm works only when DEBUG_MARGIN disabled.
    {
        AllocInfo allocInfo[2];

        bufCreateInfo.size = BUF_SIZE;
        CreateBuffer(pool, bufCreateInfo, false, allocInfo[0]);
        const VkDeviceSize biggerBufSize = BUF_SIZE + BUF_SIZE / 256;
        bufCreateInfo.size = biggerBufSize;
        CreateBuffer(pool, bufCreateInfo, false, allocInfo[1]);

        DestroyAllocation(allocInfo[0]);

        VmaDefragmentationStats defragStats;
        Defragment(&allocInfo[1], 1, nullptr, &defragStats);
        // If this fails, it means we couldn't do memmove with overlapping regions.
        TEST(defragStats.allocationsMoved == 1 && defragStats.bytesMoved > 0);

        ValidateAllocationsData(&allocInfo[1], 1);
        DestroyAllocation(allocInfo[1]);
    }

    vmaDestroyPool(g_hAllocator, pool);
}

void TestDefragmentationWholePool()
{
    wprintf(L"Test defragmentation whole pool\n");

    RandomNumberGenerator rand(668);

    const VkDeviceSize BUF_SIZE = 0x10000;
    const VkDeviceSize BLOCK_SIZE = BUF_SIZE * 8;
    
    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = BUF_SIZE;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo exampleAllocCreateInfo = {};
    exampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    uint32_t memTypeIndex = UINT32_MAX;
    vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufCreateInfo, &exampleAllocCreateInfo, &memTypeIndex);

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.blockSize = BLOCK_SIZE;
    poolCreateInfo.memoryTypeIndex = memTypeIndex;

    VmaDefragmentationStats defragStats[2];
    for(size_t caseIndex = 0; caseIndex < 2; ++caseIndex)
    {
        VmaPool pool;
        ERR_GUARD_VULKAN( vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool) );

        std::vector<AllocInfo> allocations;

        // Buffers of fixed size.
        // Fill 2 blocks. Remove odd buffers. Defragment all of them.
        for(size_t i = 0; i < BLOCK_SIZE / BUF_SIZE * 2; ++i)
        {
            AllocInfo allocInfo;
            CreateBuffer(pool, bufCreateInfo, false, allocInfo);
            allocations.push_back(allocInfo);
        }

        for(size_t i = 1; i < allocations.size(); ++i)
        {
            DestroyAllocation(allocations[i]);
            allocations.erase(allocations.begin() + i);
        }

        VmaDefragmentationInfo2 defragInfo = {};
        defragInfo.maxCpuAllocationsToMove = UINT32_MAX;
        defragInfo.maxCpuBytesToMove = VK_WHOLE_SIZE;
        std::vector<VmaAllocation> allocationsToDefrag;
        if(caseIndex == 0)
        {
            defragInfo.poolCount = 1;
            defragInfo.pPools = &pool;
        }
        else
        {
            const size_t allocCount = allocations.size();
            allocationsToDefrag.resize(allocCount);
            std::transform(
                allocations.begin(), allocations.end(),
                allocationsToDefrag.begin(),
                [](const AllocInfo& allocInfo) { return allocInfo.m_Allocation; });
            defragInfo.allocationCount = (uint32_t)allocCount;
            defragInfo.pAllocations = allocationsToDefrag.data();
        }

        VmaDefragmentationContext defragCtx = VK_NULL_HANDLE;
        VkResult res = vmaDefragmentationBegin(g_hAllocator, &defragInfo, &defragStats[caseIndex], &defragCtx);
        TEST(res >= VK_SUCCESS);
        vmaDefragmentationEnd(g_hAllocator, defragCtx);

        TEST(defragStats[caseIndex].allocationsMoved > 0 && defragStats[caseIndex].bytesMoved > 0);

        ValidateAllocationsData(allocations.data(), allocations.size());

        DestroyAllAllocations(allocations);

        vmaDestroyPool(g_hAllocator, pool);
    }

    TEST(defragStats[0].bytesMoved == defragStats[1].bytesMoved);
    TEST(defragStats[0].allocationsMoved == defragStats[1].allocationsMoved);
    TEST(defragStats[0].bytesFreed == defragStats[1].bytesFreed);
    TEST(defragStats[0].deviceMemoryBlocksFreed == defragStats[1].deviceMemoryBlocksFreed);
}

void TestDefragmentationFull()
{
    std::vector<AllocInfo> allocations;

    // Create initial allocations.
    for(size_t i = 0; i < 400; ++i)
    {
        AllocInfo allocation;
        CreateAllocation(allocation);
        allocations.push_back(allocation);
    }

    // Delete random allocations
    const size_t allocationsToDeletePercent = 80;
    size_t allocationsToDelete = allocations.size() * allocationsToDeletePercent / 100;
    for(size_t i = 0; i < allocationsToDelete; ++i)
    {
        size_t index = (size_t)rand() % allocations.size();
        DestroyAllocation(allocations[index]);
        allocations.erase(allocations.begin() + index);
    }

    for(size_t i = 0; i < allocations.size(); ++i)
        ValidateAllocationData(allocations[i]);

    //SaveAllocatorStatsToFile(L"Before.csv");

    {
        std::vector<VmaAllocation> vmaAllocations(allocations.size());
        for(size_t i = 0; i < allocations.size(); ++i)
            vmaAllocations[i] = allocations[i].m_Allocation;

        const size_t nonMovablePercent = 0;
        size_t nonMovableCount = vmaAllocations.size() * nonMovablePercent / 100;
        for(size_t i = 0; i < nonMovableCount; ++i)
        {
            size_t index = (size_t)rand() % vmaAllocations.size();
            vmaAllocations.erase(vmaAllocations.begin() + index);
        }

        const uint32_t defragCount = 1;
        for(uint32_t defragIndex = 0; defragIndex < defragCount; ++defragIndex)
        {
            std::vector<VkBool32> allocationsChanged(vmaAllocations.size());

            VmaDefragmentationInfo defragmentationInfo;
            defragmentationInfo.maxAllocationsToMove = UINT_MAX;
            defragmentationInfo.maxBytesToMove = SIZE_MAX;

            wprintf(L"Defragmentation #%u\n", defragIndex);

            time_point begTime = std::chrono::high_resolution_clock::now();

            VmaDefragmentationStats stats;
            VkResult res = vmaDefragment(g_hAllocator, vmaAllocations.data(), vmaAllocations.size(), allocationsChanged.data(), &defragmentationInfo, &stats);
            TEST(res >= 0);

            float defragmentDuration = ToFloatSeconds(std::chrono::high_resolution_clock::now() - begTime);

            wprintf(L"Moved allocations %u, bytes %llu\n", stats.allocationsMoved, stats.bytesMoved);
            wprintf(L"Freed blocks %u, bytes %llu\n", stats.deviceMemoryBlocksFreed, stats.bytesFreed);
            wprintf(L"Time: %.2f s\n", defragmentDuration);

            for(size_t i = 0; i < vmaAllocations.size(); ++i)
            {
                if(allocationsChanged[i])
                {
                    RecreateAllocationResource(allocations[i]);
                }
            }

            for(size_t i = 0; i < allocations.size(); ++i)
                ValidateAllocationData(allocations[i]);

            //wchar_t fileName[MAX_PATH];
            //swprintf(fileName, MAX_PATH, L"After_%02u.csv", defragIndex);
            //SaveAllocatorStatsToFile(fileName);
        }
    }

    // Destroy all remaining allocations.
    DestroyAllAllocations(allocations);
}

static void TestDefragmentationGpu()
{
    wprintf(L"Test defragmentation GPU\n");

    std::vector<AllocInfo> allocations;

    // Create that many allocations to surely fill 3 new blocks of 256 MB.
    const VkDeviceSize bufSizeMin = 5ull * 1024 * 1024;
    const VkDeviceSize bufSizeMax = 10ull * 1024 * 1024;
    const VkDeviceSize totalSize = 3ull * 256 * 1024 * 1024;
    const size_t bufCount = (size_t)(totalSize / bufSizeMin);
    const size_t percentToLeave = 30;
    const size_t percentNonMovable = 3;
    RandomNumberGenerator rand = { 234522 };

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.flags = 0;

    // Create all intended buffers.
    for(size_t i = 0; i < bufCount; ++i)
    {
        bufCreateInfo.size = align_up(rand.Generate() % (bufSizeMax - bufSizeMin) + bufSizeMin, 32ull);

        if(rand.Generate() % 100 < percentNonMovable)
        {
            bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocCreateInfo.pUserData = (void*)(uintptr_t)2;
        }
        else
        {
            // Different usage just to see different color in output from VmaDumpVis.
            bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            // And in JSON dump.
            allocCreateInfo.pUserData = (void*)(uintptr_t)1;
        }

        AllocInfo alloc;
        alloc.CreateBuffer(bufCreateInfo, allocCreateInfo);
        alloc.m_StartValue = rand.Generate();
        allocations.push_back(alloc);
    }

    // Destroy some percentage of them.
    {
        const size_t buffersToDestroy = round_div<size_t>(bufCount * (100 - percentToLeave), 100);
        for(size_t i = 0; i < buffersToDestroy; ++i)
        {
            const size_t index = rand.Generate() % allocations.size();
            allocations[index].Destroy();
            allocations.erase(allocations.begin() + index);
        }
    }

    // Fill them with meaningful data.
    UploadGpuData(allocations.data(), allocations.size());

    wchar_t fileName[MAX_PATH];
    swprintf_s(fileName, L"GPU_defragmentation_A_before.json");
    SaveAllocatorStatsToFile(fileName);

    // Defragment using GPU only.
    {
        const size_t allocCount = allocations.size();

        std::vector<VmaAllocation> allocationPtrs;
        std::vector<VkBool32> allocationChanged;
        std::vector<size_t> allocationOriginalIndex;

        for(size_t i = 0; i < allocCount; ++i)
        {
            VmaAllocationInfo allocInfo = {};
            vmaGetAllocationInfo(g_hAllocator, allocations[i].m_Allocation, &allocInfo);
            if((uintptr_t)allocInfo.pUserData == 1) // Movable
            {
                allocationPtrs.push_back(allocations[i].m_Allocation);
                allocationChanged.push_back(VK_FALSE);
                allocationOriginalIndex.push_back(i);
            }
        }

        const size_t movableAllocCount = allocationPtrs.size();

        BeginSingleTimeCommands();

        VmaDefragmentationInfo2 defragInfo = {};
        defragInfo.flags = 0;
        defragInfo.allocationCount = (uint32_t)movableAllocCount;
        defragInfo.pAllocations = allocationPtrs.data();
        defragInfo.pAllocationsChanged = allocationChanged.data();
        defragInfo.maxGpuBytesToMove = VK_WHOLE_SIZE;
        defragInfo.maxGpuAllocationsToMove = UINT32_MAX;
        defragInfo.commandBuffer = g_hTemporaryCommandBuffer;

        VmaDefragmentationStats stats = {};
        VmaDefragmentationContext ctx = VK_NULL_HANDLE;
        VkResult res = vmaDefragmentationBegin(g_hAllocator, &defragInfo, &stats, &ctx);
        TEST(res >= VK_SUCCESS);

        EndSingleTimeCommands();

        vmaDefragmentationEnd(g_hAllocator, ctx);

        for(size_t i = 0; i < movableAllocCount; ++i)
        {
            if(allocationChanged[i])
            {
                const size_t origAllocIndex = allocationOriginalIndex[i];
                RecreateAllocationResource(allocations[origAllocIndex]);
            }
        }

        // If corruption detection is enabled, GPU defragmentation may not work on
        // memory types that have this detection active, e.g. on Intel.
        #if !defined(VMA_DEBUG_DETECT_CORRUPTION) || VMA_DEBUG_DETECT_CORRUPTION == 0
            TEST(stats.allocationsMoved > 0 && stats.bytesMoved > 0);
            TEST(stats.deviceMemoryBlocksFreed > 0 && stats.bytesFreed > 0);
        #endif
    }

    ValidateGpuData(allocations.data(), allocations.size());

    swprintf_s(fileName, L"GPU_defragmentation_B_after.json");
    SaveAllocatorStatsToFile(fileName);

    // Destroy all remaining buffers.
    for(size_t i = allocations.size(); i--; )
    {
        allocations[i].Destroy();
    }
}

static void ProcessDefragmentationStepInfo(VmaDefragmentationPassInfo &stepInfo)
{
    std::vector<VkImageMemoryBarrier> beginImageBarriers;
    std::vector<VkImageMemoryBarrier> finalizeImageBarriers;

    VkPipelineStageFlags beginSrcStageMask = 0;
    VkPipelineStageFlags beginDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkPipelineStageFlags finalizeSrcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkPipelineStageFlags finalizeDstStageMask = 0;

    bool wantsMemoryBarrier = false;

    VkMemoryBarrier beginMemoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
    VkMemoryBarrier finalizeMemoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };

    for(uint32_t i = 0; i < stepInfo.moveCount; ++i)
    {
        VmaAllocationInfo info;
        vmaGetAllocationInfo(g_hAllocator, stepInfo.pMoves[i].allocation, &info);

        AllocInfo *allocInfo = (AllocInfo *)info.pUserData;

        if(allocInfo->m_Image)
        {
            VkImage newImage;

            const VkResult result = vkCreateImage(g_hDevice, &allocInfo->m_ImageInfo, g_Allocs, &newImage);
            TEST(result >= VK_SUCCESS);

            vkBindImageMemory(g_hDevice, newImage, stepInfo.pMoves[i].memory, stepInfo.pMoves[i].offset);
            allocInfo->m_NewImage = newImage;

            // Keep track of our pipeline stages that we need to wait/signal on
            beginSrcStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            finalizeDstStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            // We need one pipeline barrier and two image layout transitions here
            // First we'll have to turn our newly created image into VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            // And the second one is turning the old image into VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL

            VkImageSubresourceRange subresourceRange = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0, VK_REMAINING_MIP_LEVELS,
                0, VK_REMAINING_ARRAY_LAYERS
            };

            VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = newImage;
            barrier.subresourceRange = subresourceRange;

            beginImageBarriers.push_back(barrier);

            // Second barrier to convert the existing image. This one actually needs a real barrier                         
            barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout = allocInfo->m_ImageLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.image = allocInfo->m_Image;

            beginImageBarriers.push_back(barrier);

            // And lastly we need a barrier that turns our new image into the layout of the old one
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = allocInfo->m_ImageLayout;
            barrier.image = newImage;

            finalizeImageBarriers.push_back(barrier);
        }
        else if(allocInfo->m_Buffer)
        {
            VkBuffer newBuffer;

            const VkResult result = vkCreateBuffer(g_hDevice, &allocInfo->m_BufferInfo, g_Allocs, &newBuffer);
            TEST(result >= VK_SUCCESS);

            vkBindBufferMemory(g_hDevice, newBuffer, stepInfo.pMoves[i].memory, stepInfo.pMoves[i].offset);
            allocInfo->m_NewBuffer = newBuffer;

            // Keep track of our pipeline stages that we need to wait/signal on
            beginSrcStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            finalizeDstStageMask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            beginMemoryBarrier.srcAccessMask |= VK_ACCESS_MEMORY_WRITE_BIT;
            beginMemoryBarrier.dstAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;

            finalizeMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
            finalizeMemoryBarrier.dstAccessMask |= VK_ACCESS_MEMORY_READ_BIT;

            wantsMemoryBarrier = true;
        }
    }

    if(!beginImageBarriers.empty() || wantsMemoryBarrier)
    {
        const uint32_t memoryBarrierCount = wantsMemoryBarrier ? 1 : 0;

        vkCmdPipelineBarrier(g_hTemporaryCommandBuffer, beginSrcStageMask, beginDstStageMask, 0,
            memoryBarrierCount, &beginMemoryBarrier,
            0, nullptr,
            (uint32_t)beginImageBarriers.size(), beginImageBarriers.data());
    }

    for(uint32_t i = 0; i < stepInfo.moveCount; ++ i)
    {
        VmaAllocationInfo info;
        vmaGetAllocationInfo(g_hAllocator, stepInfo.pMoves[i].allocation, &info);

        AllocInfo *allocInfo = (AllocInfo *)info.pUserData;

        if(allocInfo->m_Image)
        {
            std::vector<VkImageCopy> imageCopies;

            // Copy all mips of the source image into the target image
            VkOffset3D offset = { 0, 0, 0 };
            VkExtent3D extent = allocInfo->m_ImageInfo.extent;

            VkImageSubresourceLayers subresourceLayers = {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                0, 1
            };

            for(uint32_t mip = 0; mip < allocInfo->m_ImageInfo.mipLevels; ++ mip)
            {
                subresourceLayers.mipLevel = mip;

                VkImageCopy imageCopy{
                    subresourceLayers,
                    offset,
                    subresourceLayers,
                    offset,
                    extent
                };

                imageCopies.push_back(imageCopy);

                extent.width = std::max(uint32_t(1), extent.width >> 1);
                extent.height = std::max(uint32_t(1), extent.height >> 1);
                extent.depth = std::max(uint32_t(1), extent.depth >> 1);
            }

            vkCmdCopyImage(
                g_hTemporaryCommandBuffer,
                allocInfo->m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                allocInfo->m_NewImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                (uint32_t)imageCopies.size(), imageCopies.data());
        }
        else if(allocInfo->m_Buffer)
        {
            VkBufferCopy region = {
                0,
                0,
                allocInfo->m_BufferInfo.size };

            vkCmdCopyBuffer(g_hTemporaryCommandBuffer, 
                allocInfo->m_Buffer, allocInfo->m_NewBuffer,
                1, &region);
        }
    }

    if(!finalizeImageBarriers.empty() || wantsMemoryBarrier)
    {
        const uint32_t memoryBarrierCount = wantsMemoryBarrier ? 1 : 0;

        vkCmdPipelineBarrier(g_hTemporaryCommandBuffer, finalizeSrcStageMask, finalizeDstStageMask, 0,
            memoryBarrierCount, &finalizeMemoryBarrier,
            0, nullptr,
            (uint32_t)finalizeImageBarriers.size(), finalizeImageBarriers.data());
    }
}


static void TestDefragmentationIncrementalBasic()
{
    wprintf(L"Test defragmentation incremental basic\n");

    std::vector<AllocInfo> allocations;

    // Create that many allocations to surely fill 3 new blocks of 256 MB.
    const std::array<uint32_t, 3> imageSizes = { 256, 512, 1024 };
    const VkDeviceSize bufSizeMin = 5ull * 1024 * 1024;
    const VkDeviceSize bufSizeMax = 10ull * 1024 * 1024;
    const VkDeviceSize totalSize = 3ull * 256 * 1024 * 1024;
    const size_t imageCount = totalSize / ((size_t)imageSizes[0] * imageSizes[0] * 4) / 2;
    const size_t bufCount = (size_t)(totalSize / bufSizeMin) / 2;
    const size_t percentToLeave = 30;
    RandomNumberGenerator rand = { 234522 };

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.flags = 0;

    // Create all intended images.
    for(size_t i = 0; i < imageCount; ++i)
    {
        const uint32_t size = imageSizes[rand.Generate() % 3];

        imageInfo.extent.width = size;
        imageInfo.extent.height = size;

        AllocInfo alloc;
        alloc.CreateImage(imageInfo, allocCreateInfo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        alloc.m_StartValue = 0;

        allocations.push_back(alloc);
    }

    // And all buffers
    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

    for(size_t i = 0; i < bufCount; ++i)
    {
        bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
        bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        AllocInfo alloc;
        alloc.CreateBuffer(bufCreateInfo, allocCreateInfo);
        alloc.m_StartValue = 0;

        allocations.push_back(alloc);
    }

    // Destroy some percentage of them.
    {
        const size_t allocationsToDestroy = round_div<size_t>((imageCount + bufCount) * (100 - percentToLeave), 100);
        for(size_t i = 0; i < allocationsToDestroy; ++i)
        {
            const size_t index = rand.Generate() % allocations.size();
            allocations[index].Destroy();
            allocations.erase(allocations.begin() + index);
        }
    }

    {
        // Set our user data pointers. A real application should probably be more clever here
        const size_t allocationCount = allocations.size();
        for(size_t i = 0; i < allocationCount; ++i)
        {
            AllocInfo &alloc = allocations[i];
            vmaSetAllocationUserData(g_hAllocator, alloc.m_Allocation, &alloc);
        }
    }

    // Fill them with meaningful data.
    UploadGpuData(allocations.data(), allocations.size());

    wchar_t fileName[MAX_PATH];
    swprintf_s(fileName, L"GPU_defragmentation_incremental_basic_A_before.json");
    SaveAllocatorStatsToFile(fileName);

    // Defragment using GPU only.
    {
        const size_t allocCount = allocations.size();

        std::vector<VmaAllocation> allocationPtrs;

        for(size_t i = 0; i < allocCount; ++i)
        {
            allocationPtrs.push_back(allocations[i].m_Allocation);
        }

        const size_t movableAllocCount = allocationPtrs.size();

        VmaDefragmentationInfo2 defragInfo = {};
        defragInfo.flags = VMA_DEFRAGMENTATION_FLAG_INCREMENTAL;
        defragInfo.allocationCount = (uint32_t)movableAllocCount;
        defragInfo.pAllocations = allocationPtrs.data();
        defragInfo.maxGpuBytesToMove = VK_WHOLE_SIZE;
        defragInfo.maxGpuAllocationsToMove = UINT32_MAX;

        VmaDefragmentationStats stats = {};
        VmaDefragmentationContext ctx = VK_NULL_HANDLE;
        VkResult res = vmaDefragmentationBegin(g_hAllocator, &defragInfo, &stats, &ctx);
        TEST(res >= VK_SUCCESS);

        res = VK_NOT_READY;

        std::vector<VmaDefragmentationPassMoveInfo> moveInfo;
        moveInfo.resize(movableAllocCount);

        while(res == VK_NOT_READY)
        {
            VmaDefragmentationPassInfo stepInfo = {};
            stepInfo.pMoves = moveInfo.data();
            stepInfo.moveCount = (uint32_t)moveInfo.size();

            res = vmaBeginDefragmentationPass(g_hAllocator, ctx, &stepInfo);
            TEST(res >= VK_SUCCESS);

            BeginSingleTimeCommands();
            std::vector<void*> newHandles;
            ProcessDefragmentationStepInfo(stepInfo);
            EndSingleTimeCommands();

            res = vmaEndDefragmentationPass(g_hAllocator, ctx);
            
            // Destroy old buffers/images and replace them with new handles.
            for(size_t i = 0; i < stepInfo.moveCount; ++i)
            {
                VmaAllocation const alloc = stepInfo.pMoves[i].allocation;
                VmaAllocationInfo vmaAllocInfo;
                vmaGetAllocationInfo(g_hAllocator, alloc, &vmaAllocInfo);
                AllocInfo* allocInfo = (AllocInfo*)vmaAllocInfo.pUserData;
                if(allocInfo->m_Buffer)
                {
                    assert(allocInfo->m_NewBuffer && !allocInfo->m_Image && !allocInfo->m_NewImage);
                    vkDestroyBuffer(g_hDevice, allocInfo->m_Buffer, g_Allocs);
                    allocInfo->m_Buffer = allocInfo->m_NewBuffer;
                    allocInfo->m_NewBuffer = VK_NULL_HANDLE;
                }
                else if(allocInfo->m_Image)
                {
                    assert(allocInfo->m_NewImage && !allocInfo->m_Buffer && !allocInfo->m_NewBuffer);
                    vkDestroyImage(g_hDevice, allocInfo->m_Image, g_Allocs);
                    allocInfo->m_Image = allocInfo->m_NewImage;
                    allocInfo->m_NewImage = VK_NULL_HANDLE;
                }
                else
                    assert(0);
            }
        }

        TEST(res >= VK_SUCCESS);
        vmaDefragmentationEnd(g_hAllocator, ctx);

        // If corruption detection is enabled, GPU defragmentation may not work on
        // memory types that have this detection active, e.g. on Intel.
#if !defined(VMA_DEBUG_DETECT_CORRUPTION) || VMA_DEBUG_DETECT_CORRUPTION == 0
        TEST(stats.allocationsMoved > 0 && stats.bytesMoved > 0);
        TEST(stats.deviceMemoryBlocksFreed > 0 && stats.bytesFreed > 0);
#endif
    }

    //ValidateGpuData(allocations.data(), allocations.size());

    swprintf_s(fileName, L"GPU_defragmentation_incremental_basic_B_after.json");
    SaveAllocatorStatsToFile(fileName);

    // Destroy all remaining buffers and images.
    for(size_t i = allocations.size(); i--; )
    {
        allocations[i].Destroy();
    }
}

void TestDefragmentationIncrementalComplex()
{
    wprintf(L"Test defragmentation incremental complex\n");
    
    std::vector<AllocInfo> allocations;

    // Create that many allocations to surely fill 3 new blocks of 256 MB.
    const std::array<uint32_t, 3> imageSizes = { 256, 512, 1024 };
    const VkDeviceSize bufSizeMin = 5ull * 1024 * 1024;
    const VkDeviceSize bufSizeMax = 10ull * 1024 * 1024;
    const VkDeviceSize totalSize = 3ull * 256 * 1024 * 1024;
    const size_t imageCount = (size_t)(totalSize / (imageSizes[0] * imageSizes[0] * 4)) / 2;
    const size_t bufCount = (size_t)(totalSize / bufSizeMin) / 2;
    const size_t percentToLeave = 30;
    RandomNumberGenerator rand = { 234522 };

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.flags = 0;

    // Create all intended images.
    for(size_t i = 0; i < imageCount; ++i)
    {
        const uint32_t size = imageSizes[rand.Generate() % 3];

        imageInfo.extent.width = size;
        imageInfo.extent.height = size;

        AllocInfo alloc;
        alloc.CreateImage(imageInfo, allocCreateInfo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        alloc.m_StartValue = 0;

        allocations.push_back(alloc);
    }

    // And all buffers
    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };

    for(size_t i = 0; i < bufCount; ++i)
    {
        bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
        bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        AllocInfo alloc;
        alloc.CreateBuffer(bufCreateInfo, allocCreateInfo);
        alloc.m_StartValue = 0;

        allocations.push_back(alloc);
    }

    // Destroy some percentage of them.
    {
        const size_t allocationsToDestroy = round_div<size_t>((imageCount + bufCount) * (100 - percentToLeave), 100);
        for(size_t i = 0; i < allocationsToDestroy; ++i)
        {
            const size_t index = rand.Generate() % allocations.size();
            allocations[index].Destroy();
            allocations.erase(allocations.begin() + index);
        }
    }

    {
        // Set our user data pointers. A real application should probably be more clever here
        const size_t allocationCount = allocations.size();
        for(size_t i = 0; i < allocationCount; ++i)
        {
            AllocInfo &alloc = allocations[i];
            vmaSetAllocationUserData(g_hAllocator, alloc.m_Allocation, &alloc);
        }
    }

    // Fill them with meaningful data.
    UploadGpuData(allocations.data(), allocations.size());

    wchar_t fileName[MAX_PATH];
    swprintf_s(fileName, L"GPU_defragmentation_incremental_complex_A_before.json");
    SaveAllocatorStatsToFile(fileName);

    std::vector<AllocInfo> additionalAllocations;

#define MakeAdditionalAllocation() \
    do { \
        { \
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16); \
            bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; \
            \
            AllocInfo alloc; \
            alloc.CreateBuffer(bufCreateInfo, allocCreateInfo); \
            \
            additionalAllocations.push_back(alloc); \
        } \
    } while(0)

    // Defragment using GPU only.
    {
        const size_t allocCount = allocations.size();

        std::vector<VmaAllocation> allocationPtrs;

        for(size_t i = 0; i < allocCount; ++i)
        {
            VmaAllocationInfo allocInfo = {};
            vmaGetAllocationInfo(g_hAllocator, allocations[i].m_Allocation, &allocInfo);

            allocationPtrs.push_back(allocations[i].m_Allocation);
        }

        const size_t movableAllocCount = allocationPtrs.size();

        VmaDefragmentationInfo2 defragInfo = {};
        defragInfo.flags = VMA_DEFRAGMENTATION_FLAG_INCREMENTAL;
        defragInfo.allocationCount = (uint32_t)movableAllocCount;
        defragInfo.pAllocations = allocationPtrs.data();
        defragInfo.maxGpuBytesToMove = VK_WHOLE_SIZE;
        defragInfo.maxGpuAllocationsToMove = UINT32_MAX;

        VmaDefragmentationStats stats = {};
        VmaDefragmentationContext ctx = VK_NULL_HANDLE;
        VkResult res = vmaDefragmentationBegin(g_hAllocator, &defragInfo, &stats, &ctx);
        TEST(res >= VK_SUCCESS);

        res = VK_NOT_READY;

        std::vector<VmaDefragmentationPassMoveInfo> moveInfo;
        moveInfo.resize(movableAllocCount);

        MakeAdditionalAllocation();

        while(res == VK_NOT_READY)
        {
            VmaDefragmentationPassInfo stepInfo = {};
            stepInfo.pMoves = moveInfo.data();
            stepInfo.moveCount = (uint32_t)moveInfo.size();

            res = vmaBeginDefragmentationPass(g_hAllocator, ctx, &stepInfo);
            TEST(res >= VK_SUCCESS);

            MakeAdditionalAllocation();

            BeginSingleTimeCommands();
            ProcessDefragmentationStepInfo(stepInfo);
            EndSingleTimeCommands();

            res = vmaEndDefragmentationPass(g_hAllocator, ctx);

            // Destroy old buffers/images and replace them with new handles.
            for(size_t i = 0; i < stepInfo.moveCount; ++i)
            {
                VmaAllocation const alloc = stepInfo.pMoves[i].allocation;
                VmaAllocationInfo vmaAllocInfo;
                vmaGetAllocationInfo(g_hAllocator, alloc, &vmaAllocInfo);
                AllocInfo* allocInfo = (AllocInfo*)vmaAllocInfo.pUserData;
                if(allocInfo->m_Buffer)
                {
                    assert(allocInfo->m_NewBuffer && !allocInfo->m_Image && !allocInfo->m_NewImage);
                    vkDestroyBuffer(g_hDevice, allocInfo->m_Buffer, g_Allocs);
                    allocInfo->m_Buffer = allocInfo->m_NewBuffer;
                    allocInfo->m_NewBuffer = VK_NULL_HANDLE;
                }
                else if(allocInfo->m_Image)
                {
                    assert(allocInfo->m_NewImage && !allocInfo->m_Buffer && !allocInfo->m_NewBuffer);
                    vkDestroyImage(g_hDevice, allocInfo->m_Image, g_Allocs);
                    allocInfo->m_Image = allocInfo->m_NewImage;
                    allocInfo->m_NewImage = VK_NULL_HANDLE;
                }
                else
                    assert(0);
            }

            MakeAdditionalAllocation();
        }

        TEST(res >= VK_SUCCESS);
        vmaDefragmentationEnd(g_hAllocator, ctx);

        // If corruption detection is enabled, GPU defragmentation may not work on
        // memory types that have this detection active, e.g. on Intel.
#if !defined(VMA_DEBUG_DETECT_CORRUPTION) || VMA_DEBUG_DETECT_CORRUPTION == 0
        TEST(stats.allocationsMoved > 0 && stats.bytesMoved > 0);
        TEST(stats.deviceMemoryBlocksFreed > 0 && stats.bytesFreed > 0);
#endif
    }

    //ValidateGpuData(allocations.data(), allocations.size());

    swprintf_s(fileName, L"GPU_defragmentation_incremental_complex_B_after.json");
    SaveAllocatorStatsToFile(fileName);

    // Destroy all remaining buffers.
    for(size_t i = allocations.size(); i--; )
    {
        allocations[i].Destroy();
    }

    for(size_t i = additionalAllocations.size(); i--; )
    {
        additionalAllocations[i].Destroy();
    }
}


static void TestUserData()
{
    VkResult res;

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufCreateInfo.size = 0x10000;

    for(uint32_t testIndex = 0; testIndex < 2; ++testIndex)
    {
        // Opaque pointer
        {

            void* numberAsPointer = (void*)(size_t)0xC2501FF3u;
            void* pointerToSomething = &res;

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            allocCreateInfo.pUserData = numberAsPointer;
            if(testIndex == 1)
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            VkBuffer buf; VmaAllocation alloc; VmaAllocationInfo allocInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(allocInfo.pUserData = numberAsPointer);

            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            TEST(allocInfo.pUserData == numberAsPointer);

            vmaSetAllocationUserData(g_hAllocator, alloc, pointerToSomething);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            TEST(allocInfo.pUserData == pointerToSomething);

            vmaDestroyBuffer(g_hAllocator, buf, alloc);
        }

        // String
        {
            const char* name1 = "Buffer name \\\"\'<>&% \nSecond line .,;=";
            const char* name2 = "2";
            const size_t name1Len = strlen(name1);

            char* name1Buf = new char[name1Len + 1];
            strcpy_s(name1Buf, name1Len + 1, name1);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
            allocCreateInfo.pUserData = name1Buf;
            if(testIndex == 1)
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            VkBuffer buf; VmaAllocation alloc; VmaAllocationInfo allocInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(allocInfo.pUserData != nullptr && allocInfo.pUserData != name1Buf);
            TEST(strcmp(name1, (const char*)allocInfo.pUserData) == 0);

            delete[] name1Buf;

            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            TEST(strcmp(name1, (const char*)allocInfo.pUserData) == 0);

            vmaSetAllocationUserData(g_hAllocator, alloc, (void*)name2);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            TEST(strcmp(name2, (const char*)allocInfo.pUserData) == 0);

            vmaSetAllocationUserData(g_hAllocator, alloc, nullptr);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            TEST(allocInfo.pUserData == nullptr);

            vmaDestroyBuffer(g_hAllocator, buf, alloc);
        }
    }
}

static void TestInvalidAllocations()
{
    VkResult res;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    // Try to allocate 0 bytes.
    {
        VkMemoryRequirements memReq = {};
        memReq.size = 0; // !!!
        memReq.alignment = 4;
        memReq.memoryTypeBits = UINT32_MAX;
        VmaAllocation alloc = VK_NULL_HANDLE;
        res = vmaAllocateMemory(g_hAllocator, &memReq, &allocCreateInfo, &alloc, nullptr);
        TEST(res == VK_ERROR_VALIDATION_FAILED_EXT && alloc == VK_NULL_HANDLE);
    }

    // Try to create buffer with size = 0.
    {
        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufCreateInfo.size = 0; // !!!
        VkBuffer buf = VK_NULL_HANDLE;
        VmaAllocation alloc = VK_NULL_HANDLE;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, nullptr);
        TEST(res == VK_ERROR_VALIDATION_FAILED_EXT && buf == VK_NULL_HANDLE && alloc == VK_NULL_HANDLE);
    }

    // Try to create image with one dimension = 0.
    {
        VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        imageCreateInfo.extent.width = 128;
        imageCreateInfo.extent.height = 0; // !!!
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation alloc = VK_NULL_HANDLE;
        res = vmaCreateImage(g_hAllocator, &imageCreateInfo, &allocCreateInfo, &image, &alloc, nullptr);
        TEST(res == VK_ERROR_VALIDATION_FAILED_EXT && image == VK_NULL_HANDLE && alloc == VK_NULL_HANDLE);
    }
}

static void TestMemoryRequirements()
{
    VkResult res;
    VkBuffer buf;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo;

    const VkPhysicalDeviceMemoryProperties* memProps;
    vmaGetMemoryProperties(g_hAllocator, &memProps);

    VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.size = 128;

    VmaAllocationCreateInfo allocCreateInfo = {};

    // No requirements.
    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    TEST(res == VK_SUCCESS);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // Usage.
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.requiredFlags = 0;
    allocCreateInfo.preferredFlags = 0;
    allocCreateInfo.memoryTypeBits = UINT32_MAX;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    TEST(res == VK_SUCCESS);
    TEST(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // Required flags, preferred flags.
    allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    allocCreateInfo.memoryTypeBits = 0;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    TEST(res == VK_SUCCESS);
    TEST(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    TEST(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // memoryTypeBits.
    const uint32_t memType = allocInfo.memoryType;
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.requiredFlags = 0;
    allocCreateInfo.preferredFlags = 0;
    allocCreateInfo.memoryTypeBits = 1u << memType;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    TEST(res == VK_SUCCESS);
    TEST(allocInfo.memoryType == memType);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

}

static void TestGetAllocatorInfo()
{
    wprintf(L"Test vnaGetAllocatorInfo\n");

    VmaAllocatorInfo allocInfo = {};
    vmaGetAllocatorInfo(g_hAllocator, &allocInfo);
    TEST(allocInfo.instance == g_hVulkanInstance);
    TEST(allocInfo.physicalDevice == g_hPhysicalDevice);
    TEST(allocInfo.device == g_hDevice);
}

static void TestBasics()
{
    wprintf(L"Test basics\n");

    VkResult res;

    TestGetAllocatorInfo();

    TestMemoryRequirements();

    // Lost allocation
    {
        VmaAllocation alloc = VK_NULL_HANDLE;
        vmaCreateLostAllocation(g_hAllocator, &alloc);
        TEST(alloc != VK_NULL_HANDLE);

        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
        TEST(allocInfo.deviceMemory == VK_NULL_HANDLE);
        TEST(allocInfo.size == 0);

        vmaFreeMemory(g_hAllocator, alloc);
    }
    
    // Allocation that is MAPPED and not necessarily HOST_VISIBLE.
    {
        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufCreateInfo.size = 128;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer buf; VmaAllocation alloc; VmaAllocationInfo allocInfo;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
        TEST(res == VK_SUCCESS);

        vmaDestroyBuffer(g_hAllocator, buf, alloc);

        // Same with OWN_MEMORY.
        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
        TEST(res == VK_SUCCESS);

        vmaDestroyBuffer(g_hAllocator, buf, alloc);
    }

    TestUserData();

    TestInvalidAllocations();
}

static void TestAllocationVersusResourceSize()
{
    wprintf(L"Test allocation versus resource size\n");

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 22921; // Prime number
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    for(uint32_t i = 0; i < 2; ++i)
    {
        allocCreateInfo.flags = (i == 1) ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;

        AllocInfo info;
        info.CreateBuffer(bufCreateInfo, allocCreateInfo);
        
        VmaAllocationInfo allocInfo = {};
        vmaGetAllocationInfo(g_hAllocator, info.m_Allocation, &allocInfo);
        //wprintf(L"  Buffer size = %llu, allocation size = %llu\n", bufCreateInfo.size, allocInfo.size);

        // Map and test accessing entire area of the allocation, not only the buffer.
        void* mappedPtr = nullptr;
        VkResult res = vmaMapMemory(g_hAllocator, info.m_Allocation, &mappedPtr);
        TEST(res == VK_SUCCESS);

        memset(mappedPtr, 0xCC, (size_t)allocInfo.size);

        vmaUnmapMemory(g_hAllocator, info.m_Allocation);

        info.Destroy();
    }
}

static void TestPool_MinBlockCount()
{
#if defined(VMA_DEBUG_MARGIN) && VMA_DEBUG_MARGIN > 0
    return;
#endif

    wprintf(L"Test Pool MinBlockCount\n");
    VkResult res;

    static const VkDeviceSize ALLOC_SIZE = 512ull * 1024;
    static const VkDeviceSize BLOCK_SIZE = ALLOC_SIZE * 2; // Each block can fit 2 allocations.

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_COPY;

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufCreateInfo.size = ALLOC_SIZE;

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.blockSize = BLOCK_SIZE;
    poolCreateInfo.minBlockCount = 2; // At least 2 blocks always present.
    res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    VmaPool pool = VK_NULL_HANDLE;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS && pool != VK_NULL_HANDLE);

    // Check that there are 2 blocks preallocated as requested.
    VmaPoolStats begPoolStats = {};
    vmaGetPoolStats(g_hAllocator, pool, &begPoolStats);
    TEST(begPoolStats.blockCount == 2 && begPoolStats.allocationCount == 0 && begPoolStats.size == BLOCK_SIZE * 2);

    // Allocate 5 buffers to create 3 blocks.
    static const uint32_t BUF_COUNT = 5;
    allocCreateInfo.pool = pool;
    std::vector<AllocInfo> allocs(BUF_COUNT);
    for(uint32_t i = 0; i < BUF_COUNT; ++i)
    {
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &allocs[i].m_Buffer, &allocs[i].m_Allocation, nullptr);
        TEST(res == VK_SUCCESS && allocs[i].m_Buffer != VK_NULL_HANDLE && allocs[i].m_Allocation != VK_NULL_HANDLE);
    }

    // Check that there are really 3 blocks.
    VmaPoolStats poolStats2 = {};
    vmaGetPoolStats(g_hAllocator, pool, &poolStats2);
    TEST(poolStats2.blockCount == 3 && poolStats2.allocationCount == BUF_COUNT && poolStats2.size == BLOCK_SIZE * 3);

    // Free two first allocations to make one block empty.
    allocs[0].Destroy();
    allocs[1].Destroy();

    // Check that there are still 3 blocks due to hysteresis.
    VmaPoolStats poolStats3 = {};
    vmaGetPoolStats(g_hAllocator, pool, &poolStats3);
    TEST(poolStats3.blockCount == 3 && poolStats3.allocationCount == BUF_COUNT - 2 && poolStats2.size == BLOCK_SIZE * 3);

    // Free the last allocation to make second block empty.
    allocs[BUF_COUNT - 1].Destroy();

    // Check that there are now 2 blocks only.
    VmaPoolStats poolStats4 = {};
    vmaGetPoolStats(g_hAllocator, pool, &poolStats4);
    TEST(poolStats4.blockCount == 2 && poolStats4.allocationCount == BUF_COUNT - 3 && poolStats4.size == BLOCK_SIZE * 2);

    // Cleanup.
    for(size_t i = allocs.size(); i--; )
    {
        allocs[i].Destroy();
    }
    vmaDestroyPool(g_hAllocator, pool);
}

void TestHeapSizeLimit()
{
    const VkDeviceSize HEAP_SIZE_LIMIT = 100ull * 1024 * 1024; // 100 MB
    const VkDeviceSize BLOCK_SIZE      =  10ull * 1024 * 1024; // 10 MB

    VkDeviceSize heapSizeLimit[VK_MAX_MEMORY_HEAPS];
    for(uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i)
    {
        heapSizeLimit[i] = HEAP_SIZE_LIMIT;
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = g_hPhysicalDevice;
    allocatorCreateInfo.device = g_hDevice;
    allocatorCreateInfo.instance = g_hVulkanInstance;
    allocatorCreateInfo.pHeapSizeLimit = heapSizeLimit;

    VmaAllocator hAllocator;
    VkResult res = vmaCreateAllocator(&allocatorCreateInfo, &hAllocator);
    TEST(res == VK_SUCCESS);

    struct Item
    {
        VkBuffer hBuf;
        VmaAllocation hAlloc;
    };
    std::vector<Item> items;

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // 1. Allocate two blocks of dedicated memory, half the size of BLOCK_SIZE.
    VmaAllocationInfo dedicatedAllocInfo;
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        bufCreateInfo.size = BLOCK_SIZE / 2;

        for(size_t i = 0; i < 2; ++i)
        {
            Item item;
            res = vmaCreateBuffer(hAllocator, &bufCreateInfo, &allocCreateInfo, &item.hBuf, &item.hAlloc, &dedicatedAllocInfo);
            TEST(res == VK_SUCCESS);
            items.push_back(item);
        }
    }

    // Create pool to make sure allocations must be out of this memory type.
    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = dedicatedAllocInfo.memoryType;
    poolCreateInfo.blockSize = BLOCK_SIZE;

    VmaPool hPool;
    res = vmaCreatePool(hAllocator, &poolCreateInfo, &hPool);
    TEST(res == VK_SUCCESS);

    // 2. Allocate normal buffers from all the remaining memory.
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.pool = hPool;

        bufCreateInfo.size = BLOCK_SIZE / 2;

        const size_t bufCount = ((HEAP_SIZE_LIMIT / BLOCK_SIZE) - 1) * 2;
        for(size_t i = 0; i < bufCount; ++i)
        {
            Item item;
            res = vmaCreateBuffer(hAllocator, &bufCreateInfo, &allocCreateInfo, &item.hBuf, &item.hAlloc, nullptr);
            TEST(res == VK_SUCCESS);
            items.push_back(item);
        }
    }

    // 3. Allocation of one more (even small) buffer should fail.
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.pool = hPool;

        bufCreateInfo.size = 128;

        VkBuffer hBuf;
        VmaAllocation hAlloc;
        res = vmaCreateBuffer(hAllocator, &bufCreateInfo, &allocCreateInfo, &hBuf, &hAlloc, nullptr);
        TEST(res == VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    // Destroy everything.
    for(size_t i = items.size(); i--; )
    {
        vmaDestroyBuffer(hAllocator, items[i].hBuf, items[i].hAlloc);
    }

    vmaDestroyPool(hAllocator, hPool);

    vmaDestroyAllocator(hAllocator);
}

#if VMA_DEBUG_MARGIN
static void TestDebugMargin()
{
    if(VMA_DEBUG_MARGIN == 0)
    {
        return;
    }

    VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    // Create few buffers of different size.
    const size_t BUF_COUNT = 10;
    BufferInfo buffers[BUF_COUNT];
    VmaAllocationInfo allocInfo[BUF_COUNT];
    for(size_t i = 0; i < 10; ++i)
    {
        bufInfo.size = (VkDeviceSize)(i + 1) * 64;
        // Last one will be mapped.
        allocCreateInfo.flags = (i == BUF_COUNT - 1) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

        VkResult res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buffers[i].Buffer, &buffers[i].Allocation, &allocInfo[i]);
        TEST(res == VK_SUCCESS);
        // Margin is preserved also at the beginning of a block.
        TEST(allocInfo[i].offset >= VMA_DEBUG_MARGIN);

        if(i == BUF_COUNT - 1)
        {
            // Fill with data.
            TEST(allocInfo[i].pMappedData != nullptr);
            // Uncomment this "+ 1" to overwrite past end of allocation and check corruption detection.
            memset(allocInfo[i].pMappedData, 0xFF, bufInfo.size /* + 1 */);
        }
    }

    // Check if their offsets preserve margin between them.
    std::sort(allocInfo, allocInfo + BUF_COUNT, [](const VmaAllocationInfo& lhs, const VmaAllocationInfo& rhs) -> bool
    {
        if(lhs.deviceMemory != rhs.deviceMemory)
        {
            return lhs.deviceMemory < rhs.deviceMemory;
        }
        return lhs.offset < rhs.offset;
    });
    for(size_t i = 1; i < BUF_COUNT; ++i)
    {
        if(allocInfo[i].deviceMemory == allocInfo[i - 1].deviceMemory)
        {
            TEST(allocInfo[i].offset >= allocInfo[i - 1].offset + VMA_DEBUG_MARGIN);
        }
    }

    VkResult res = vmaCheckCorruption(g_hAllocator, UINT32_MAX);
    TEST(res == VK_SUCCESS);

    // Destroy all buffers.
    for(size_t i = BUF_COUNT; i--; )
    {
        vmaDestroyBuffer(g_hAllocator, buffers[i].Buffer, buffers[i].Allocation);
    }
}
#endif

static void TestLinearAllocator()
{
    wprintf(L"Test linear allocator\n");

    RandomNumberGenerator rand{645332};

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = 1024; // Whatever.
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    poolCreateInfo.blockSize = 1024 * 300;
    poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    poolCreateInfo.minBlockCount = poolCreateInfo.maxBlockCount = 1;

    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    VkBufferCreateInfo bufCreateInfo = sampleBufCreateInfo;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;

    constexpr size_t maxBufCount = 100;
    std::vector<BufferInfo> bufInfo;

    constexpr VkDeviceSize bufSizeMin = 16;
    constexpr VkDeviceSize bufSizeMax = 1024;
    VmaAllocationInfo allocInfo;
    VkDeviceSize prevOffset = 0;

    // Test one-time free.
    for(size_t i = 0; i < 2; ++i)
    {
        // Allocate number of buffers of varying size that surely fit into this block.
        VkDeviceSize bufSumSize = 0;
        for(size_t i = 0; i < maxBufCount; ++i)
        {
			bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(i == 0 || allocInfo.offset > prevOffset);
            bufInfo.push_back(newBufInfo);
            prevOffset = allocInfo.offset;
            bufSumSize += bufCreateInfo.size;
        }

        // Validate pool stats.
        VmaPoolStats stats;
        vmaGetPoolStats(g_hAllocator, pool, &stats);
        TEST(stats.size == poolCreateInfo.blockSize);
        TEST(stats.unusedSize = poolCreateInfo.blockSize - bufSumSize);
        TEST(stats.allocationCount == bufInfo.size());

        // Destroy the buffers in random order.
        while(!bufInfo.empty())
        {
            const size_t indexToDestroy = rand.Generate() % bufInfo.size();
            const BufferInfo& currBufInfo = bufInfo[indexToDestroy];
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.erase(bufInfo.begin() + indexToDestroy);
        }
    }

    // Test stack.
    {
        // Allocate number of buffers of varying size that surely fit into this block.
        for(size_t i = 0; i < maxBufCount; ++i)
        {
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(i == 0 || allocInfo.offset > prevOffset);
            bufInfo.push_back(newBufInfo);
            prevOffset = allocInfo.offset;
        }

        // Destroy few buffers from top of the stack.
        for(size_t i = 0; i < maxBufCount / 5; ++i)
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }

        // Create some more
        for(size_t i = 0; i < maxBufCount / 5; ++i)
        {
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(i == 0 || allocInfo.offset > prevOffset);
            bufInfo.push_back(newBufInfo);
            prevOffset = allocInfo.offset;
        }

        // Destroy the buffers in reverse order.
        while(!bufInfo.empty())
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }
    }

    // Test ring buffer.
    {
        // Allocate number of buffers that surely fit into this block.
        bufCreateInfo.size = bufSizeMax;
        for(size_t i = 0; i < maxBufCount; ++i)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(i == 0 || allocInfo.offset > prevOffset);
            bufInfo.push_back(newBufInfo);
            prevOffset = allocInfo.offset;
        }

        // Free and allocate new buffers so many times that we make sure we wrap-around at least once.
        const size_t buffersPerIter = maxBufCount / 10 - 1;
        const size_t iterCount = poolCreateInfo.blockSize / bufCreateInfo.size / buffersPerIter * 2;
        for(size_t iter = 0; iter < iterCount; ++iter)
        {
            for(size_t bufPerIter = 0; bufPerIter < buffersPerIter; ++bufPerIter)
            {
                const BufferInfo& currBufInfo = bufInfo.front();
                vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
                bufInfo.erase(bufInfo.begin());
            }
            for(size_t bufPerIter = 0; bufPerIter < buffersPerIter; ++bufPerIter)
            {
                BufferInfo newBufInfo;
                res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                    &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
                TEST(res == VK_SUCCESS);
                bufInfo.push_back(newBufInfo);
            }
        }
        
        // Allocate buffers until we reach out-of-memory.
        uint32_t debugIndex = 0;
        while(res == VK_SUCCESS)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            if(res == VK_SUCCESS)
            {
                bufInfo.push_back(newBufInfo);
            }
            else
            {
                TEST(res == VK_ERROR_OUT_OF_DEVICE_MEMORY);
            }
            ++debugIndex;
        }

        // Destroy the buffers in random order.
        while(!bufInfo.empty())
        {
            const size_t indexToDestroy = rand.Generate() % bufInfo.size();
            const BufferInfo& currBufInfo = bufInfo[indexToDestroy];
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.erase(bufInfo.begin() + indexToDestroy);
        }
    }

    // Test double stack.
    {
        // Allocate number of buffers of varying size that surely fit into this block, alternate from bottom/top.
        VkDeviceSize prevOffsetLower = 0;
        VkDeviceSize prevOffsetUpper = poolCreateInfo.blockSize;
        for(size_t i = 0; i < maxBufCount; ++i)
        {
            const bool upperAddress = (i % 2) != 0;
            if(upperAddress)
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            else
                allocCreateInfo.flags &= ~VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            if(upperAddress)
            {
                TEST(allocInfo.offset < prevOffsetUpper);
                prevOffsetUpper = allocInfo.offset;
            }
            else
            {
                TEST(allocInfo.offset >= prevOffsetLower);
                prevOffsetLower = allocInfo.offset;
            }
            TEST(prevOffsetLower < prevOffsetUpper);
            bufInfo.push_back(newBufInfo);
        }

        // Destroy few buffers from top of the stack.
        for(size_t i = 0; i < maxBufCount / 5; ++i)
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }

        // Create some more
        for(size_t i = 0; i < maxBufCount / 5; ++i)
        {
            const bool upperAddress = (i % 2) != 0;
            if(upperAddress)
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            else
                allocCreateInfo.flags &= ~VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
        }

        // Destroy the buffers in reverse order.
        while(!bufInfo.empty())
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }

        // Create buffers on both sides until we reach out of memory.
        prevOffsetLower = 0;
        prevOffsetUpper = poolCreateInfo.blockSize;
        res = VK_SUCCESS;
        for(size_t i = 0; res == VK_SUCCESS; ++i)
        {
            const bool upperAddress = (i % 2) != 0;
            if(upperAddress)
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            else
                allocCreateInfo.flags &= ~VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            if(res == VK_SUCCESS)
            {
                if(upperAddress)
                {
                    TEST(allocInfo.offset < prevOffsetUpper);
                    prevOffsetUpper = allocInfo.offset;
                }
                else
                {
                    TEST(allocInfo.offset >= prevOffsetLower);
                    prevOffsetLower = allocInfo.offset;
                }
                TEST(prevOffsetLower < prevOffsetUpper);
                bufInfo.push_back(newBufInfo);
            }
        }

        // Destroy the buffers in random order.
        while(!bufInfo.empty())
        {
            const size_t indexToDestroy = rand.Generate() % bufInfo.size();
            const BufferInfo& currBufInfo = bufInfo[indexToDestroy];
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.erase(bufInfo.begin() + indexToDestroy);
        }

        // Create buffers on upper side only, constant size, until we reach out of memory.
        prevOffsetUpper = poolCreateInfo.blockSize;
        res = VK_SUCCESS;
        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
        bufCreateInfo.size = bufSizeMax;
        for(size_t i = 0; res == VK_SUCCESS; ++i)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST(allocInfo.offset < prevOffsetUpper);
                prevOffsetUpper = allocInfo.offset;
                bufInfo.push_back(newBufInfo);
            }
        }

        // Destroy the buffers in reverse order.
        while(!bufInfo.empty())
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }
    }

    // Test ring buffer with lost allocations.
    {
        // Allocate number of buffers until pool is full.
        // Notice CAN_BECOME_LOST flag and call to vmaSetCurrentFrameIndex.
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT;
        res = VK_SUCCESS;
        for(size_t i = 0; res == VK_SUCCESS; ++i)
        {
            vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);

            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            if(res == VK_SUCCESS)
                bufInfo.push_back(newBufInfo);
        }

        // Free first half of it.
        {
            const size_t buffersToDelete = bufInfo.size() / 2;
            for(size_t i = 0; i < buffersToDelete; ++i)
            {
                vmaDestroyBuffer(g_hAllocator, bufInfo[i].Buffer, bufInfo[i].Allocation);
            }
            bufInfo.erase(bufInfo.begin(), bufInfo.begin() + buffersToDelete);
        }

        // Allocate number of buffers until pool is full again.
        // This way we make sure ring buffers wraps around, front in in the middle.
        res = VK_SUCCESS;
        for(size_t i = 0; res == VK_SUCCESS; ++i)
        {
            vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);

            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            if(res == VK_SUCCESS)
                bufInfo.push_back(newBufInfo);
        }

        VkDeviceSize firstNewOffset;
        {
            vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

            // Allocate a large buffer with CAN_MAKE_OTHER_LOST.
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
            bufCreateInfo.size = bufSizeMax;

            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
            firstNewOffset = allocInfo.offset;

            // Make sure at least one buffer from the beginning became lost.
            vmaGetAllocationInfo(g_hAllocator, bufInfo[0].Allocation, &allocInfo);
            TEST(allocInfo.deviceMemory == VK_NULL_HANDLE);
        }

#if 0 // TODO Fix and uncomment. Failing on Intel.
        // Allocate more buffers that CAN_MAKE_OTHER_LOST until we wrap-around with this.
        size_t newCount = 1;
        for(;;)
        {
            vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

            bufCreateInfo.size = align_up<VkDeviceSize>(bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin), 16);

            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);

            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
            ++newCount;
            if(allocInfo.offset < firstNewOffset)
                break;
        }
#endif

        // Delete buffers that are lost.
        for(size_t i = bufInfo.size(); i--; )
        {
            vmaGetAllocationInfo(g_hAllocator, bufInfo[i].Allocation, &allocInfo);
            if(allocInfo.deviceMemory == VK_NULL_HANDLE)
            {
                vmaDestroyBuffer(g_hAllocator, bufInfo[i].Buffer, bufInfo[i].Allocation);
                bufInfo.erase(bufInfo.begin() + i);
            }
        }

        // Test vmaMakePoolAllocationsLost
        {
            vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

            size_t lostAllocCount = 0;
            vmaMakePoolAllocationsLost(g_hAllocator, pool, &lostAllocCount);
            TEST(lostAllocCount > 0);

            size_t realLostAllocCount = 0;
            for(size_t i = 0; i < bufInfo.size(); ++i)
            {
                vmaGetAllocationInfo(g_hAllocator, bufInfo[i].Allocation, &allocInfo);
                if(allocInfo.deviceMemory == VK_NULL_HANDLE)
                    ++realLostAllocCount;
            }
            TEST(realLostAllocCount == lostAllocCount);
        }

        // Destroy all the buffers in forward order.
        for(size_t i = 0; i < bufInfo.size(); ++i)
            vmaDestroyBuffer(g_hAllocator, bufInfo[i].Buffer, bufInfo[i].Allocation);
        bufInfo.clear();
    }

    vmaDestroyPool(g_hAllocator, pool);
}

static void TestLinearAllocatorMultiBlock()
{
    wprintf(L"Test linear allocator multi block\n");

    RandomNumberGenerator rand{345673};

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = 1024 * 1024;
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    VkBufferCreateInfo bufCreateInfo = sampleBufCreateInfo;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;
    
    std::vector<BufferInfo> bufInfo;
    VmaAllocationInfo allocInfo;

    // Test one-time free.
    {
        // Allocate buffers until we move to a second block.
        VkDeviceMemory lastMem = VK_NULL_HANDLE;
        for(uint32_t i = 0; ; ++i)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
            if(lastMem && allocInfo.deviceMemory != lastMem)
            {
                break;
            }
            lastMem = allocInfo.deviceMemory;
        }

        TEST(bufInfo.size() > 2);

        // Make sure that pool has now two blocks.
        VmaPoolStats poolStats = {};
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        TEST(poolStats.blockCount == 2);

        // Destroy all the buffers in random order.
        while(!bufInfo.empty())
        {
            const size_t indexToDestroy = rand.Generate() % bufInfo.size();
            const BufferInfo& currBufInfo = bufInfo[indexToDestroy];
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.erase(bufInfo.begin() + indexToDestroy);
        }

        // Make sure that pool has now at most one block.
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        TEST(poolStats.blockCount <= 1);
    }

    // Test stack.
    {
        // Allocate buffers until we move to a second block.
        VkDeviceMemory lastMem = VK_NULL_HANDLE;
        for(uint32_t i = 0; ; ++i)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
            if(lastMem && allocInfo.deviceMemory != lastMem)
            {
                break;
            }
            lastMem = allocInfo.deviceMemory;
        }

        TEST(bufInfo.size() > 2);

        // Add few more buffers.
        for(uint32_t i = 0; i < 5; ++i)
        {
            BufferInfo newBufInfo;
            res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
                &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            bufInfo.push_back(newBufInfo);
        }

        // Make sure that pool has now two blocks.
        VmaPoolStats poolStats = {};
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        TEST(poolStats.blockCount == 2);
        
        // Delete half of buffers, LIFO.
        for(size_t i = 0, countToDelete = bufInfo.size() / 2; i < countToDelete; ++i)
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }

        // Add one more buffer.
        BufferInfo newBufInfo;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        // Make sure that pool has now one block.
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        TEST(poolStats.blockCount == 1);

        // Delete all the remaining buffers, LIFO.
        while(!bufInfo.empty())
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }
    }

    vmaDestroyPool(g_hAllocator, pool);
}

static void ManuallyTestLinearAllocator()
{
    VmaStats origStats;
    vmaCalculateStats(g_hAllocator, &origStats);

    wprintf(L"Manually test linear allocator\n");

    RandomNumberGenerator rand{645332};

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = 1024; // Whatever.
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    poolCreateInfo.blockSize = 10 * 1024;
    poolCreateInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    poolCreateInfo.minBlockCount = poolCreateInfo.maxBlockCount = 1;

    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    VkBufferCreateInfo bufCreateInfo = sampleBufCreateInfo;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;

    std::vector<BufferInfo> bufInfo;
    VmaAllocationInfo allocInfo;
    BufferInfo newBufInfo;

    // Test double stack.
    {
        /*
        Lower: Buffer 32 B, Buffer 1024 B, Buffer 32 B
        Upper: Buffer 16 B, Buffer 1024 B, Buffer 128 B

        Totally:
        1 block allocated
        10240 Vulkan bytes
        6 new allocations
        2256 bytes in allocations
        */

        bufCreateInfo.size = 32;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        bufCreateInfo.size = 1024;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        bufCreateInfo.size = 32;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;

        bufCreateInfo.size = 128;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        bufCreateInfo.size = 1024;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        bufCreateInfo.size = 16;
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);

        VmaStats currStats;
        vmaCalculateStats(g_hAllocator, &currStats);
        VmaPoolStats poolStats;
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);

        char* statsStr = nullptr;
        vmaBuildStatsString(g_hAllocator, &statsStr, VK_TRUE);

        // PUT BREAKPOINT HERE TO CHECK.
        // Inspect: currStats versus origStats, poolStats, statsStr.
        int I = 0;

        vmaFreeStatsString(g_hAllocator, statsStr);

        // Destroy the buffers in reverse order.
        while(!bufInfo.empty())
        {
            const BufferInfo& currBufInfo = bufInfo.back();
            vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
            bufInfo.pop_back();
        }
    }

    vmaDestroyPool(g_hAllocator, pool);
}

static void BenchmarkAlgorithmsCase(FILE* file,
    uint32_t algorithm,
    bool empty,
    VmaAllocationCreateFlags allocStrategy,
    FREE_ORDER freeOrder)
{
    RandomNumberGenerator rand{16223};

    const VkDeviceSize bufSizeMin = 32;
    const VkDeviceSize bufSizeMax = 1024;
    const size_t maxBufCapacity = 10000;
    const uint32_t iterationCount = 10;

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = bufSizeMax;
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    poolCreateInfo.blockSize = bufSizeMax * maxBufCapacity;
    poolCreateInfo.flags |= algorithm;
    poolCreateInfo.minBlockCount = poolCreateInfo.maxBlockCount = 1;

    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    // Buffer created just to get memory requirements. Never bound to any memory.
    VkBuffer dummyBuffer = VK_NULL_HANDLE;
    res = vkCreateBuffer(g_hDevice, &sampleBufCreateInfo, g_Allocs, &dummyBuffer);
    TEST(res == VK_SUCCESS && dummyBuffer);

    VkMemoryRequirements memReq = {};
    vkGetBufferMemoryRequirements(g_hDevice, dummyBuffer, &memReq);

    vkDestroyBuffer(g_hDevice, dummyBuffer, g_Allocs);

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;
    allocCreateInfo.flags = allocStrategy;

    VmaAllocation alloc;
    std::vector<VmaAllocation> baseAllocations;

    if(!empty)
    {
        // Make allocations up to 1/3 of pool size.
        VkDeviceSize totalSize = 0;
        while(totalSize < poolCreateInfo.blockSize / 3)
        {
            // This test intentionally allows sizes that are aligned to 4 or 16 bytes.
            // This is theoretically allowed and already uncovered one bug.
            memReq.size = bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin);
            res = vmaAllocateMemory(g_hAllocator, &memReq, &allocCreateInfo, &alloc, nullptr);
            TEST(res == VK_SUCCESS);
            baseAllocations.push_back(alloc);
            totalSize += memReq.size;
        }

        // Delete half of them, choose randomly.
        size_t allocsToDelete = baseAllocations.size() / 2;
        for(size_t i = 0; i < allocsToDelete; ++i)
        {
            const size_t index = (size_t)rand.Generate() % baseAllocations.size();
            vmaFreeMemory(g_hAllocator, baseAllocations[index]);
            baseAllocations.erase(baseAllocations.begin() + index);
        }
    }

    // BENCHMARK
    const size_t allocCount = maxBufCapacity / 3;
    std::vector<VmaAllocation> testAllocations;
    testAllocations.reserve(allocCount);
    duration allocTotalDuration = duration::zero();
    duration freeTotalDuration = duration::zero();
    for(uint32_t iterationIndex = 0; iterationIndex < iterationCount; ++iterationIndex)
    {
        // Allocations
        time_point allocTimeBeg = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < allocCount; ++i)
        {
            memReq.size = bufSizeMin + rand.Generate() % (bufSizeMax - bufSizeMin);
            res = vmaAllocateMemory(g_hAllocator, &memReq, &allocCreateInfo, &alloc, nullptr);
            TEST(res == VK_SUCCESS);
            testAllocations.push_back(alloc);
        }
        allocTotalDuration += std::chrono::high_resolution_clock::now() - allocTimeBeg;

        // Deallocations
        switch(freeOrder)
        {
        case FREE_ORDER::FORWARD:
            // Leave testAllocations unchanged.
            break;
        case FREE_ORDER::BACKWARD:
            std::reverse(testAllocations.begin(), testAllocations.end());
            break;
        case FREE_ORDER::RANDOM:
            std::shuffle(testAllocations.begin(), testAllocations.end(), MyUniformRandomNumberGenerator(rand));
            break;
        default: assert(0);
        }

        time_point freeTimeBeg = std::chrono::high_resolution_clock::now();
        for(size_t i = 0; i < allocCount; ++i)
            vmaFreeMemory(g_hAllocator, testAllocations[i]);
        freeTotalDuration += std::chrono::high_resolution_clock::now() - freeTimeBeg;

        testAllocations.clear();
    }

    // Delete baseAllocations
    while(!baseAllocations.empty())
    {
        vmaFreeMemory(g_hAllocator, baseAllocations.back());
        baseAllocations.pop_back();
    }

    vmaDestroyPool(g_hAllocator, pool);

    const float allocTotalSeconds = ToFloatSeconds(allocTotalDuration);
    const float freeTotalSeconds  = ToFloatSeconds(freeTotalDuration);

    printf("    Algorithm=%s %s Allocation=%s FreeOrder=%s: allocations %g s, free %g s\n",
        AlgorithmToStr(algorithm),
        empty ? "Empty" : "Not empty",
        GetAllocationStrategyName(allocStrategy),
        FREE_ORDER_NAMES[(size_t)freeOrder],
        allocTotalSeconds,
        freeTotalSeconds);

    if(file)
    {
        std::string currTime;
        CurrentTimeToStr(currTime);

        fprintf(file, "%s,%s,%s,%u,%s,%s,%g,%g\n",
            CODE_DESCRIPTION, currTime.c_str(),
            AlgorithmToStr(algorithm),
            empty ? 1 : 0,
            GetAllocationStrategyName(allocStrategy),
            FREE_ORDER_NAMES[(uint32_t)freeOrder],
            allocTotalSeconds,
            freeTotalSeconds);
    }
}

static void TestBufferDeviceAddress()
{
    wprintf(L"Test buffer device address\n");

    assert(VK_KHR_buffer_device_address_enabled);

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 0x10000;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT; // !!!

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    for(uint32_t testIndex = 0; testIndex < 2; ++testIndex)
    {
        // 1st is placed, 2nd is dedicated.
        if(testIndex == 1)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        BufferInfo bufInfo = {};
        VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &bufInfo.Buffer, &bufInfo.Allocation, nullptr);
        TEST(res == VK_SUCCESS);

        VkBufferDeviceAddressInfoEXT bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT };
        bufferDeviceAddressInfo.buffer = bufInfo.Buffer;
        TEST(g_vkGetBufferDeviceAddressKHR != nullptr);
        VkDeviceAddress addr = g_vkGetBufferDeviceAddressKHR(g_hDevice, &bufferDeviceAddressInfo);
        TEST(addr != 0);

        vmaDestroyBuffer(g_hAllocator, bufInfo.Buffer, bufInfo.Allocation);
    }
}

static void TestMemoryPriority()
{
    wprintf(L"Test memory priority\n");

    assert(VK_EXT_memory_priority_enabled);

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 0x10000;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.priority = 1.f;

    for(uint32_t testIndex = 0; testIndex < 2; ++testIndex)
    {
        // 1st is placed, 2nd is dedicated.
        if(testIndex == 1)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        BufferInfo bufInfo = {};
        VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &bufInfo.Buffer, &bufInfo.Allocation, nullptr);
        TEST(res == VK_SUCCESS);

        // There is nothing we can do to validate the priority.

        vmaDestroyBuffer(g_hAllocator, bufInfo.Buffer, bufInfo.Allocation);
    }
}

static void BenchmarkAlgorithms(FILE* file)
{
    wprintf(L"Benchmark algorithms\n");

    if(file)
    {
        fprintf(file,
            "Code,Time,"
            "Algorithm,Empty,Allocation strategy,Free order,"
            "Allocation time (s),Deallocation time (s)\n");
    }

    uint32_t freeOrderCount = 1;
    if(ConfigType >= CONFIG_TYPE::CONFIG_TYPE_LARGE)
        freeOrderCount = 3;
    else if(ConfigType >= CONFIG_TYPE::CONFIG_TYPE_SMALL)
        freeOrderCount = 2;

    const uint32_t emptyCount = ConfigType >= CONFIG_TYPE::CONFIG_TYPE_SMALL ? 2 : 1;
    const uint32_t allocStrategyCount = GetAllocationStrategyCount();

    for(uint32_t freeOrderIndex = 0; freeOrderIndex < freeOrderCount; ++freeOrderIndex)
    {
        FREE_ORDER freeOrder = FREE_ORDER::COUNT;
        switch(freeOrderIndex)
        {
        case 0: freeOrder = FREE_ORDER::BACKWARD; break;
        case 1: freeOrder = FREE_ORDER::FORWARD; break;
        case 2: freeOrder = FREE_ORDER::RANDOM; break;
        default: assert(0);
        }

        for(uint32_t emptyIndex = 0; emptyIndex < emptyCount; ++emptyIndex)
        {
            for(uint32_t algorithmIndex = 0; algorithmIndex < 3; ++algorithmIndex)
            {
                uint32_t algorithm = 0;
                switch(algorithmIndex)
                {
                case 0:
                    break;
                case 1:
                    algorithm = VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT;
                    break;
                case 2:
                    algorithm = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
                    break;
                default:
                    assert(0);
                }

                uint32_t currAllocStrategyCount = algorithm != 0 ? 1 : allocStrategyCount;
                for(uint32_t allocStrategyIndex = 0; allocStrategyIndex < currAllocStrategyCount; ++allocStrategyIndex)
                {
                    VmaAllocatorCreateFlags strategy = 0;
                    if(currAllocStrategyCount > 1)
                    {
                        switch(allocStrategyIndex)
                        {
                        case 0: strategy = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT; break;
                        case 1: strategy = VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT; break;
                        case 2: strategy = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT; break;
                        default: assert(0);
                        }
                    }

                    BenchmarkAlgorithmsCase(
                        file,
                        algorithm,
                        (emptyIndex == 0), // empty
                        strategy,
                        freeOrder); // freeOrder
                }
            }
        }
    }
}

static void TestPool_SameSize()
{
    const VkDeviceSize BUF_SIZE = 1024 * 1024;
    const size_t BUF_COUNT = 100;
    VkResult res;

    RandomNumberGenerator rand{123};

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = BUF_SIZE;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    uint32_t memoryTypeBits = UINT32_MAX;
    {
        VkBuffer dummyBuffer;
        res = vkCreateBuffer(g_hDevice, &bufferInfo, g_Allocs, &dummyBuffer);
        TEST(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(g_hDevice, dummyBuffer, &memReq);
        memoryTypeBits = memReq.memoryTypeBits;

        vkDestroyBuffer(g_hDevice, dummyBuffer, g_Allocs);
    }

    VmaAllocationCreateInfo poolAllocInfo = {};
    poolAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    uint32_t memTypeIndex;
    res = vmaFindMemoryTypeIndex(
        g_hAllocator,
        memoryTypeBits,
        &poolAllocInfo,
        &memTypeIndex);

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = memTypeIndex;
    poolCreateInfo.blockSize = BUF_SIZE * BUF_COUNT / 4;
    poolCreateInfo.minBlockCount = 1;
    poolCreateInfo.maxBlockCount = 4;
    poolCreateInfo.frameInUseCount = 0;

    VmaPool pool;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    // Test pool name
    {
        static const char* const POOL_NAME = "Pool name";
        vmaSetPoolName(g_hAllocator, pool, POOL_NAME);

        const char* fetchedPoolName = nullptr;
        vmaGetPoolName(g_hAllocator, pool, &fetchedPoolName);
        TEST(strcmp(fetchedPoolName, POOL_NAME) == 0);

        vmaSetPoolName(g_hAllocator, pool, nullptr);
    }

    vmaSetCurrentFrameIndex(g_hAllocator, 1);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.pool = pool;
    allocInfo.flags = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT |
        VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;

    struct BufItem
    {
        VkBuffer Buf;
        VmaAllocation Alloc;
    };
    std::vector<BufItem> items;

    // Fill entire pool.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Make sure that another allocation would fail.
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    // Validate that no buffer is lost. Also check that they are not mapped.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        TEST(allocInfo.deviceMemory != VK_NULL_HANDLE);
        TEST(allocInfo.pMappedData == nullptr);
    }

    // Free some percent of random items.
    {
        const size_t PERCENT_TO_FREE = 10;
        size_t itemsToFree = items.size() * PERCENT_TO_FREE / 100;
        for(size_t i = 0; i < itemsToFree; ++i)
        {
            size_t index = (size_t)rand.Generate() % items.size();
            vmaDestroyBuffer(g_hAllocator, items[index].Buf, items[index].Alloc);
            items.erase(items.begin() + index);
        }
    }

    // Randomly allocate and free items.
    {
        const size_t OPERATION_COUNT = BUF_COUNT;
        for(size_t i = 0; i < OPERATION_COUNT; ++i)
        {
            bool allocate = rand.Generate() % 2 != 0;
            if(allocate)
            {
                if(items.size() < BUF_COUNT)
                {
                    BufItem item;
                    res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
                    TEST(res == VK_SUCCESS);
                    items.push_back(item);
               }
            }
            else // Free
            {
                if(!items.empty())
                {
                    size_t index = (size_t)rand.Generate() % items.size();
                    vmaDestroyBuffer(g_hAllocator, items[index].Buf, items[index].Alloc);
                    items.erase(items.begin() + index);
                }
            }
        }
    }

    // Allocate up to maximum.
    while(items.size() < BUF_COUNT)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Validate that no buffer is lost.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        TEST(allocInfo.deviceMemory != VK_NULL_HANDLE);
    }
    
    // Next frame.
    vmaSetCurrentFrameIndex(g_hAllocator, 2);

    // Allocate another BUF_COUNT buffers.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Make sure the first BUF_COUNT is lost. Delete them.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        TEST(allocInfo.deviceMemory == VK_NULL_HANDLE);
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);
    }
    items.erase(items.begin(), items.begin() + BUF_COUNT);

    // Validate that no buffer is lost.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        TEST(allocInfo.deviceMemory != VK_NULL_HANDLE);
    }

    // Free one item.
    vmaDestroyBuffer(g_hAllocator, items.back().Buf, items.back().Alloc);
    items.pop_back();

    // Validate statistics.
    {
        VmaPoolStats poolStats = {};
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        TEST(poolStats.allocationCount == items.size());
        TEST(poolStats.size = BUF_COUNT * BUF_SIZE);
        TEST(poolStats.unusedRangeCount == 1);
        TEST(poolStats.unusedRangeSizeMax == BUF_SIZE);
        TEST(poolStats.unusedSize == BUF_SIZE);
    }

    // Free all remaining items.
    for(size_t i = items.size(); i--; )
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);
    items.clear();

    // Allocate maximum items again.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Delete every other item.
    for(size_t i = 0; i < BUF_COUNT / 2; ++i)
    {
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);
        items.erase(items.begin() + i);
    }

    // Defragment!
    {
        std::vector<VmaAllocation> allocationsToDefragment(items.size());
        for(size_t i = 0; i < items.size(); ++i)
            allocationsToDefragment[i] = items[i].Alloc;

        VmaDefragmentationStats defragmentationStats;
        res = vmaDefragment(g_hAllocator, allocationsToDefragment.data(), items.size(), nullptr, nullptr, &defragmentationStats);
        TEST(res == VK_SUCCESS);
        TEST(defragmentationStats.deviceMemoryBlocksFreed == 2);
    }

    // Free all remaining items.
    for(size_t i = items.size(); i--; )
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);
    items.clear();

    ////////////////////////////////////////////////////////////////////////////////
    // Test for vmaMakePoolAllocationsLost

    // Allocate 4 buffers on frame 10.
    vmaSetCurrentFrameIndex(g_hAllocator, 10);
    for(size_t i = 0; i < 4; ++i)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        TEST(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Touch first 2 of them on frame 11.
    vmaSetCurrentFrameIndex(g_hAllocator, 11);
    for(size_t i = 0; i < 2; ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
    }

    // vmaMakePoolAllocationsLost. Only remaining 2 should be lost.
    size_t lostCount = 0xDEADC0DE;
    vmaMakePoolAllocationsLost(g_hAllocator, pool, &lostCount);
    TEST(lostCount == 2);

    // Make another call. Now 0 should be lost.
    vmaMakePoolAllocationsLost(g_hAllocator, pool, &lostCount);
    TEST(lostCount == 0);

    // Make another call, with null count. Should not crash.
    vmaMakePoolAllocationsLost(g_hAllocator, pool, nullptr);

    // END: Free all remaining items.
    for(size_t i = items.size(); i--; )
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);

    items.clear();

    ////////////////////////////////////////////////////////////////////////////////
    // Test for allocation too large for pool

    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.pool = pool;

        VkMemoryRequirements memReq;
        memReq.memoryTypeBits = UINT32_MAX;
        memReq.alignment = 1;
        memReq.size = poolCreateInfo.blockSize + 4;
        
        VmaAllocation alloc = nullptr;
        res = vmaAllocateMemory(g_hAllocator, &memReq, &allocCreateInfo, &alloc, nullptr);
        TEST(res == VK_ERROR_OUT_OF_DEVICE_MEMORY && alloc == nullptr);
    }

    vmaDestroyPool(g_hAllocator, pool);
}

static bool ValidatePattern(const void* pMemory, size_t size, uint8_t pattern)
{
    const uint8_t* pBytes = (const uint8_t*)pMemory;
    for(size_t i = 0; i < size; ++i)
    {
        if(pBytes[i] != pattern)
        {
            return false;
        }
    }
    return true;
}

static void TestAllocationsInitialization()
{
    VkResult res;

    const size_t BUF_SIZE = 1024;

    // Create pool.

    VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufInfo.size = BUF_SIZE;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo dummyBufAllocCreateInfo = {};
    dummyBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.blockSize = BUF_SIZE * 10;
    poolCreateInfo.minBlockCount = 1; // To keep memory alive while pool exists.
    poolCreateInfo.maxBlockCount = 1;
    res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufInfo, &dummyBufAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    VmaAllocationCreateInfo bufAllocCreateInfo = {};
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &bufAllocCreateInfo.pool);
    TEST(res == VK_SUCCESS);

    // Create one persistently mapped buffer to keep memory of this block mapped,
    // so that pointer to mapped data will remain (more or less...) valid even
    // after destruction of other allocations.
    
    bufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VkBuffer firstBuf;
    VmaAllocation firstAlloc;
    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &bufAllocCreateInfo, &firstBuf, &firstAlloc, nullptr);
    TEST(res == VK_SUCCESS);

    // Test buffers.

    for(uint32_t i = 0; i < 2; ++i)
    {
        const bool persistentlyMapped = i == 0;
        bufAllocCreateInfo.flags = persistentlyMapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
        VkBuffer buf;
        VmaAllocation alloc;
        VmaAllocationInfo allocInfo;
        res = vmaCreateBuffer(g_hAllocator, &bufInfo, &bufAllocCreateInfo, &buf, &alloc, &allocInfo);
        TEST(res == VK_SUCCESS);

        void* pMappedData;
        if(!persistentlyMapped)
        {
            res = vmaMapMemory(g_hAllocator, alloc, &pMappedData);
            TEST(res == VK_SUCCESS);
        }
        else
        {
            pMappedData = allocInfo.pMappedData;
        }

        // Validate initialized content
        bool valid = ValidatePattern(pMappedData, BUF_SIZE, 0xDC);
        TEST(valid);

        if(!persistentlyMapped)
        {
            vmaUnmapMemory(g_hAllocator, alloc);
        }

        vmaDestroyBuffer(g_hAllocator, buf, alloc);

        // Validate freed content
        valid = ValidatePattern(pMappedData, BUF_SIZE, 0xEF);
        TEST(valid);
    }

    vmaDestroyBuffer(g_hAllocator, firstBuf, firstAlloc);
    vmaDestroyPool(g_hAllocator, bufAllocCreateInfo.pool);
}

static void TestPool_Benchmark(
    PoolTestResult& outResult,
    const PoolTestConfig& config)
{
    TEST(config.ThreadCount > 0);

    RandomNumberGenerator mainRand{config.RandSeed};

    uint32_t allocationSizeProbabilitySum = std::accumulate(
        config.AllocationSizes.begin(),
        config.AllocationSizes.end(),
        0u,
        [](uint32_t sum, const AllocationSize& allocSize) {
            return sum + allocSize.Probability;
        });

    VkBufferCreateInfo bufferTemplateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferTemplateInfo.size = 256; // Whatever.
    bufferTemplateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkImageCreateInfo imageTemplateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageTemplateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageTemplateInfo.extent.width = 256; // Whatever.
    imageTemplateInfo.extent.height = 256; // Whatever.
    imageTemplateInfo.extent.depth = 1;
    imageTemplateInfo.mipLevels = 1;
    imageTemplateInfo.arrayLayers = 1;
    imageTemplateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageTemplateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // LINEAR if CPU memory.
    imageTemplateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageTemplateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // TRANSFER_SRC if CPU memory.
    imageTemplateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    uint32_t bufferMemoryTypeBits = UINT32_MAX;
    {
        VkBuffer dummyBuffer;
        VkResult res = vkCreateBuffer(g_hDevice, &bufferTemplateInfo, g_Allocs, &dummyBuffer);
        TEST(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(g_hDevice, dummyBuffer, &memReq);
        bufferMemoryTypeBits = memReq.memoryTypeBits;

        vkDestroyBuffer(g_hDevice, dummyBuffer, g_Allocs);
    }

    uint32_t imageMemoryTypeBits = UINT32_MAX;
    {
        VkImage dummyImage;
        VkResult res = vkCreateImage(g_hDevice, &imageTemplateInfo, g_Allocs, &dummyImage);
        TEST(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(g_hDevice, dummyImage, &memReq);
        imageMemoryTypeBits = memReq.memoryTypeBits;

        vkDestroyImage(g_hDevice, dummyImage, g_Allocs);
    }

    uint32_t memoryTypeBits = 0;
    if(config.UsesBuffers() && config.UsesImages())
    {
        memoryTypeBits = bufferMemoryTypeBits & imageMemoryTypeBits;
        if(memoryTypeBits == 0)
        {
            PrintWarning(L"Cannot test buffers + images in the same memory pool on this GPU.");
            return;
        }
    }
    else if(config.UsesBuffers())
        memoryTypeBits = bufferMemoryTypeBits;
    else if(config.UsesImages())
        memoryTypeBits = imageMemoryTypeBits;
    else
        TEST(0);

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.minBlockCount = 1;
    poolCreateInfo.maxBlockCount = 1;
    poolCreateInfo.blockSize = config.PoolSize;
    poolCreateInfo.frameInUseCount = 1;

    const VkPhysicalDeviceMemoryProperties* memProps = nullptr;
    vmaGetMemoryProperties(g_hAllocator, &memProps);

    VmaPool pool = VK_NULL_HANDLE;
    VkResult res;
    // Loop over memory types because we sometimes allocate a big block here,
    // while the most eligible DEVICE_LOCAL heap may be only 256 MB on some GPUs.
    while(memoryTypeBits)
    {
        VmaAllocationCreateInfo dummyAllocCreateInfo = {};
        dummyAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaFindMemoryTypeIndex(g_hAllocator, memoryTypeBits, &dummyAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);

        const uint32_t heapIndex = memProps->memoryTypes[poolCreateInfo.memoryTypeIndex].heapIndex;
        // Protection against validation layer error when trying to allocate a block larger than entire heap size,
        // which may be only 256 MB on some platforms.
        if(poolCreateInfo.blockSize * poolCreateInfo.minBlockCount < memProps->memoryHeaps[heapIndex].size)
        {
            res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
            if(res == VK_SUCCESS)
                break;
        }
        memoryTypeBits &= ~(1u << poolCreateInfo.memoryTypeIndex);
    }
    TEST(pool);

    // Start time measurement - after creating pool and initializing data structures.
    time_point timeBeg = std::chrono::high_resolution_clock::now();

    ////////////////////////////////////////////////////////////////////////////////
    // ThreadProc
    auto ThreadProc = [&config, allocationSizeProbabilitySum, pool](
        PoolTestThreadResult* outThreadResult,
        uint32_t randSeed,
        HANDLE frameStartEvent,
        HANDLE frameEndEvent) -> void
    {
        RandomNumberGenerator threadRand{randSeed};
        VkResult res = VK_SUCCESS;

        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = 256; // Whatever.
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = 256; // Whatever.
        imageInfo.extent.height = 256; // Whatever.
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // LINEAR if CPU memory.
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // TRANSFER_SRC if CPU memory.
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        outThreadResult->AllocationTimeMin = duration::max();
        outThreadResult->AllocationTimeSum = duration::zero();
        outThreadResult->AllocationTimeMax = duration::min();
        outThreadResult->DeallocationTimeMin = duration::max();
        outThreadResult->DeallocationTimeSum = duration::zero();
        outThreadResult->DeallocationTimeMax = duration::min();
        outThreadResult->AllocationCount = 0;
        outThreadResult->DeallocationCount = 0;
        outThreadResult->LostAllocationCount = 0;
        outThreadResult->LostAllocationTotalSize = 0;
        outThreadResult->FailedAllocationCount = 0;
        outThreadResult->FailedAllocationTotalSize = 0;

        struct Item
        {
            VkDeviceSize BufferSize = 0;
            VkExtent2D ImageSize = { 0, 0 };
            VkBuffer Buf = VK_NULL_HANDLE;
            VkImage Image = VK_NULL_HANDLE;
            VmaAllocation Alloc = VK_NULL_HANDLE;
            
            Item() { }
            Item(Item&& src) :
                BufferSize(src.BufferSize), ImageSize(src.ImageSize), Buf(src.Buf), Image(src.Image), Alloc(src.Alloc)
            {
                src.BufferSize = 0;
                src.ImageSize = {0, 0};
                src.Buf = VK_NULL_HANDLE;
                src.Image = VK_NULL_HANDLE;
                src.Alloc = VK_NULL_HANDLE;
            }
            Item(const Item& src) = delete;
            ~Item()
            {
                DestroyResources();
            }
            Item& operator=(Item&& src)
            {
                if(&src != this)
                {
                    DestroyResources();
                    BufferSize = src.BufferSize; ImageSize = src.ImageSize;
                    Buf = src.Buf; Image = src.Image; Alloc = src.Alloc;
                    src.BufferSize = 0;
                    src.ImageSize = {0, 0};
                    src.Buf = VK_NULL_HANDLE;
                    src.Image = VK_NULL_HANDLE;
                    src.Alloc = VK_NULL_HANDLE;
                }
                return *this;
            }
            Item& operator=(const Item& src) = delete;
            void DestroyResources()
            {
                if(Buf)
                {
                    assert(Image == VK_NULL_HANDLE);
                    vmaDestroyBuffer(g_hAllocator, Buf, Alloc);
                    Buf = VK_NULL_HANDLE;
                }
                else
                {
                    vmaDestroyImage(g_hAllocator, Image, Alloc);
                    Image = VK_NULL_HANDLE;
                }
                Alloc = VK_NULL_HANDLE;
            }
            VkDeviceSize CalcSizeBytes() const
            {
                return BufferSize +
                    4ull * ImageSize.width * ImageSize.height;
            }
        };
        std::vector<Item> unusedItems, usedItems;

        const size_t threadTotalItemCount = config.TotalItemCount / config.ThreadCount;

        // Create all items - all unused, not yet allocated.
        for(size_t i = 0; i < threadTotalItemCount; ++i)
        {
            Item item = {};

            uint32_t allocSizeIndex = 0;
            uint32_t r = threadRand.Generate() % allocationSizeProbabilitySum;
            while(r >= config.AllocationSizes[allocSizeIndex].Probability)
                r -= config.AllocationSizes[allocSizeIndex++].Probability;

            const AllocationSize& allocSize = config.AllocationSizes[allocSizeIndex];
            if(allocSize.BufferSizeMax > 0)
            {
                TEST(allocSize.BufferSizeMin > 0);
                TEST(allocSize.ImageSizeMin == 0 && allocSize.ImageSizeMax == 0);
                if(allocSize.BufferSizeMax == allocSize.BufferSizeMin)
                    item.BufferSize = allocSize.BufferSizeMin;
                else
                {
                    item.BufferSize = allocSize.BufferSizeMin + threadRand.Generate() % (allocSize.BufferSizeMax - allocSize.BufferSizeMin);
                    item.BufferSize = item.BufferSize / 16 * 16;
                }
            }
            else
            {
                TEST(allocSize.ImageSizeMin > 0 && allocSize.ImageSizeMax > 0);
                if(allocSize.ImageSizeMax == allocSize.ImageSizeMin)
                    item.ImageSize.width = item.ImageSize.height = allocSize.ImageSizeMax;
                else
                {
                    item.ImageSize.width  = allocSize.ImageSizeMin + threadRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
                    item.ImageSize.height = allocSize.ImageSizeMin + threadRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
                }
            }

            unusedItems.push_back(std::move(item));
        }

        auto Allocate = [&](Item& item) -> VkResult
        {
            assert(item.Buf == VK_NULL_HANDLE && item.Image == VK_NULL_HANDLE && item.Alloc == VK_NULL_HANDLE);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.pool = pool;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT |
                VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;

            if(item.BufferSize)
            {
                bufferInfo.size = item.BufferSize;
                VkResult res = VK_SUCCESS;
                {
                    PoolAllocationTimeRegisterObj timeRegisterObj(*outThreadResult);
                    res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocCreateInfo, &item.Buf, &item.Alloc, nullptr);
                }
                if(res == VK_SUCCESS)
                    SetDebugUtilsObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)item.Buf, "TestPool_Benchmark_Buffer");
                return res;
            }
            else
            {
                TEST(item.ImageSize.width && item.ImageSize.height);

                imageInfo.extent.width = item.ImageSize.width;
                imageInfo.extent.height = item.ImageSize.height;
                VkResult res = VK_SUCCESS;
                {
                    PoolAllocationTimeRegisterObj timeRegisterObj(*outThreadResult);
                    res = vmaCreateImage(g_hAllocator, &imageInfo, &allocCreateInfo, &item.Image, &item.Alloc, nullptr);
                }
                if(res == VK_SUCCESS)
                    SetDebugUtilsObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)item.Image, "TestPool_Benchmark_Image");
                return res;
            }
        };

        ////////////////////////////////////////////////////////////////////////////////
        // Frames
        for(uint32_t frameIndex = 0; frameIndex < config.FrameCount; ++frameIndex)
        {
            WaitForSingleObject(frameStartEvent, INFINITE);

            // Always make some percent of used bufs unused, to choose different used ones.
            const size_t bufsToMakeUnused = usedItems.size() * config.ItemsToMakeUnusedPercent / 100;
            for(size_t i = 0; i < bufsToMakeUnused; ++i)
            {
                size_t index = threadRand.Generate() % usedItems.size();
                auto it = usedItems.begin() + index;
                Item item = std::move(*it);
                usedItems.erase(it);
                unusedItems.push_back(std::move(item));
            }

            // Determine which bufs we want to use in this frame.
            const size_t usedBufCount = (threadRand.Generate() % (config.UsedItemCountMax - config.UsedItemCountMin) + config.UsedItemCountMin)
                / config.ThreadCount;
            TEST(usedBufCount < usedItems.size() + unusedItems.size());
            // Move some used to unused.
            while(usedBufCount < usedItems.size())
            {
                size_t index = threadRand.Generate() % usedItems.size();
                auto it = usedItems.begin() + index;
                Item item = std::move(*it);
                usedItems.erase(it);
                unusedItems.push_back(std::move(item));
            }
            // Move some unused to used.
            while(usedBufCount > usedItems.size())
            {
                size_t index = threadRand.Generate() % unusedItems.size();
                auto it = unusedItems.begin() + index;
                Item item = std::move(*it);
                unusedItems.erase(it);
                usedItems.push_back(std::move(item));
            }

            uint32_t touchExistingCount = 0;
            uint32_t touchLostCount = 0;
            uint32_t createSucceededCount = 0;
            uint32_t createFailedCount = 0;

            // Touch all used bufs. If not created or lost, allocate.
            for(size_t i = 0; i < usedItems.size(); ++i)
            {
                Item& item = usedItems[i];
                // Not yet created.
                if(item.Alloc == VK_NULL_HANDLE)
                {
                    res = Allocate(item);
                    ++outThreadResult->AllocationCount;
                    if(res != VK_SUCCESS)
                    {
                        assert(item.Alloc == VK_NULL_HANDLE && item.Buf == VK_NULL_HANDLE && item.Image == VK_NULL_HANDLE);
                        ++outThreadResult->FailedAllocationCount;
                        outThreadResult->FailedAllocationTotalSize += item.CalcSizeBytes();
                        ++createFailedCount;
                    }
                    else
                        ++createSucceededCount;
                }
                else
                {
                    // Touch.
                    VmaAllocationInfo allocInfo;
                    vmaGetAllocationInfo(g_hAllocator, item.Alloc, &allocInfo);
                    // Lost.
                    if(allocInfo.deviceMemory == VK_NULL_HANDLE)
                    {
                        ++touchLostCount;

                        // Destroy.
                        {
                            PoolDeallocationTimeRegisterObj timeRegisterObj(*outThreadResult);
                            item.DestroyResources();
                            ++outThreadResult->DeallocationCount;
                        }

                        ++outThreadResult->LostAllocationCount;
                        outThreadResult->LostAllocationTotalSize += item.CalcSizeBytes();

                        // Recreate.
                        res = Allocate(item);
                        ++outThreadResult->AllocationCount;
                        // Creation failed.
                        if(res != VK_SUCCESS)
                        {
                            TEST(item.Alloc == VK_NULL_HANDLE && item.Buf == VK_NULL_HANDLE && item.Image == VK_NULL_HANDLE);
                            ++outThreadResult->FailedAllocationCount;
                            outThreadResult->FailedAllocationTotalSize += item.CalcSizeBytes();
                            ++createFailedCount;
                        }
                        else
                            ++createSucceededCount;
                    }
                    else
                        ++touchExistingCount;
                }
            }
 
            /*
            printf("Thread %u frame %u: Touch existing %u lost %u, create succeeded %u failed %u\n",
                randSeed, frameIndex,
                touchExistingCount, touchLostCount,
                createSucceededCount, createFailedCount);
            */

            SetEvent(frameEndEvent);
        }

        // Free all remaining items.
        for(size_t i = usedItems.size(); i--; )
        {
            PoolDeallocationTimeRegisterObj timeRegisterObj(*outThreadResult);
            usedItems[i].DestroyResources();
            ++outThreadResult->DeallocationCount;
        }
        for(size_t i = unusedItems.size(); i--; )
        {
            PoolDeallocationTimeRegisterObj timeRegisterOb(*outThreadResult);
            unusedItems[i].DestroyResources();
            ++outThreadResult->DeallocationCount;
        }
    };

    // Launch threads.
    uint32_t threadRandSeed = mainRand.Generate();
    std::vector<HANDLE> frameStartEvents{config.ThreadCount};
    std::vector<HANDLE> frameEndEvents{config.ThreadCount};
    std::vector<std::thread> bkgThreads;
    std::vector<PoolTestThreadResult> threadResults{config.ThreadCount};
    for(uint32_t threadIndex = 0; threadIndex < config.ThreadCount; ++threadIndex)
    {
        frameStartEvents[threadIndex] = CreateEvent(NULL, FALSE, FALSE, NULL);
        frameEndEvents[threadIndex] = CreateEvent(NULL, FALSE, FALSE, NULL);
        bkgThreads.emplace_back(std::bind(
            ThreadProc,
            &threadResults[threadIndex],
            threadRandSeed + threadIndex,
            frameStartEvents[threadIndex],
            frameEndEvents[threadIndex]));
    }

    // Execute frames.
    TEST(config.ThreadCount <= MAXIMUM_WAIT_OBJECTS);
    for(uint32_t frameIndex = 0; frameIndex < config.FrameCount; ++frameIndex)
    {
        vmaSetCurrentFrameIndex(g_hAllocator, frameIndex);
        for(size_t threadIndex = 0; threadIndex < config.ThreadCount; ++threadIndex)
            SetEvent(frameStartEvents[threadIndex]);
        WaitForMultipleObjects(config.ThreadCount, &frameEndEvents[0], TRUE, INFINITE);
    }

    // Wait for threads finished
    for(size_t i = 0; i < bkgThreads.size(); ++i)
    {
        bkgThreads[i].join();
        CloseHandle(frameEndEvents[i]);
        CloseHandle(frameStartEvents[i]);
    }
    bkgThreads.clear();

    // Finish time measurement - before destroying pool.
    outResult.TotalTime = std::chrono::high_resolution_clock::now() - timeBeg;

    vmaDestroyPool(g_hAllocator, pool);

    outResult.AllocationTimeMin = duration::max();
    outResult.AllocationTimeAvg = duration::zero();
    outResult.AllocationTimeMax = duration::min();
    outResult.DeallocationTimeMin = duration::max();
    outResult.DeallocationTimeAvg = duration::zero();
    outResult.DeallocationTimeMax = duration::min();
    outResult.LostAllocationCount = 0;
    outResult.LostAllocationTotalSize = 0;
    outResult.FailedAllocationCount = 0;
    outResult.FailedAllocationTotalSize = 0;
    size_t allocationCount = 0;
    size_t deallocationCount = 0;
    for(size_t threadIndex = 0; threadIndex < config.ThreadCount; ++threadIndex)
    {
        const PoolTestThreadResult& threadResult = threadResults[threadIndex];
        outResult.AllocationTimeMin = std::min(outResult.AllocationTimeMin, threadResult.AllocationTimeMin);
        outResult.AllocationTimeMax = std::max(outResult.AllocationTimeMax, threadResult.AllocationTimeMax);
        outResult.AllocationTimeAvg += threadResult.AllocationTimeSum;
        outResult.DeallocationTimeMin = std::min(outResult.DeallocationTimeMin, threadResult.DeallocationTimeMin);
        outResult.DeallocationTimeMax = std::max(outResult.DeallocationTimeMax, threadResult.DeallocationTimeMax);
        outResult.DeallocationTimeAvg += threadResult.DeallocationTimeSum;
        allocationCount += threadResult.AllocationCount;
        deallocationCount += threadResult.DeallocationCount;
        outResult.FailedAllocationCount += threadResult.FailedAllocationCount;
        outResult.FailedAllocationTotalSize += threadResult.FailedAllocationTotalSize;
        outResult.LostAllocationCount += threadResult.LostAllocationCount;
        outResult.LostAllocationTotalSize += threadResult.LostAllocationTotalSize;
    }
    if(allocationCount)
        outResult.AllocationTimeAvg /= allocationCount;
    if(deallocationCount)
        outResult.DeallocationTimeAvg /= deallocationCount;
}

static inline bool MemoryRegionsOverlap(char* ptr1, size_t size1, char* ptr2, size_t size2)
{
    if(ptr1 < ptr2)
        return ptr1 + size1 > ptr2;
    else if(ptr2 < ptr1)
        return ptr2 + size2 > ptr1;
    else
        return true;
}

static void TestMemoryUsage()
{
    wprintf(L"Testing memory usage:\n");

    static const VmaMemoryUsage lastUsage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
    for(uint32_t usage = 0; usage <= lastUsage; ++usage)
    {
        switch(usage)
        {
        case VMA_MEMORY_USAGE_UNKNOWN: printf("  VMA_MEMORY_USAGE_UNKNOWN:\n"); break;
        case VMA_MEMORY_USAGE_GPU_ONLY: printf("  VMA_MEMORY_USAGE_GPU_ONLY:\n"); break;
        case VMA_MEMORY_USAGE_CPU_ONLY: printf("  VMA_MEMORY_USAGE_CPU_ONLY:\n"); break;
        case VMA_MEMORY_USAGE_CPU_TO_GPU: printf("  VMA_MEMORY_USAGE_CPU_TO_GPU:\n"); break;
        case VMA_MEMORY_USAGE_GPU_TO_CPU: printf("  VMA_MEMORY_USAGE_GPU_TO_CPU:\n"); break;
        case VMA_MEMORY_USAGE_CPU_COPY: printf("  VMA_MEMORY_USAGE_CPU_COPY:\n"); break;
        case VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED: printf("  VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED:\n"); break;
        default: assert(0);
        }

        auto printResult = [](const char* testName, VkResult res, uint32_t memoryTypeBits, uint32_t memoryTypeIndex)
        {
            if(res == VK_SUCCESS)
                printf("    %s: memoryTypeBits=0x%X, memoryTypeIndex=%u\n", testName, memoryTypeBits, memoryTypeIndex);
            else
                printf("    %s: memoryTypeBits=0x%X, FAILED with res=%d\n", testName, memoryTypeBits, (int32_t)res);
        };
        
        // 1: Buffer for copy
        {
            VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufCreateInfo.size = 65536;
            bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VkBuffer buf = VK_NULL_HANDLE;
            VkResult res = vkCreateBuffer(g_hDevice, &bufCreateInfo, g_Allocs, &buf);
            TEST(res == VK_SUCCESS && buf != VK_NULL_HANDLE);

            VkMemoryRequirements memReq = {};
            vkGetBufferMemoryRequirements(g_hDevice, buf, &memReq);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = (VmaMemoryUsage)usage;
            VmaAllocation alloc = VK_NULL_HANDLE;
            VmaAllocationInfo allocInfo = {};
            res = vmaAllocateMemoryForBuffer(g_hAllocator, buf, &allocCreateInfo, &alloc, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST((memReq.memoryTypeBits & (1u << allocInfo.memoryType)) != 0);
                res = vkBindBufferMemory(g_hDevice, buf, allocInfo.deviceMemory, allocInfo.offset);
                TEST(res == VK_SUCCESS);
            }
            printResult("Buffer TRANSFER_DST + TRANSFER_SRC", res, memReq.memoryTypeBits, allocInfo.memoryType);
            vmaDestroyBuffer(g_hAllocator, buf, alloc);
        }

        // 2: Vertex buffer
        {
            VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufCreateInfo.size = 65536;
            bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

            VkBuffer buf = VK_NULL_HANDLE;
            VkResult res = vkCreateBuffer(g_hDevice, &bufCreateInfo, g_Allocs, &buf);
            TEST(res == VK_SUCCESS && buf != VK_NULL_HANDLE);

            VkMemoryRequirements memReq = {};
            vkGetBufferMemoryRequirements(g_hDevice, buf, &memReq);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = (VmaMemoryUsage)usage;
            VmaAllocation alloc = VK_NULL_HANDLE;
            VmaAllocationInfo allocInfo = {};
            res = vmaAllocateMemoryForBuffer(g_hAllocator, buf, &allocCreateInfo, &alloc, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST((memReq.memoryTypeBits & (1u << allocInfo.memoryType)) != 0);
                res = vkBindBufferMemory(g_hDevice, buf, allocInfo.deviceMemory, allocInfo.offset);
                TEST(res == VK_SUCCESS);
            }
            printResult("Buffer TRANSFER_DST + VERTEX_BUFFER", res, memReq.memoryTypeBits, allocInfo.memoryType);
            vmaDestroyBuffer(g_hAllocator, buf, alloc);
        }

        // 3: Image for copy, OPTIMAL
        {
            VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imgCreateInfo.extent.width = 256;
            imgCreateInfo.extent.height = 256;
            imgCreateInfo.extent.depth = 1;
            imgCreateInfo.mipLevels = 1;
            imgCreateInfo.arrayLayers = 1;
            imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VkImage img = VK_NULL_HANDLE;
            VkResult res = vkCreateImage(g_hDevice, &imgCreateInfo, g_Allocs, &img);
            TEST(res == VK_SUCCESS && img != VK_NULL_HANDLE);

            VkMemoryRequirements memReq = {};
            vkGetImageMemoryRequirements(g_hDevice, img, &memReq);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = (VmaMemoryUsage)usage;
            VmaAllocation alloc = VK_NULL_HANDLE;
            VmaAllocationInfo allocInfo = {};
            res = vmaAllocateMemoryForImage(g_hAllocator, img, &allocCreateInfo, &alloc, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST((memReq.memoryTypeBits & (1u << allocInfo.memoryType)) != 0);
                res = vkBindImageMemory(g_hDevice, img, allocInfo.deviceMemory, allocInfo.offset);
                TEST(res == VK_SUCCESS);
            }
            printResult("Image OPTIMAL TRANSFER_DST + TRANSFER_SRC", res, memReq.memoryTypeBits, allocInfo.memoryType);

            vmaDestroyImage(g_hAllocator, img, alloc);
        }

        // 4: Image SAMPLED, OPTIMAL
        {
            VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imgCreateInfo.extent.width = 256;
            imgCreateInfo.extent.height = 256;
            imgCreateInfo.extent.depth = 1;
            imgCreateInfo.mipLevels = 1;
            imgCreateInfo.arrayLayers = 1;
            imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VkImage img = VK_NULL_HANDLE;
            VkResult res = vkCreateImage(g_hDevice, &imgCreateInfo, g_Allocs, &img);
            TEST(res == VK_SUCCESS && img != VK_NULL_HANDLE);

            VkMemoryRequirements memReq = {};
            vkGetImageMemoryRequirements(g_hDevice, img, &memReq);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = (VmaMemoryUsage)usage;
            VmaAllocation alloc = VK_NULL_HANDLE;
            VmaAllocationInfo allocInfo = {};
            res = vmaAllocateMemoryForImage(g_hAllocator, img, &allocCreateInfo, &alloc, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST((memReq.memoryTypeBits & (1u << allocInfo.memoryType)) != 0);
                res = vkBindImageMemory(g_hDevice, img, allocInfo.deviceMemory, allocInfo.offset);
                TEST(res == VK_SUCCESS);
            }
            printResult("Image OPTIMAL TRANSFER_DST + SAMPLED", res, memReq.memoryTypeBits, allocInfo.memoryType);
            vmaDestroyImage(g_hAllocator, img, alloc);
        }

        // 5: Image COLOR_ATTACHMENT, OPTIMAL
        {
            VkImageCreateInfo imgCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imgCreateInfo.extent.width = 256;
            imgCreateInfo.extent.height = 256;
            imgCreateInfo.extent.depth = 1;
            imgCreateInfo.mipLevels = 1;
            imgCreateInfo.arrayLayers = 1;
            imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imgCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            VkImage img = VK_NULL_HANDLE;
            VkResult res = vkCreateImage(g_hDevice, &imgCreateInfo, g_Allocs, &img);
            TEST(res == VK_SUCCESS && img != VK_NULL_HANDLE);

            VkMemoryRequirements memReq = {};
            vkGetImageMemoryRequirements(g_hDevice, img, &memReq);

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = (VmaMemoryUsage)usage;
            VmaAllocation alloc = VK_NULL_HANDLE;
            VmaAllocationInfo allocInfo = {};
            res = vmaAllocateMemoryForImage(g_hAllocator, img, &allocCreateInfo, &alloc, &allocInfo);
            if(res == VK_SUCCESS)
            {
                TEST((memReq.memoryTypeBits & (1u << allocInfo.memoryType)) != 0);
                res = vkBindImageMemory(g_hDevice, img, allocInfo.deviceMemory, allocInfo.offset);
                TEST(res == VK_SUCCESS);
            }
            printResult("Image OPTIMAL SAMPLED + COLOR_ATTACHMENT", res, memReq.memoryTypeBits, allocInfo.memoryType);
            vmaDestroyImage(g_hAllocator, img, alloc);
        }
    }
}

static uint32_t FindDeviceCoherentMemoryTypeBits()
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(g_hPhysicalDevice, &memProps);

    uint32_t memTypeBits = 0;
    for(uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
    {
        if(memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
            memTypeBits |= 1u << i;
    }
    return memTypeBits;
}

static void TestDeviceCoherentMemory()
{
    if(!VK_AMD_device_coherent_memory_enabled)
        return;

    uint32_t deviceCoherentMemoryTypeBits = FindDeviceCoherentMemoryTypeBits();
    // Extension is enabled, feature is enabled, and the device still doesn't support any such memory type?
    // OK then, so it's just fake!
    if(deviceCoherentMemoryTypeBits == 0)
        return;

    wprintf(L"Testing device coherent memory...\n");

    // 1. Try to allocate buffer from a memory type that is DEVICE_COHERENT.

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 0x10000;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;

    AllocInfo alloc = {};
    VmaAllocationInfo allocInfo = {};
    VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &alloc.m_Buffer, &alloc.m_Allocation, &allocInfo);

    // Make sure it succeeded and was really created in such memory type.
    TEST(res == VK_SUCCESS);
    TEST((1u << allocInfo.memoryType) & deviceCoherentMemoryTypeBits);

    alloc.Destroy();

    // 2. Try to create a pool in such memory type.
    {
        VmaPoolCreateInfo poolCreateInfo = {};

        res = vmaFindMemoryTypeIndex(g_hAllocator, UINT32_MAX, &allocCreateInfo, &poolCreateInfo.memoryTypeIndex);
        TEST(res == VK_SUCCESS);
        TEST((1u << poolCreateInfo.memoryTypeIndex) & deviceCoherentMemoryTypeBits);

        VmaPool pool = VK_NULL_HANDLE;
        res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
        TEST(res == VK_SUCCESS);

        vmaDestroyPool(g_hAllocator, pool);
    }

    // 3. Try the same with a local allocator created without VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT.

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    SetAllocatorCreateInfo(allocatorCreateInfo);
    allocatorCreateInfo.flags &= ~VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
    
    VmaAllocator localAllocator = VK_NULL_HANDLE;
    res = vmaCreateAllocator(&allocatorCreateInfo, &localAllocator);
    TEST(res == VK_SUCCESS && localAllocator);

    res = vmaCreateBuffer(localAllocator, &bufCreateInfo, &allocCreateInfo, &alloc.m_Buffer, &alloc.m_Allocation, &allocInfo);

    // Make sure it failed.
    TEST(res != VK_SUCCESS && !alloc.m_Buffer && !alloc.m_Allocation);

    // 4. Try to find memory type.
    {
        uint32_t memTypeIndex = UINT_MAX;
        res = vmaFindMemoryTypeIndex(localAllocator, UINT32_MAX, &allocCreateInfo, &memTypeIndex);
        TEST(res != VK_SUCCESS);
    }

    vmaDestroyAllocator(localAllocator);
}

static void TestBudget()
{
    wprintf(L"Testing budget...\n");

    static const VkDeviceSize BUF_SIZE = 10ull * 1024 * 1024;
    static const uint32_t BUF_COUNT = 4;

    const VkPhysicalDeviceMemoryProperties* memProps = {};
    vmaGetMemoryProperties(g_hAllocator, &memProps);

    for(uint32_t testIndex = 0; testIndex < 2; ++testIndex)
    {
        vmaSetCurrentFrameIndex(g_hAllocator, ++g_FrameIndex);

        VmaBudget budgetBeg[VK_MAX_MEMORY_HEAPS] = {};
        vmaGetBudget(g_hAllocator, budgetBeg);

        for(uint32_t i = 0; i < memProps->memoryHeapCount; ++i)
        {
            TEST(budgetBeg[i].budget > 0);
            TEST(budgetBeg[i].budget <= memProps->memoryHeaps[i].size);
            TEST(budgetBeg[i].allocationBytes <= budgetBeg[i].blockBytes);
        }

        VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufInfo.size = BUF_SIZE;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if(testIndex == 0)
        {
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        // CREATE BUFFERS
        uint32_t heapIndex = 0;
        BufferInfo bufInfos[BUF_COUNT] = {};
        for(uint32_t bufIndex = 0; bufIndex < BUF_COUNT; ++bufIndex)
        {
            VmaAllocationInfo allocInfo;
            VkResult res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo,
                &bufInfos[bufIndex].Buffer, &bufInfos[bufIndex].Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            if(bufIndex == 0)
            {
                heapIndex = MemoryTypeToHeap(allocInfo.memoryType);
            }
            else
            {
                // All buffers need to fall into the same heap.
                TEST(MemoryTypeToHeap(allocInfo.memoryType) == heapIndex);
            }
        }

        VmaBudget budgetWithBufs[VK_MAX_MEMORY_HEAPS] = {};
        vmaGetBudget(g_hAllocator, budgetWithBufs);

        // DESTROY BUFFERS
        for(size_t bufIndex = BUF_COUNT; bufIndex--; )
        {
            vmaDestroyBuffer(g_hAllocator, bufInfos[bufIndex].Buffer, bufInfos[bufIndex].Allocation);
        }

        VmaBudget budgetEnd[VK_MAX_MEMORY_HEAPS] = {};
        vmaGetBudget(g_hAllocator, budgetEnd);

        // CHECK
        for(uint32_t i = 0; i < memProps->memoryHeapCount; ++i)
        {
            TEST(budgetEnd[i].allocationBytes <= budgetEnd[i].blockBytes);
            if(i == heapIndex)
            {
                TEST(budgetEnd[i].allocationBytes == budgetBeg[i].allocationBytes);
                TEST(budgetWithBufs[i].allocationBytes == budgetBeg[i].allocationBytes + BUF_SIZE * BUF_COUNT);
                TEST(budgetWithBufs[i].blockBytes >= budgetEnd[i].blockBytes);
            }
            else
            {
                TEST(budgetEnd[i].allocationBytes == budgetEnd[i].allocationBytes &&
                    budgetEnd[i].allocationBytes == budgetWithBufs[i].allocationBytes);
                TEST(budgetEnd[i].blockBytes == budgetEnd[i].blockBytes &&
                    budgetEnd[i].blockBytes == budgetWithBufs[i].blockBytes);
            }
        }
    }
}

static void TestAliasing()
{
    wprintf(L"Testing aliasing...\n");

    /*
    This is just a simple test, more like a code sample to demonstrate it's possible.
    */

    // A 512x512 texture to be sampled.
    VkImageCreateInfo img1CreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    img1CreateInfo.imageType = VK_IMAGE_TYPE_2D;
    img1CreateInfo.extent.width = 512;
    img1CreateInfo.extent.height = 512;
    img1CreateInfo.extent.depth = 1;
    img1CreateInfo.mipLevels = 10;
    img1CreateInfo.arrayLayers = 1;
    img1CreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    img1CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    img1CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img1CreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    img1CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    // A full screen texture to be used as color attachment.
    VkImageCreateInfo img2CreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    img2CreateInfo.imageType = VK_IMAGE_TYPE_2D;
    img2CreateInfo.extent.width = 1920;
    img2CreateInfo.extent.height = 1080;
    img2CreateInfo.extent.depth = 1;
    img2CreateInfo.mipLevels = 1;
    img2CreateInfo.arrayLayers = 1;
    img2CreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    img2CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    img2CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img2CreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    img2CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage img1 = VK_NULL_HANDLE;
    ERR_GUARD_VULKAN(vkCreateImage(g_hDevice, &img1CreateInfo, g_Allocs, &img1));
    VkImage img2 = VK_NULL_HANDLE;
    ERR_GUARD_VULKAN(vkCreateImage(g_hDevice, &img2CreateInfo, g_Allocs, &img2));

    VkMemoryRequirements img1MemReq = {};
    vkGetImageMemoryRequirements(g_hDevice, img1, &img1MemReq);
    VkMemoryRequirements img2MemReq = {};
    vkGetImageMemoryRequirements(g_hDevice, img2, &img2MemReq);

    VkMemoryRequirements finalMemReq = {};
    finalMemReq.size = std::max(img1MemReq.size, img2MemReq.size);
    finalMemReq.alignment = std::max(img1MemReq.alignment, img2MemReq.alignment);
    finalMemReq.memoryTypeBits = img1MemReq.memoryTypeBits & img2MemReq.memoryTypeBits;
    if(finalMemReq.memoryTypeBits != 0)
    {
        wprintf(L"  size: max(%llu, %llu) = %llu\n",
            img1MemReq.size, img2MemReq.size, finalMemReq.size);
        wprintf(L"  alignment: max(%llu, %llu) = %llu\n",
            img1MemReq.alignment, img2MemReq.alignment, finalMemReq.alignment);
        wprintf(L"  memoryTypeBits: %u & %u = %u\n",
            img1MemReq.memoryTypeBits, img2MemReq.memoryTypeBits, finalMemReq.memoryTypeBits);

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VmaAllocation alloc = VK_NULL_HANDLE;
        ERR_GUARD_VULKAN(vmaAllocateMemory(g_hAllocator, &finalMemReq, &allocCreateInfo, &alloc, nullptr));

        ERR_GUARD_VULKAN(vmaBindImageMemory(g_hAllocator, alloc, img1));
        ERR_GUARD_VULKAN(vmaBindImageMemory(g_hAllocator, alloc, img2));

        // You can use img1, img2 here, but not at the same time!

        vmaFreeMemory(g_hAllocator, alloc);
    }
    else
    {
        wprintf(L"  Textures cannot alias!\n");
    }

    vkDestroyImage(g_hDevice, img2, g_Allocs);
    vkDestroyImage(g_hDevice, img1, g_Allocs);
}

static void TestMapping()
{
    wprintf(L"Testing mapping...\n");

    VkResult res;
    uint32_t memTypeIndex = UINT32_MAX;

    enum TEST
    {
        TEST_NORMAL,
        TEST_POOL,
        TEST_DEDICATED,
        TEST_COUNT
    };
    for(uint32_t testIndex = 0; testIndex < TEST_COUNT; ++testIndex)
    {
        VmaPool pool = nullptr;
        if(testIndex == TEST_POOL)
        {
            TEST(memTypeIndex != UINT32_MAX);
            VmaPoolCreateInfo poolInfo = {};
            poolInfo.memoryTypeIndex = memTypeIndex;
            res = vmaCreatePool(g_hAllocator, &poolInfo, &pool);
            TEST(res == VK_SUCCESS);
        }

        VkBufferCreateInfo bufInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufInfo.size = 0x10000;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        allocCreateInfo.pool = pool;
        if(testIndex == TEST_DEDICATED)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        VmaAllocationInfo allocInfo;

        // Mapped manually

        // Create 2 buffers.
        BufferInfo bufferInfos[3];
        for(size_t i = 0; i < 2; ++i)
        {
            res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo,
                &bufferInfos[i].Buffer, &bufferInfos[i].Allocation, &allocInfo);
            TEST(res == VK_SUCCESS);
            TEST(allocInfo.pMappedData == nullptr);
            memTypeIndex = allocInfo.memoryType;
        }

        // Map buffer 0.
        char* data00 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[0].Allocation, (void**)&data00);
        TEST(res == VK_SUCCESS && data00 != nullptr);
        data00[0xFFFF] = data00[0];

        // Map buffer 0 second time.
        char* data01 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[0].Allocation, (void**)&data01);
        TEST(res == VK_SUCCESS && data01 == data00);

        // Map buffer 1.
        char* data1 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[1].Allocation, (void**)&data1);
        TEST(res == VK_SUCCESS && data1 != nullptr);
        TEST(!MemoryRegionsOverlap(data00, (size_t)bufInfo.size, data1, (size_t)bufInfo.size));
        data1[0xFFFF] = data1[0];

        // Unmap buffer 0 two times.
        vmaUnmapMemory(g_hAllocator, bufferInfos[0].Allocation);
        vmaUnmapMemory(g_hAllocator, bufferInfos[0].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[0].Allocation, &allocInfo);
        TEST(allocInfo.pMappedData == nullptr);

        // Unmap buffer 1.
        vmaUnmapMemory(g_hAllocator, bufferInfos[1].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[1].Allocation, &allocInfo);
        TEST(allocInfo.pMappedData == nullptr);

        // Create 3rd buffer - persistently mapped.
        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo,
            &bufferInfos[2].Buffer, &bufferInfos[2].Allocation, &allocInfo);
        TEST(res == VK_SUCCESS && allocInfo.pMappedData != nullptr);

        // Map buffer 2.
        char* data2 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[2].Allocation, (void**)&data2);
        TEST(res == VK_SUCCESS && data2 == allocInfo.pMappedData);
        data2[0xFFFF] = data2[0];

        // Unmap buffer 2.
        vmaUnmapMemory(g_hAllocator, bufferInfos[2].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[2].Allocation, &allocInfo);
        TEST(allocInfo.pMappedData == data2);

        // Destroy all buffers.
        for(size_t i = 3; i--; )
            vmaDestroyBuffer(g_hAllocator, bufferInfos[i].Buffer, bufferInfos[i].Allocation);

        vmaDestroyPool(g_hAllocator, pool);
    }
}

// Test CREATE_MAPPED with required DEVICE_LOCAL. There was a bug with it.
static void TestDeviceLocalMapped()
{
    VkResult res;

    for(uint32_t testIndex = 0; testIndex < 3; ++testIndex)
    {
        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufCreateInfo.size = 4096;

        VmaPool pool = VK_NULL_HANDLE;
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        if(testIndex == 2)
        {
            VmaPoolCreateInfo poolCreateInfo = {};
            res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &poolCreateInfo.memoryTypeIndex);
            TEST(res == VK_SUCCESS);
            res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
            TEST(res == VK_SUCCESS);
            allocCreateInfo.pool = pool;
        }
        else if(testIndex == 1)
        {
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;
        }

        VkBuffer buf = VK_NULL_HANDLE;
        VmaAllocation alloc = VK_NULL_HANDLE;
        VmaAllocationInfo allocInfo = {};
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
        TEST(res == VK_SUCCESS && alloc);

        VkMemoryPropertyFlags memTypeFlags = 0;
        vmaGetMemoryTypeProperties(g_hAllocator, allocInfo.memoryType, &memTypeFlags);
        const bool shouldBeMapped = (memTypeFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        TEST((allocInfo.pMappedData != nullptr) == shouldBeMapped);

        vmaDestroyBuffer(g_hAllocator, buf, alloc);
        vmaDestroyPool(g_hAllocator, pool);
    }
}

static void TestMappingMultithreaded()
{
    wprintf(L"Testing mapping multithreaded...\n");

    static const uint32_t threadCount = 16;
    static const uint32_t bufferCount = 1024;
    static const uint32_t threadBufferCount = bufferCount / threadCount;

    VkResult res;
    volatile uint32_t memTypeIndex = UINT32_MAX;

    enum TEST
    {
        TEST_NORMAL,
        TEST_POOL,
        TEST_DEDICATED,
        TEST_COUNT
    };
    for(uint32_t testIndex = 0; testIndex < TEST_COUNT; ++testIndex)
    {
        VmaPool pool = nullptr;
        if(testIndex == TEST_POOL)
        {
            TEST(memTypeIndex != UINT32_MAX);
            VmaPoolCreateInfo poolInfo = {};
            poolInfo.memoryTypeIndex = memTypeIndex;
            res = vmaCreatePool(g_hAllocator, &poolInfo, &pool);
            TEST(res == VK_SUCCESS);
        }

        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.size = 0x10000;
        bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        allocCreateInfo.pool = pool;
        if(testIndex == TEST_DEDICATED)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
        std::thread threads[threadCount];
        for(uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
        {
            threads[threadIndex] = std::thread([=, &memTypeIndex](){
                // ======== THREAD FUNCTION ========

                RandomNumberGenerator rand{threadIndex};
                
                enum class MODE
                {
                    // Don't map this buffer at all.
                    DONT_MAP,
                    // Map and quickly unmap.
                    MAP_FOR_MOMENT,
                    // Map and unmap before destruction.
                    MAP_FOR_LONGER,
                    // Map two times. Quickly unmap, second unmap before destruction.
                    MAP_TWO_TIMES,
                    // Create this buffer as persistently mapped.
                    PERSISTENTLY_MAPPED,
                    COUNT
                };
                std::vector<BufferInfo> bufInfos{threadBufferCount};
                std::vector<MODE> bufModes{threadBufferCount};
                
                for(uint32_t bufferIndex = 0; bufferIndex < threadBufferCount; ++bufferIndex)
                {
                    BufferInfo& bufInfo = bufInfos[bufferIndex];
                    const MODE mode = (MODE)(rand.Generate() % (uint32_t)MODE::COUNT);
                    bufModes[bufferIndex] = mode;

                    VmaAllocationCreateInfo localAllocCreateInfo = allocCreateInfo;
                    if(mode == MODE::PERSISTENTLY_MAPPED)
                        localAllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    
                    VmaAllocationInfo allocInfo;
                    VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &localAllocCreateInfo,
                        &bufInfo.Buffer, &bufInfo.Allocation, &allocInfo);
                    TEST(res == VK_SUCCESS);
                    
                    if(memTypeIndex == UINT32_MAX)
                        memTypeIndex = allocInfo.memoryType;

                    char* data = nullptr;

                    if(mode == MODE::PERSISTENTLY_MAPPED)
                    {
                        data = (char*)allocInfo.pMappedData;
                        TEST(data != nullptr);
                    }
                    else if(mode == MODE::MAP_FOR_MOMENT || mode == MODE::MAP_FOR_LONGER ||
                        mode == MODE::MAP_TWO_TIMES)
                    {
                        TEST(data == nullptr);
                        res = vmaMapMemory(g_hAllocator, bufInfo.Allocation, (void**)&data);
                        TEST(res == VK_SUCCESS && data != nullptr);

                        if(mode == MODE::MAP_TWO_TIMES)
                        {
                            char* data2 = nullptr;
                            res = vmaMapMemory(g_hAllocator, bufInfo.Allocation, (void**)&data2);
                            TEST(res == VK_SUCCESS && data2 == data);
                        }
                    }
                    else if(mode == MODE::DONT_MAP)
                    {
                        TEST(allocInfo.pMappedData == nullptr);
                    }
                    else
                        TEST(0);

                    // Test if reading and writing from the beginning and end of mapped memory doesn't crash.
                    if(data)
                        data[0xFFFF] = data[0];

                    if(mode == MODE::MAP_FOR_MOMENT || mode == MODE::MAP_TWO_TIMES)
                    {
                        vmaUnmapMemory(g_hAllocator, bufInfo.Allocation);

                        VmaAllocationInfo allocInfo;
                        vmaGetAllocationInfo(g_hAllocator, bufInfo.Allocation, &allocInfo);
                        if(mode == MODE::MAP_FOR_MOMENT)
                            TEST(allocInfo.pMappedData == nullptr);
                        else
                            TEST(allocInfo.pMappedData == data);
                    }

                    switch(rand.Generate() % 3)
                    {
                    case 0: Sleep(0); break; // Yield.
                    case 1: Sleep(10); break; // 10 ms
                    // default: No sleep.
                    }

                    // Test if reading and writing from the beginning and end of mapped memory doesn't crash.
                    if(data)
                        data[0xFFFF] = data[0];
                }

                for(size_t bufferIndex = threadBufferCount; bufferIndex--; )
                {
                    if(bufModes[bufferIndex] == MODE::MAP_FOR_LONGER ||
                        bufModes[bufferIndex] == MODE::MAP_TWO_TIMES)
                    {
                        vmaUnmapMemory(g_hAllocator, bufInfos[bufferIndex].Allocation);

                        VmaAllocationInfo allocInfo;
                        vmaGetAllocationInfo(g_hAllocator, bufInfos[bufferIndex].Allocation, &allocInfo);
                        TEST(allocInfo.pMappedData == nullptr);
                    }

                    vmaDestroyBuffer(g_hAllocator, bufInfos[bufferIndex].Buffer, bufInfos[bufferIndex].Allocation);
                }
            });
        }

        for(uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
            threads[threadIndex].join();
    
        vmaDestroyPool(g_hAllocator, pool);
    }
}

static void WriteMainTestResultHeader(FILE* file)
{
    fprintf(file,
        "Code,Time,"
        "Threads,Buffers and images,Sizes,Operations,Allocation strategy,Free order,"
        "Total Time (us),"
        "Allocation Time Min (us),"
        "Allocation Time Avg (us),"
        "Allocation Time Max (us),"
        "Deallocation Time Min (us),"
        "Deallocation Time Avg (us),"
        "Deallocation Time Max (us),"
        "Total Memory Allocated (B),"
        "Free Range Size Avg (B),"
        "Free Range Size Max (B)\n");
}

static void WriteMainTestResult(
    FILE* file,
    const char* codeDescription,
    const char* testDescription,
    const Config& config, const Result& result)
{
    float totalTimeSeconds = ToFloatSeconds(result.TotalTime);
    float allocationTimeMinSeconds = ToFloatSeconds(result.AllocationTimeMin);
    float allocationTimeAvgSeconds = ToFloatSeconds(result.AllocationTimeAvg);
    float allocationTimeMaxSeconds = ToFloatSeconds(result.AllocationTimeMax);
    float deallocationTimeMinSeconds = ToFloatSeconds(result.DeallocationTimeMin);
    float deallocationTimeAvgSeconds = ToFloatSeconds(result.DeallocationTimeAvg);
    float deallocationTimeMaxSeconds = ToFloatSeconds(result.DeallocationTimeMax);

    std::string currTime;
    CurrentTimeToStr(currTime);

    fprintf(file,
        "%s,%s,%s,"
        "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%I64u,%I64u,%I64u\n",
        codeDescription,
        currTime.c_str(),
        testDescription,
        totalTimeSeconds * 1e6f,
        allocationTimeMinSeconds * 1e6f,
        allocationTimeAvgSeconds * 1e6f,
        allocationTimeMaxSeconds * 1e6f,
        deallocationTimeMinSeconds * 1e6f,
        deallocationTimeAvgSeconds * 1e6f,
        deallocationTimeMaxSeconds * 1e6f,
        result.TotalMemoryAllocated,
        result.FreeRangeSizeAvg,
        result.FreeRangeSizeMax);
}

static void WritePoolTestResultHeader(FILE* file)
{
    fprintf(file,
        "Code,Test,Time,"
        "Config,"
        "Total Time (us),"
        "Allocation Time Min (us),"
        "Allocation Time Avg (us),"
        "Allocation Time Max (us),"
        "Deallocation Time Min (us),"
        "Deallocation Time Avg (us),"
        "Deallocation Time Max (us),"
        "Lost Allocation Count,"
        "Lost Allocation Total Size (B),"
        "Failed Allocation Count,"
        "Failed Allocation Total Size (B)\n");
}

static void WritePoolTestResult(
    FILE* file,
    const char* codeDescription,
    const char* testDescription,
    const PoolTestConfig& config,
    const PoolTestResult& result)
{
    float totalTimeSeconds = ToFloatSeconds(result.TotalTime);
    float allocationTimeMinSeconds = ToFloatSeconds(result.AllocationTimeMin);
    float allocationTimeAvgSeconds = ToFloatSeconds(result.AllocationTimeAvg);
    float allocationTimeMaxSeconds = ToFloatSeconds(result.AllocationTimeMax);
    float deallocationTimeMinSeconds = ToFloatSeconds(result.DeallocationTimeMin);
    float deallocationTimeAvgSeconds = ToFloatSeconds(result.DeallocationTimeAvg);
    float deallocationTimeMaxSeconds = ToFloatSeconds(result.DeallocationTimeMax);

    std::string currTime;
    CurrentTimeToStr(currTime);

    fprintf(file,
        "%s,%s,%s,"
        "ThreadCount=%u PoolSize=%llu FrameCount=%u TotalItemCount=%u UsedItemCount=%u...%u ItemsToMakeUnusedPercent=%u,"
        "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%I64u,%I64u,%I64u,%I64u\n",
        // General
        codeDescription,
        testDescription,
        currTime.c_str(),
        // Config
        config.ThreadCount,
        (unsigned long long)config.PoolSize,
        config.FrameCount,
        config.TotalItemCount,
        config.UsedItemCountMin,
        config.UsedItemCountMax,
        config.ItemsToMakeUnusedPercent,
        // Results
        totalTimeSeconds * 1e6f,
        allocationTimeMinSeconds * 1e6f,
        allocationTimeAvgSeconds * 1e6f,
        allocationTimeMaxSeconds * 1e6f,
        deallocationTimeMinSeconds * 1e6f,
        deallocationTimeAvgSeconds * 1e6f,
        deallocationTimeMaxSeconds * 1e6f,
        result.LostAllocationCount,
        result.LostAllocationTotalSize,
        result.FailedAllocationCount,
        result.FailedAllocationTotalSize);
}

static void PerformCustomMainTest(FILE* file)
{
    Config config{};
    config.RandSeed = 65735476;
    //config.MaxBytesToAllocate = 4ull * 1024 * 1024; // 4 MB
    config.MaxBytesToAllocate = 4ull * 1024 * 1024 * 1024; // 4 GB
    config.MemUsageProbability[0] = 1; // VMA_MEMORY_USAGE_GPU_ONLY
    config.FreeOrder = FREE_ORDER::FORWARD;
    config.ThreadCount = 16;
    config.ThreadsUsingCommonAllocationsProbabilityPercent = 50;
    config.AllocationStrategy = 0;

    // Buffers
    //config.AllocationSizes.push_back({4, 16, 1024});
    config.AllocationSizes.push_back({4, 0x10000, 0xA00000}); // 64 KB ... 10 MB

    // Images
    //config.AllocationSizes.push_back({4, 0, 0, 4, 32});
    //config.AllocationSizes.push_back({4, 0, 0, 256, 2048});

    config.BeginBytesToAllocate = config.MaxBytesToAllocate * 5 / 100;
    config.AdditionalOperationCount = 1024;

    Result result{};
    VkResult res = MainTest(result, config);
    TEST(res == VK_SUCCESS);
    WriteMainTestResult(file, "Foo", "CustomTest", config, result);
}

static void PerformCustomPoolTest(FILE* file)
{
    PoolTestConfig config;
    config.PoolSize = 100 * 1024 * 1024;
    config.RandSeed = 2345764;
    config.ThreadCount = 1;
    config.FrameCount = 200;
    config.ItemsToMakeUnusedPercent = 2;
    
    AllocationSize allocSize = {};
    allocSize.BufferSizeMin = 1024;
    allocSize.BufferSizeMax = 1024 * 1024;
    allocSize.Probability = 1;
    config.AllocationSizes.push_back(allocSize);

    allocSize.BufferSizeMin = 0;
    allocSize.BufferSizeMax = 0;
    allocSize.ImageSizeMin = 128;
    allocSize.ImageSizeMax = 1024;
    allocSize.Probability = 1;
    config.AllocationSizes.push_back(allocSize);

    config.PoolSize = config.CalcAvgResourceSize() * 200;
    config.UsedItemCountMax = 160;
    config.TotalItemCount = config.UsedItemCountMax * 10;
    config.UsedItemCountMin = config.UsedItemCountMax * 80 / 100;

    PoolTestResult result = {};
    TestPool_Benchmark(result, config);

    WritePoolTestResult(file, "Code desc", "Test desc", config, result);
}

static void PerformMainTests(FILE* file)
{
    wprintf(L"MAIN TESTS:\n");

    uint32_t repeatCount = 1;
    if(ConfigType >= CONFIG_TYPE_MAXIMUM) repeatCount = 3;

    Config config{};
    config.RandSeed = 65735476;
    config.MemUsageProbability[0] = 1; // VMA_MEMORY_USAGE_GPU_ONLY
    config.FreeOrder = FREE_ORDER::FORWARD;

    size_t threadCountCount = 1;
    switch(ConfigType)
    {
    case CONFIG_TYPE_MINIMUM: threadCountCount = 1; break;
    case CONFIG_TYPE_SMALL:   threadCountCount = 2; break;
    case CONFIG_TYPE_AVERAGE: threadCountCount = 3; break;
    case CONFIG_TYPE_LARGE:   threadCountCount = 5; break;
    case CONFIG_TYPE_MAXIMUM: threadCountCount = 7; break;
    default: assert(0);
    }

    const size_t strategyCount = GetAllocationStrategyCount();

    for(size_t threadCountIndex = 0; threadCountIndex < threadCountCount; ++threadCountIndex)
    {
        std::string desc1;

        switch(threadCountIndex)
        {
        case 0:
            desc1 += "1_thread";
            config.ThreadCount = 1;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 0;
            break;
        case 1:
            desc1 += "16_threads+0%_common";
            config.ThreadCount = 16;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 0;
            break;
        case 2:
            desc1 += "16_threads+50%_common";
            config.ThreadCount = 16;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 50;
            break;
        case 3:
            desc1 += "16_threads+100%_common";
            config.ThreadCount = 16;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 100;
            break;
        case 4:
            desc1 += "2_threads+0%_common";
            config.ThreadCount = 2;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 0;
            break;
        case 5:
            desc1 += "2_threads+50%_common";
            config.ThreadCount = 2;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 50;
            break;
        case 6:
            desc1 += "2_threads+100%_common";
            config.ThreadCount = 2;
            config.ThreadsUsingCommonAllocationsProbabilityPercent = 100;
            break;
        default:
            assert(0);
        }

        // 0 = buffers, 1 = images, 2 = buffers and images
        size_t buffersVsImagesCount = 2;
        if(ConfigType >= CONFIG_TYPE_LARGE) ++buffersVsImagesCount;
        for(size_t buffersVsImagesIndex = 0; buffersVsImagesIndex < buffersVsImagesCount; ++buffersVsImagesIndex)
        {
            std::string desc2 = desc1;
            switch(buffersVsImagesIndex)
            {
            case 0: desc2 += ",Buffers"; break;
            case 1: desc2 += ",Images"; break;
            case 2: desc2 += ",Buffers+Images"; break;
            default: assert(0);
            }

            // 0 = small, 1 = large, 2 = small and large
            size_t smallVsLargeCount = 2;
            if(ConfigType >= CONFIG_TYPE_LARGE) ++smallVsLargeCount;
            for(size_t smallVsLargeIndex = 0; smallVsLargeIndex < smallVsLargeCount; ++smallVsLargeIndex)
            {
                std::string desc3 = desc2;
                switch(smallVsLargeIndex)
                {
                case 0: desc3 += ",Small"; break;
                case 1: desc3 += ",Large"; break;
                case 2: desc3 += ",Small+Large"; break;
                default: assert(0);
                }

                if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                    config.MaxBytesToAllocate = 4ull * 1024 * 1024 * 1024; // 4 GB
                else
                    config.MaxBytesToAllocate = 4ull * 1024 * 1024;

                // 0 = varying sizes min...max, 1 = set of constant sizes
                size_t constantSizesCount = 1;
                if(ConfigType >= CONFIG_TYPE_SMALL) ++constantSizesCount;
                for(size_t constantSizesIndex = 0; constantSizesIndex < constantSizesCount; ++constantSizesIndex)
                {
                    std::string desc4 = desc3;
                    switch(constantSizesIndex)
                    {
                    case 0: desc4 += " Varying_sizes"; break;
                    case 1: desc4 += " Constant_sizes"; break;
                    default: assert(0);
                    }

                    config.AllocationSizes.clear();
                    // Buffers present
                    if(buffersVsImagesIndex == 0 || buffersVsImagesIndex == 2)
                    {
                        // Small
                        if(smallVsLargeIndex == 0 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 16, 1024});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 16, 16});
                                config.AllocationSizes.push_back({1, 64, 64});
                                config.AllocationSizes.push_back({1, 256, 256});
                                config.AllocationSizes.push_back({1, 1024, 1024});
                            }
                        }
                        // Large
                        if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0x10000, 0xA00000}); // 64 KB ... 10 MB
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0x10000, 0x10000});
                                config.AllocationSizes.push_back({1, 0x80000, 0x80000});
                                config.AllocationSizes.push_back({1, 0x200000, 0x200000});
                                config.AllocationSizes.push_back({1, 0xA00000, 0xA00000});
                            }
                        }
                    }
                    // Images present
                    if(buffersVsImagesIndex == 1 || buffersVsImagesIndex == 2)
                    {
                        // Small
                        if(smallVsLargeIndex == 0 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0, 0, 4, 32});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0, 0,  4,  4});
                                config.AllocationSizes.push_back({1, 0, 0,  8,  8});
                                config.AllocationSizes.push_back({1, 0, 0, 16, 16});
                                config.AllocationSizes.push_back({1, 0, 0, 32, 32});
                            }
                        }
                        // Large
                        if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0, 0, 256, 2048});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0, 0,  256,  256});
                                config.AllocationSizes.push_back({1, 0, 0,  512,  512});
                                config.AllocationSizes.push_back({1, 0, 0, 1024, 1024});
                                config.AllocationSizes.push_back({1, 0, 0, 2048, 2048});
                            }
                        }
                    }

                    // 0 = 100%, additional_operations = 0, 1 = 50%, 2 = 5%, 3 = 95% additional_operations = a lot
                    size_t beginBytesToAllocateCount = 1;
                    if(ConfigType >= CONFIG_TYPE_SMALL) ++beginBytesToAllocateCount;
                    if(ConfigType >= CONFIG_TYPE_AVERAGE) ++beginBytesToAllocateCount;
                    if(ConfigType >= CONFIG_TYPE_LARGE) ++beginBytesToAllocateCount;
                    for(size_t beginBytesToAllocateIndex = 0; beginBytesToAllocateIndex < beginBytesToAllocateCount; ++beginBytesToAllocateIndex)
                    {
                        std::string desc5 = desc4;

                        switch(beginBytesToAllocateIndex)
                        {
                        case 0:
                            desc5 += ",Allocate_100%";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate;
                            config.AdditionalOperationCount = 0;
                            break;
                        case 1:
                            desc5 += ",Allocate_50%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate * 50 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        case 2:
                            desc5 += ",Allocate_5%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate *  5 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        case 3:
                            desc5 += ",Allocate_95%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate * 95 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        default:
                            assert(0);
                        }

                        for(size_t strategyIndex = 0; strategyIndex < strategyCount; ++strategyIndex)
                        {
                            std::string desc6 = desc5;
                            switch(strategyIndex)
                            {
                            case 0:
                                desc6 += ",BestFit";
                                config.AllocationStrategy = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
                                break;
                            case 1:
                                desc6 += ",WorstFit";
                                config.AllocationStrategy = VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT;
                                break;
                            case 2:
                                desc6 += ",FirstFit";
                                config.AllocationStrategy = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT;
                                break;
                            default:
                                assert(0);
                            }

                            desc6 += ',';
                            desc6 += FREE_ORDER_NAMES[(uint32_t)config.FreeOrder];

                            const char* testDescription = desc6.c_str();

                            for(size_t repeat = 0; repeat < repeatCount; ++repeat)
                            {
                                printf("%s #%u\n", testDescription, (uint32_t)repeat);

                                Result result{};
                                VkResult res = MainTest(result, config);
                                TEST(res == VK_SUCCESS);
                                if(file)
                                {
                                    WriteMainTestResult(file, CODE_DESCRIPTION, testDescription, config, result);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static void PerformPoolTests(FILE* file)
{
    wprintf(L"POOL TESTS:\n");

    const size_t AVG_RESOURCES_PER_POOL = 300;

    uint32_t repeatCount = 1;
    if(ConfigType >= CONFIG_TYPE_MAXIMUM) repeatCount = 3;

    PoolTestConfig config{};
    config.RandSeed = 2346343;
    config.FrameCount = 200;
    config.ItemsToMakeUnusedPercent = 2;

    size_t threadCountCount = 1;
    switch(ConfigType)
    {
    case CONFIG_TYPE_MINIMUM: threadCountCount = 1; break;
    case CONFIG_TYPE_SMALL:   threadCountCount = 2; break;
    case CONFIG_TYPE_AVERAGE: threadCountCount = 2; break;
    case CONFIG_TYPE_LARGE:   threadCountCount = 3; break;
    case CONFIG_TYPE_MAXIMUM: threadCountCount = 3; break;
    default: assert(0);
    }
    for(size_t threadCountIndex = 0; threadCountIndex < threadCountCount; ++threadCountIndex)
    {
        std::string desc1;

        switch(threadCountIndex)
        {
        case 0:
            desc1 += "1_thread";
            config.ThreadCount = 1;
            break;
        case 1:
            desc1 += "16_threads";
            config.ThreadCount = 16;
            break;
        case 2:
            desc1 += "2_threads";
            config.ThreadCount = 2;
            break;
        default:
            assert(0);
        }

        // 0 = buffers, 1 = images, 2 = buffers and images
        size_t buffersVsImagesCount = 2;
        if(ConfigType >= CONFIG_TYPE_LARGE) ++buffersVsImagesCount;
        for(size_t buffersVsImagesIndex = 0; buffersVsImagesIndex < buffersVsImagesCount; ++buffersVsImagesIndex)
        {
            std::string desc2 = desc1;
            switch(buffersVsImagesIndex)
            {
            case 0: desc2 += " Buffers"; break;
            case 1: desc2 += " Images"; break;
            case 2: desc2 += " Buffers+Images"; break;
            default: assert(0);
            }

            // 0 = small, 1 = large, 2 = small and large
            size_t smallVsLargeCount = 2;
            if(ConfigType >= CONFIG_TYPE_LARGE) ++smallVsLargeCount;
            for(size_t smallVsLargeIndex = 0; smallVsLargeIndex < smallVsLargeCount; ++smallVsLargeIndex)
            {
                std::string desc3 = desc2;
                switch(smallVsLargeIndex)
                {
                case 0: desc3 += " Small"; break;
                case 1: desc3 += " Large"; break;
                case 2: desc3 += " Small+Large"; break;
                default: assert(0);
                }

                if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                    config.PoolSize = 6ull * 1024 * 1024 * 1024; // 6 GB
                else
                    config.PoolSize = 4ull * 1024 * 1024;

                // 0 = varying sizes min...max, 1 = set of constant sizes
                size_t constantSizesCount = 1;
                if(ConfigType >= CONFIG_TYPE_SMALL) ++constantSizesCount;
                for(size_t constantSizesIndex = 0; constantSizesIndex < constantSizesCount; ++constantSizesIndex)
                {
                    std::string desc4 = desc3;
                    switch(constantSizesIndex)
                    {
                    case 0: desc4 += " Varying_sizes"; break;
                    case 1: desc4 += " Constant_sizes"; break;
                    default: assert(0);
                    }

                    config.AllocationSizes.clear();
                    // Buffers present
                    if(buffersVsImagesIndex == 0 || buffersVsImagesIndex == 2)
                    {
                        // Small
                        if(smallVsLargeIndex == 0 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 16, 1024});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 16, 16});
                                config.AllocationSizes.push_back({1, 64, 64});
                                config.AllocationSizes.push_back({1, 256, 256});
                                config.AllocationSizes.push_back({1, 1024, 1024});
                            }
                        }
                        // Large
                        if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0x10000, 0xA00000}); // 64 KB ... 10 MB
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0x10000, 0x10000});
                                config.AllocationSizes.push_back({1, 0x80000, 0x80000});
                                config.AllocationSizes.push_back({1, 0x200000, 0x200000});
                                config.AllocationSizes.push_back({1, 0xA00000, 0xA00000});
                            }
                        }
                    }
                    // Images present
                    if(buffersVsImagesIndex == 1 || buffersVsImagesIndex == 2)
                    {
                        // Small
                        if(smallVsLargeIndex == 0 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0, 0, 4, 32});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0, 0,  4,  4});
                                config.AllocationSizes.push_back({1, 0, 0,  8,  8});
                                config.AllocationSizes.push_back({1, 0, 0, 16, 16});
                                config.AllocationSizes.push_back({1, 0, 0, 32, 32});
                            }
                        }
                        // Large
                        if(smallVsLargeIndex == 1 || smallVsLargeIndex == 2)
                        {
                            // Varying size
                            if(constantSizesIndex == 0)
                                config.AllocationSizes.push_back({4, 0, 0, 256, 2048});
                            // Constant sizes
                            else
                            {
                                config.AllocationSizes.push_back({1, 0, 0,  256,  256});
                                config.AllocationSizes.push_back({1, 0, 0,  512,  512});
                                config.AllocationSizes.push_back({1, 0, 0, 1024, 1024});
                                config.AllocationSizes.push_back({1, 0, 0, 2048, 2048});
                            }
                        }
                    }

                    const VkDeviceSize avgResourceSize = config.CalcAvgResourceSize();
                    config.PoolSize = avgResourceSize * AVG_RESOURCES_PER_POOL;

                    // 0 = 66%, 1 = 133%, 2 = 100%, 3 = 33%, 4 = 166%
                    size_t subscriptionModeCount;
                    switch(ConfigType)
                    {
                    case CONFIG_TYPE_MINIMUM: subscriptionModeCount = 2; break;
                    case CONFIG_TYPE_SMALL:   subscriptionModeCount = 2; break;
                    case CONFIG_TYPE_AVERAGE: subscriptionModeCount = 3; break;
                    case CONFIG_TYPE_LARGE:   subscriptionModeCount = 5; break;
                    case CONFIG_TYPE_MAXIMUM: subscriptionModeCount = 5; break;
                    default: assert(0);
                    }
                    for(size_t subscriptionModeIndex = 0; subscriptionModeIndex < subscriptionModeCount; ++subscriptionModeIndex)
                    {
                        std::string desc5 = desc4;

                        switch(subscriptionModeIndex)
                        {
                        case 0:
                            desc5 += " Subscription_66%";
                            config.UsedItemCountMax = AVG_RESOURCES_PER_POOL * 66 / 100;
                            break;
                        case 1:
                            desc5 += " Subscription_133%";
                            config.UsedItemCountMax = AVG_RESOURCES_PER_POOL * 133 / 100;
                            break;
                        case 2:
                            desc5 += " Subscription_100%";
                            config.UsedItemCountMax = AVG_RESOURCES_PER_POOL;
                            break;
                        case 3:
                            desc5 += " Subscription_33%";
                            config.UsedItemCountMax = AVG_RESOURCES_PER_POOL * 33 / 100;
                            break;
                        case 4:
                            desc5 += " Subscription_166%";
                            config.UsedItemCountMax = AVG_RESOURCES_PER_POOL * 166 / 100;
                            break;
                        default:
                            assert(0);
                        }

                        config.TotalItemCount = config.UsedItemCountMax * 5;
                        config.UsedItemCountMin = config.UsedItemCountMax * 80 / 100;

                        const char* testDescription = desc5.c_str();

                        for(size_t repeat = 0; repeat < repeatCount; ++repeat)
                        {
                            printf("%s #%u\n", testDescription, (uint32_t)repeat);

                            PoolTestResult result{};
                            TestPool_Benchmark(result, config);
                            WritePoolTestResult(file, CODE_DESCRIPTION, testDescription, config, result);
                        }
                    }
                }
            }
        }
    }
}

static void BasicTestBuddyAllocator()
{
    wprintf(L"Basic test buddy allocator\n");

    RandomNumberGenerator rand{76543};

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = 1024; // Whatever.
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    // Deliberately adding 1023 to test usable size smaller than memory block size.
    poolCreateInfo.blockSize = 1024 * 1024 + 1023;
    poolCreateInfo.flags = VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT;
    //poolCreateInfo.minBlockCount = poolCreateInfo.maxBlockCount = 1;

    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    VkBufferCreateInfo bufCreateInfo = sampleBufCreateInfo;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;

    std::vector<BufferInfo> bufInfo;
    BufferInfo newBufInfo;
    VmaAllocationInfo allocInfo;
    
    bufCreateInfo.size = 1024 * 256;
    res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
        &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
    TEST(res == VK_SUCCESS);
    bufInfo.push_back(newBufInfo);

    bufCreateInfo.size = 1024 * 512;
    res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
        &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
    TEST(res == VK_SUCCESS);
    bufInfo.push_back(newBufInfo);

    bufCreateInfo.size = 1024 * 128;
    res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
        &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
    TEST(res == VK_SUCCESS);
    bufInfo.push_back(newBufInfo);
    
    // Test very small allocation, smaller than minimum node size.
    bufCreateInfo.size = 1;
    res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
        &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
    TEST(res == VK_SUCCESS);
    bufInfo.push_back(newBufInfo);

    // Test some small allocation with alignment requirement.
    {
        VkMemoryRequirements memReq;
        memReq.alignment = 256;
        memReq.memoryTypeBits = UINT32_MAX;
        memReq.size = 32;

        newBufInfo.Buffer = VK_NULL_HANDLE;
        res = vmaAllocateMemory(g_hAllocator, &memReq, &allocCreateInfo,
            &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        TEST(allocInfo.offset % memReq.alignment == 0);
        bufInfo.push_back(newBufInfo);
    }

    //SaveAllocatorStatsToFile(L"TEST.json");

    VmaPoolStats stats = {};
    vmaGetPoolStats(g_hAllocator, pool, &stats);
    int DBG = 0; // Set breakpoint here to inspect `stats`.

    // Allocate enough new buffers to surely fall into second block.
    for(uint32_t i = 0; i < 32; ++i)
    {
        bufCreateInfo.size = 1024 * (rand.Generate() % 32 + 1);
        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo,
            &newBufInfo.Buffer, &newBufInfo.Allocation, &allocInfo);
        TEST(res == VK_SUCCESS);
        bufInfo.push_back(newBufInfo);
    }

    SaveAllocatorStatsToFile(L"BuddyTest01.json");

    // Destroy the buffers in random order.
    while(!bufInfo.empty())
    {
        const size_t indexToDestroy = rand.Generate() % bufInfo.size();
        const BufferInfo& currBufInfo = bufInfo[indexToDestroy];
        vmaDestroyBuffer(g_hAllocator, currBufInfo.Buffer, currBufInfo.Allocation);
        bufInfo.erase(bufInfo.begin() + indexToDestroy);
    }

    vmaDestroyPool(g_hAllocator, pool);
}

static void BasicTestAllocatePages()
{
    wprintf(L"Basic test allocate pages\n");

    RandomNumberGenerator rand{765461};

    VkBufferCreateInfo sampleBufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    sampleBufCreateInfo.size = 1024; // Whatever.
    sampleBufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo sampleAllocCreateInfo = {};
    sampleAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VmaPoolCreateInfo poolCreateInfo = {};
    VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &sampleBufCreateInfo, &sampleAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);
    TEST(res == VK_SUCCESS);

    // 1 block of 1 MB.
    poolCreateInfo.blockSize = 1024 * 1024;
    poolCreateInfo.minBlockCount = poolCreateInfo.maxBlockCount = 1;

    // Create pool.
    VmaPool pool = nullptr;
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    TEST(res == VK_SUCCESS);

    // Make 100 allocations of 4 KB - they should fit into the pool.
    VkMemoryRequirements memReq;
    memReq.memoryTypeBits = UINT32_MAX;
    memReq.alignment = 4 * 1024;
    memReq.size = 4 * 1024;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocCreateInfo.pool = pool;

    constexpr uint32_t allocCount = 100;

    std::vector<VmaAllocation> alloc{allocCount};
    std::vector<VmaAllocationInfo> allocInfo{allocCount};
    res = vmaAllocateMemoryPages(g_hAllocator, &memReq, &allocCreateInfo, allocCount, alloc.data(), allocInfo.data());
    TEST(res == VK_SUCCESS);
    for(uint32_t i = 0; i < allocCount; ++i)
    {
        TEST(alloc[i] != VK_NULL_HANDLE &&
            allocInfo[i].pMappedData != nullptr &&
            allocInfo[i].deviceMemory == allocInfo[0].deviceMemory &&
            allocInfo[i].memoryType == allocInfo[0].memoryType);
    }

    // Free the allocations.
    vmaFreeMemoryPages(g_hAllocator, allocCount, alloc.data());
    std::fill(alloc.begin(), alloc.end(), nullptr);
    std::fill(allocInfo.begin(), allocInfo.end(), VmaAllocationInfo{});

    // Try to make 100 allocations of 100 KB. This call should fail due to not enough memory.
    // Also test optional allocationInfo = null.
    memReq.size = 100 * 1024;
    res = vmaAllocateMemoryPages(g_hAllocator, &memReq, &allocCreateInfo, allocCount, alloc.data(), nullptr);
    TEST(res != VK_SUCCESS);
    TEST(std::find_if(alloc.begin(), alloc.end(), [](VmaAllocation alloc){ return alloc != VK_NULL_HANDLE; }) == alloc.end());

    // Make 100 allocations of 4 KB, but with required alignment of 128 KB. This should also fail.
    memReq.size = 4 * 1024;
    memReq.alignment = 128 * 1024;
    res = vmaAllocateMemoryPages(g_hAllocator, &memReq, &allocCreateInfo, allocCount, alloc.data(), allocInfo.data());
    TEST(res != VK_SUCCESS);

    // Make 100 dedicated allocations of 4 KB.
    memReq.alignment = 4 * 1024;
    memReq.size = 4 * 1024;

    VmaAllocationCreateInfo dedicatedAllocCreateInfo = {};
    dedicatedAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    dedicatedAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    res = vmaAllocateMemoryPages(g_hAllocator, &memReq, &dedicatedAllocCreateInfo, allocCount, alloc.data(), allocInfo.data());
    TEST(res == VK_SUCCESS);
    for(uint32_t i = 0; i < allocCount; ++i)
    {
        TEST(alloc[i] != VK_NULL_HANDLE &&
            allocInfo[i].pMappedData != nullptr &&
            allocInfo[i].memoryType == allocInfo[0].memoryType &&
            allocInfo[i].offset == 0);
        if(i > 0)
        {
            TEST(allocInfo[i].deviceMemory != allocInfo[0].deviceMemory);
        }
    }

    // Free the allocations.
    vmaFreeMemoryPages(g_hAllocator, allocCount, alloc.data());
    std::fill(alloc.begin(), alloc.end(), nullptr);
    std::fill(allocInfo.begin(), allocInfo.end(), VmaAllocationInfo{});

    vmaDestroyPool(g_hAllocator, pool);
}

// Test the testing environment.
static void TestGpuData()
{
    RandomNumberGenerator rand = { 53434 };

    std::vector<AllocInfo> allocInfo;

    for(size_t i = 0; i < 100; ++i)
    {
        AllocInfo info = {};

        info.m_BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.m_BufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.m_BufferInfo.size = 1024 * 1024 * (rand.Generate() % 9 + 1);

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult res = vmaCreateBuffer(g_hAllocator, &info.m_BufferInfo, &allocCreateInfo, &info.m_Buffer, &info.m_Allocation, nullptr);
        TEST(res == VK_SUCCESS);

        info.m_StartValue = rand.Generate();

        allocInfo.push_back(std::move(info));
    }

    UploadGpuData(allocInfo.data(), allocInfo.size());

    ValidateGpuData(allocInfo.data(), allocInfo.size());

    DestroyAllAllocations(allocInfo);
}

void Test()
{
    wprintf(L"TESTING:\n");

    if(false)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Temporarily insert custom tests here:
        return;
    }

    // # Simple tests

    TestBasics();
    TestAllocationVersusResourceSize();
    //TestGpuData(); // Not calling this because it's just testing the testing environment.
#if VMA_DEBUG_MARGIN
    TestDebugMargin();
#else
    TestPool_SameSize();
    TestPool_MinBlockCount();
    TestHeapSizeLimit();
#endif
#if VMA_DEBUG_INITIALIZE_ALLOCATIONS
    TestAllocationsInitialization();
#endif
    TestMemoryUsage();
    TestDeviceCoherentMemory();
    TestBudget();
    TestAliasing();
    TestMapping();
    TestDeviceLocalMapped();
    TestMappingMultithreaded();
    TestLinearAllocator();
    ManuallyTestLinearAllocator();
    TestLinearAllocatorMultiBlock();

    BasicTestBuddyAllocator();
    BasicTestAllocatePages();

    if(VK_KHR_buffer_device_address_enabled)
        TestBufferDeviceAddress();
    if(VK_EXT_memory_priority_enabled)
        TestMemoryPriority();

    {
        FILE* file;
        fopen_s(&file, "Algorithms.csv", "w");
        assert(file != NULL);
        BenchmarkAlgorithms(file);
        fclose(file);
    }

    TestDefragmentationSimple();
    TestDefragmentationFull();
    TestDefragmentationWholePool();
    TestDefragmentationGpu();
    TestDefragmentationIncrementalBasic();
    TestDefragmentationIncrementalComplex();

    // # Detailed tests
    FILE* file;
    fopen_s(&file, "Results.csv", "w");
    assert(file != NULL);
    
    WriteMainTestResultHeader(file);
    PerformMainTests(file);
    //PerformCustomMainTest(file);

    WritePoolTestResultHeader(file);
    PerformPoolTests(file);
    //PerformCustomPoolTest(file);
    
    fclose(file);
    
    wprintf(L"Done, all PASSED.\n");
}

#endif // #ifdef _WIN32
