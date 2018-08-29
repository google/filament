#include "Tests.h"
#include "VmaUsage.h"
#include "Common.h"
#include <atomic>
#include <thread>
#include <mutex>

#ifdef _WIN32

enum class FREE_ORDER { FORWARD, BACKWARD, RANDOM, COUNT };

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

struct BufferInfo
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
};

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
            assert(0);
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

static void SaveAllocatorStatsToFile(const wchar_t* filePath)
{
    char* stats;
    vmaBuildStatsString(g_hAllocator, &stats, VK_TRUE);
    SaveFile(filePath, stats, strlen(stats));
    vmaFreeStatsString(g_hAllocator, stats);
}

struct AllocInfo
{
    VmaAllocation m_Allocation;
    VkBuffer m_Buffer;
    VkImage m_Image;
    uint32_t m_StartValue;
    union
    {
        VkBufferCreateInfo m_BufferInfo;
        VkImageCreateInfo m_ImageInfo;
    };
};

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
        assert((data != nullptr) == persistentlyMapped);
        if(!persistentlyMapped)
        {
            ERR_GUARD_VULKAN( vmaMapMemory(g_hAllocator, outAllocInfo.m_Allocation, (void**)&data) );
        }

        uint32_t value = outAllocInfo.m_StartValue;
        assert(bufCreateInfo.size % 4 == 0);
        for(size_t i = 0; i < bufCreateInfo.size / sizeof(uint32_t); ++i)
            data[i] = value++;

        if(!persistentlyMapped)
            vmaUnmapMemory(g_hAllocator, outAllocInfo.m_Allocation);
    }
}

static void CreateAllocation(AllocInfo& outAllocation, VmaAllocator allocator)
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

        VkResult res = vmaCreateBuffer(allocator, &bufferInfo, &vmaMemReq, &outAllocation.m_Buffer, &outAllocation.m_Allocation, &allocInfo);
        outAllocation.m_BufferInfo = bufferInfo;
        assert(res == VK_SUCCESS);
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

        VkResult res = vmaCreateImage(allocator, &imageInfo, &vmaMemReq, &outAllocation.m_Image, &outAllocation.m_Allocation, &allocInfo);
        outAllocation.m_ImageInfo = imageInfo;
        assert(res == VK_SUCCESS);
    }

    uint32_t* data = (uint32_t*)allocInfo.pMappedData;
    if(allocInfo.pMappedData == nullptr)
    {
        VkResult res = vmaMapMemory(allocator, outAllocation.m_Allocation, (void**)&data);
        assert(res == VK_SUCCESS);
    }

    uint32_t value = outAllocation.m_StartValue;
    assert(allocInfo.size % 4 == 0);
    for(size_t i = 0; i < allocInfo.size / sizeof(uint32_t); ++i)
        data[i] = value++;

    if(allocInfo.pMappedData == nullptr)
        vmaUnmapMemory(allocator, outAllocation.m_Allocation);
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
        assert(res == VK_SUCCESS);
    }

    uint32_t value = allocation.m_StartValue;
    bool ok = true;
    size_t i;
    assert(allocInfo.size % 4 == 0);
    for(i = 0; i < allocInfo.size / sizeof(uint32_t); ++i)
    {
        if(data[i] != value++)
        {
            ok = false;
            break;
        }
    }
    assert(ok);

    if(allocInfo.pMappedData == nullptr)
        vmaUnmapMemory(g_hAllocator, allocation.m_Allocation);
}

static void RecreateAllocationResource(AllocInfo& allocation)
{
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(g_hAllocator, allocation.m_Allocation, &allocInfo);

    if(allocation.m_Buffer)
    {
        vkDestroyBuffer(g_hDevice, allocation.m_Buffer, nullptr);

        VkResult res = vkCreateBuffer(g_hDevice, &allocation.m_BufferInfo, nullptr, &allocation.m_Buffer);
        assert(res == VK_SUCCESS);

        // Just to silence validation layer warnings.
        VkMemoryRequirements vkMemReq;
        vkGetBufferMemoryRequirements(g_hDevice, allocation.m_Buffer, &vkMemReq);
        assert(vkMemReq.size == allocation.m_BufferInfo.size);

        res = vkBindBufferMemory(g_hDevice, allocation.m_Buffer, allocInfo.deviceMemory, allocInfo.offset);
        assert(res == VK_SUCCESS);
    }
    else
    {
        vkDestroyImage(g_hDevice, allocation.m_Image, nullptr);

        VkResult res = vkCreateImage(g_hDevice, &allocation.m_ImageInfo, nullptr, &allocation.m_Image);
        assert(res == VK_SUCCESS);

        // Just to silence validation layer warnings.
        VkMemoryRequirements vkMemReq;
        vkGetImageMemoryRequirements(g_hDevice, allocation.m_Image, &vkMemReq);

        res = vkBindImageMemory(g_hDevice, allocation.m_Image, allocInfo.deviceMemory, allocInfo.offset);
        assert(res == VK_SUCCESS);
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
            assert(defragStats.allocationsMoved > 0 && defragStats.bytesMoved > 0);
            assert(defragStats.deviceMemoryBlocksFreed >= 1);

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
                assert(defragStats.allocationsMoved > 0 && defragStats.bytesMoved > 0);
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

    vmaDestroyPool(g_hAllocator, pool);
}

void TestDefragmentationFull()
{
    std::vector<AllocInfo> allocations;

    // Create initial allocations.
    for(size_t i = 0; i < 400; ++i)
    {
        AllocInfo allocation;
        CreateAllocation(allocation, g_hAllocator);
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

    SaveAllocatorStatsToFile(L"Before.csv");

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
            assert(res >= 0);

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

            wchar_t fileName[MAX_PATH];
            swprintf(fileName, MAX_PATH, L"After_%02u.csv", defragIndex);
            SaveAllocatorStatsToFile(fileName);
        }
    }

    // Destroy all remaining allocations.
    DestroyAllAllocations(allocations);
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
            assert(res == VK_SUCCESS);
            assert(allocInfo.pUserData = numberAsPointer);

            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            assert(allocInfo.pUserData == numberAsPointer);

            vmaSetAllocationUserData(g_hAllocator, alloc, pointerToSomething);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            assert(allocInfo.pUserData == pointerToSomething);

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
            assert(res == VK_SUCCESS);
            assert(allocInfo.pUserData != nullptr && allocInfo.pUserData != name1Buf);
            assert(strcmp(name1, (const char*)allocInfo.pUserData) == 0);

            delete[] name1Buf;

            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            assert(strcmp(name1, (const char*)allocInfo.pUserData) == 0);

            vmaSetAllocationUserData(g_hAllocator, alloc, (void*)name2);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            assert(strcmp(name2, (const char*)allocInfo.pUserData) == 0);

            vmaSetAllocationUserData(g_hAllocator, alloc, nullptr);
            vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
            assert(allocInfo.pUserData == nullptr);

            vmaDestroyBuffer(g_hAllocator, buf, alloc);
        }
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
    assert(res == VK_SUCCESS);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // Usage.
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.requiredFlags = 0;
    allocCreateInfo.preferredFlags = 0;
    allocCreateInfo.memoryTypeBits = UINT32_MAX;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    assert(res == VK_SUCCESS);
    assert(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // Required flags, preferred flags.
    allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
    allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    allocCreateInfo.memoryTypeBits = 0;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    assert(res == VK_SUCCESS);
    assert(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    assert(memProps->memoryTypes[allocInfo.memoryType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

    // memoryTypeBits.
    const uint32_t memType = allocInfo.memoryType;
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.requiredFlags = 0;
    allocCreateInfo.preferredFlags = 0;
    allocCreateInfo.memoryTypeBits = 1u << memType;

    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
    assert(res == VK_SUCCESS);
    assert(allocInfo.memoryType == memType);
    vmaDestroyBuffer(g_hAllocator, buf, alloc);

}

static void TestBasics()
{
    VkResult res;

    TestMemoryRequirements();

    // Lost allocation
    {
        VmaAllocation alloc = VK_NULL_HANDLE;
        vmaCreateLostAllocation(g_hAllocator, &alloc);
        assert(alloc != VK_NULL_HANDLE);

        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, alloc, &allocInfo);
        assert(allocInfo.deviceMemory == VK_NULL_HANDLE);
        assert(allocInfo.size == 0);

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
        assert(res == VK_SUCCESS);

        vmaDestroyBuffer(g_hAllocator, buf, alloc);

        // Same with OWN_MEMORY.
        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);
        assert(res == VK_SUCCESS);

        vmaDestroyBuffer(g_hAllocator, buf, alloc);
    }

    TestUserData();
}

void TestHeapSizeLimit()
{
    const VkDeviceSize HEAP_SIZE_LIMIT = 1ull * 1024 * 1024 * 1024; // 1 GB
    const VkDeviceSize BLOCK_SIZE      = 128ull * 1024 * 1024;      // 128 MB

    VkDeviceSize heapSizeLimit[VK_MAX_MEMORY_HEAPS];
    for(uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i)
    {
        heapSizeLimit[i] = HEAP_SIZE_LIMIT;
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = g_hPhysicalDevice;
    allocatorCreateInfo.device = g_hDevice;
    allocatorCreateInfo.pHeapSizeLimit = heapSizeLimit;

    VmaAllocator hAllocator;
    VkResult res = vmaCreateAllocator(&allocatorCreateInfo, &hAllocator);
    assert(res == VK_SUCCESS);

    struct Item
    {
        VkBuffer hBuf;
        VmaAllocation hAlloc;
    };
    std::vector<Item> items;

    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // 1. Allocate two blocks of Own Memory, half the size of BLOCK_SIZE.
    VmaAllocationInfo ownAllocInfo;
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        bufCreateInfo.size = BLOCK_SIZE / 2;

        for(size_t i = 0; i < 2; ++i)
        {
            Item item;
            res = vmaCreateBuffer(hAllocator, &bufCreateInfo, &allocCreateInfo, &item.hBuf, &item.hAlloc, &ownAllocInfo);
            assert(res == VK_SUCCESS);
            items.push_back(item);
        }
    }

    // Create pool to make sure allocations must be out of this memory type.
    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = ownAllocInfo.memoryType;
    poolCreateInfo.blockSize = BLOCK_SIZE;

    VmaPool hPool;
    res = vmaCreatePool(hAllocator, &poolCreateInfo, &hPool);
    assert(res == VK_SUCCESS);

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
            assert(res == VK_SUCCESS);
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
        assert(res == VK_ERROR_OUT_OF_DEVICE_MEMORY);
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
        assert(res == VK_SUCCESS);
        // Margin is preserved also at the beginning of a block.
        assert(allocInfo[i].offset >= VMA_DEBUG_MARGIN);

        if(i == BUF_COUNT - 1)
        {
            // Fill with data.
            assert(allocInfo[i].pMappedData != nullptr);
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
            assert(allocInfo[i].offset >= allocInfo[i - 1].offset + VMA_DEBUG_MARGIN);
        }
    }

    VkResult res = vmaCheckCorruption(g_hAllocator, UINT32_MAX);
    assert(res == VK_SUCCESS);

    // Destroy all buffers.
    for(size_t i = BUF_COUNT; i--; )
    {
        vmaDestroyBuffer(g_hAllocator, buffers[i].Buffer, buffers[i].Allocation);
    }
}
#endif

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
        res = vkCreateBuffer(g_hDevice, &bufferInfo, nullptr, &dummyBuffer);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(g_hDevice, dummyBuffer, &memReq);
        memoryTypeBits = memReq.memoryTypeBits;

        vkDestroyBuffer(g_hDevice, dummyBuffer, nullptr);
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
    assert(res == VK_SUCCESS);

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
        assert(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Make sure that another allocation would fail.
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        assert(res == VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    // Validate that no buffer is lost. Also check that they are not mapped.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        assert(allocInfo.deviceMemory != VK_NULL_HANDLE);
        assert(allocInfo.pMappedData == nullptr);
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
                    assert(res == VK_SUCCESS);
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
        assert(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Validate that no buffer is lost.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        assert(allocInfo.deviceMemory != VK_NULL_HANDLE);
    }
    
    // Next frame.
    vmaSetCurrentFrameIndex(g_hAllocator, 2);

    // Allocate another BUF_COUNT buffers.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        BufItem item;
        res = vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocInfo, &item.Buf, &item.Alloc, nullptr);
        assert(res == VK_SUCCESS);
        items.push_back(item);
    }

    // Make sure the first BUF_COUNT is lost. Delete them.
    for(size_t i = 0; i < BUF_COUNT; ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        assert(allocInfo.deviceMemory == VK_NULL_HANDLE);
        vmaDestroyBuffer(g_hAllocator, items[i].Buf, items[i].Alloc);
    }
    items.erase(items.begin(), items.begin() + BUF_COUNT);

    // Validate that no buffer is lost.
    for(size_t i = 0; i < items.size(); ++i)
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(g_hAllocator, items[i].Alloc, &allocInfo);
        assert(allocInfo.deviceMemory != VK_NULL_HANDLE);
    }

    // Free one item.
    vmaDestroyBuffer(g_hAllocator, items.back().Buf, items.back().Alloc);
    items.pop_back();

    // Validate statistics.
    {
        VmaPoolStats poolStats = {};
        vmaGetPoolStats(g_hAllocator, pool, &poolStats);
        assert(poolStats.allocationCount == items.size());
        assert(poolStats.size = BUF_COUNT * BUF_SIZE);
        assert(poolStats.unusedRangeCount == 1);
        assert(poolStats.unusedRangeSizeMax == BUF_SIZE);
        assert(poolStats.unusedSize == BUF_SIZE);
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
        assert(res == VK_SUCCESS);
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
        assert(res == VK_SUCCESS);
        assert(defragmentationStats.deviceMemoryBlocksFreed == 2);
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
        assert(res == VK_SUCCESS);
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
    assert(lostCount == 2);

    // Make another call. Now 0 should be lost.
    vmaMakePoolAllocationsLost(g_hAllocator, pool, &lostCount);
    assert(lostCount == 0);

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
        assert(res == VK_ERROR_OUT_OF_DEVICE_MEMORY && alloc == nullptr);
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
    assert(res == VK_SUCCESS);

    VmaAllocationCreateInfo bufAllocCreateInfo = {};
    res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &bufAllocCreateInfo.pool);
    assert(res == VK_SUCCESS);

    // Create one persistently mapped buffer to keep memory of this block mapped,
    // so that pointer to mapped data will remain (more or less...) valid even
    // after destruction of other allocations.
    
    bufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VkBuffer firstBuf;
    VmaAllocation firstAlloc;
    res = vmaCreateBuffer(g_hAllocator, &bufInfo, &bufAllocCreateInfo, &firstBuf, &firstAlloc, nullptr);
    assert(res == VK_SUCCESS);

    // Test buffers.

    for(uint32_t i = 0; i < 2; ++i)
    {
        const bool persistentlyMapped = i == 0;
        bufAllocCreateInfo.flags = persistentlyMapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
        VkBuffer buf;
        VmaAllocation alloc;
        VmaAllocationInfo allocInfo;
        res = vmaCreateBuffer(g_hAllocator, &bufInfo, &bufAllocCreateInfo, &buf, &alloc, &allocInfo);
        assert(res == VK_SUCCESS);

        void* pMappedData;
        if(!persistentlyMapped)
        {
            res = vmaMapMemory(g_hAllocator, alloc, &pMappedData);
            assert(res == VK_SUCCESS);
        }
        else
        {
            pMappedData = allocInfo.pMappedData;
        }

        // Validate initialized content
        bool valid = ValidatePattern(pMappedData, BUF_SIZE, 0xDC);
        assert(valid);

        if(!persistentlyMapped)
        {
            vmaUnmapMemory(g_hAllocator, alloc);
        }

        vmaDestroyBuffer(g_hAllocator, buf, alloc);

        // Validate freed content
        valid = ValidatePattern(pMappedData, BUF_SIZE, 0xEF);
        assert(valid);
    }

    vmaDestroyBuffer(g_hAllocator, firstBuf, firstAlloc);
    vmaDestroyPool(g_hAllocator, bufAllocCreateInfo.pool);
}

static void TestPool_Benchmark(
    PoolTestResult& outResult,
    const PoolTestConfig& config)
{
    assert(config.ThreadCount > 0);

    RandomNumberGenerator mainRand{config.RandSeed};

    uint32_t allocationSizeProbabilitySum = std::accumulate(
        config.AllocationSizes.begin(),
        config.AllocationSizes.end(),
        0u,
        [](uint32_t sum, const AllocationSize& allocSize) {
            return sum + allocSize.Probability;
        });

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

    uint32_t bufferMemoryTypeBits = UINT32_MAX;
    {
        VkBuffer dummyBuffer;
        VkResult res = vkCreateBuffer(g_hDevice, &bufferInfo, nullptr, &dummyBuffer);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(g_hDevice, dummyBuffer, &memReq);
        bufferMemoryTypeBits = memReq.memoryTypeBits;

        vkDestroyBuffer(g_hDevice, dummyBuffer, nullptr);
    }

    uint32_t imageMemoryTypeBits = UINT32_MAX;
    {
        VkImage dummyImage;
        VkResult res = vkCreateImage(g_hDevice, &imageInfo, nullptr, &dummyImage);
        assert(res == VK_SUCCESS);

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(g_hDevice, dummyImage, &memReq);
        imageMemoryTypeBits = memReq.memoryTypeBits;

        vkDestroyImage(g_hDevice, dummyImage, nullptr);
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
        assert(0);

    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = 0;
    poolCreateInfo.minBlockCount = 1;
    poolCreateInfo.maxBlockCount = 1;
    poolCreateInfo.blockSize = config.PoolSize;
    poolCreateInfo.frameInUseCount = 1;

    VmaAllocationCreateInfo dummyAllocCreateInfo = {};
    dummyAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaFindMemoryTypeIndex(g_hAllocator, memoryTypeBits, &dummyAllocCreateInfo, &poolCreateInfo.memoryTypeIndex);

    VmaPool pool;
    VkResult res = vmaCreatePool(g_hAllocator, &poolCreateInfo, &pool);
    assert(res == VK_SUCCESS);

    // Start time measurement - after creating pool and initializing data structures.
    time_point timeBeg = std::chrono::high_resolution_clock::now();

    ////////////////////////////////////////////////////////////////////////////////
    // ThreadProc
    auto ThreadProc = [&](
        PoolTestThreadResult* outThreadResult,
        uint32_t randSeed,
        HANDLE frameStartEvent,
        HANDLE frameEndEvent) -> void
    {
        RandomNumberGenerator threadRand{randSeed};

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
            VkDeviceSize BufferSize;
            VkExtent2D ImageSize;
            VkBuffer Buf;
            VkImage Image;
            VmaAllocation Alloc;
            
            VkDeviceSize CalcSizeBytes() const
            {
                return BufferSize +
                    ImageSize.width * ImageSize.height * 4;
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
                assert(allocSize.BufferSizeMin > 0);
                assert(allocSize.ImageSizeMin == 0 && allocSize.ImageSizeMax == 0);
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
                assert(allocSize.ImageSizeMin > 0 && allocSize.ImageSizeMax > 0);
                if(allocSize.ImageSizeMax == allocSize.ImageSizeMin)
                    item.ImageSize.width = item.ImageSize.height = allocSize.ImageSizeMax;
                else
                {
                    item.ImageSize.width  = allocSize.ImageSizeMin + threadRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
                    item.ImageSize.height = allocSize.ImageSizeMin + threadRand.Generate() % (allocSize.ImageSizeMax - allocSize.ImageSizeMin);
                }
            }

            unusedItems.push_back(item);
        }

        auto Allocate = [&](Item& item) -> VkResult
        {
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.pool = pool;
            allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT |
                VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;

            if(item.BufferSize)
            {
                bufferInfo.size = item.BufferSize;
                PoolAllocationTimeRegisterObj timeRegisterObj(*outThreadResult);
                return vmaCreateBuffer(g_hAllocator, &bufferInfo, &allocCreateInfo, &item.Buf, &item.Alloc, nullptr);
            }
            else
            {
                assert(item.ImageSize.width && item.ImageSize.height);

                imageInfo.extent.width = item.ImageSize.width;
                imageInfo.extent.height = item.ImageSize.height;
                PoolAllocationTimeRegisterObj timeRegisterObj(*outThreadResult);
                return vmaCreateImage(g_hAllocator, &imageInfo, &allocCreateInfo, &item.Image, &item.Alloc, nullptr);
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
                unusedItems.push_back(usedItems[index]);
                usedItems.erase(usedItems.begin() + index);
            }

            // Determine which bufs we want to use in this frame.
            const size_t usedBufCount = (threadRand.Generate() % (config.UsedItemCountMax - config.UsedItemCountMin) + config.UsedItemCountMin)
                / config.ThreadCount;
            assert(usedBufCount < usedItems.size() + unusedItems.size());
            // Move some used to unused.
            while(usedBufCount < usedItems.size())
            {
                size_t index = threadRand.Generate() % usedItems.size();
                unusedItems.push_back(usedItems[index]);
                usedItems.erase(usedItems.begin() + index);
            }
            // Move some unused to used.
            while(usedBufCount > usedItems.size())
            {
                size_t index = threadRand.Generate() % unusedItems.size();
                usedItems.push_back(unusedItems[index]);
                unusedItems.erase(unusedItems.begin() + index);
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
                        item.Alloc = VK_NULL_HANDLE;
                        item.Buf = VK_NULL_HANDLE;
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
                            if(item.Buf)
                                vmaDestroyBuffer(g_hAllocator, item.Buf, item.Alloc);
                            else
                                vmaDestroyImage(g_hAllocator, item.Image, item.Alloc);
                            ++outThreadResult->DeallocationCount;
                        }
                        item.Alloc = VK_NULL_HANDLE;
                        item.Buf = VK_NULL_HANDLE;

                        ++outThreadResult->LostAllocationCount;
                        outThreadResult->LostAllocationTotalSize += item.CalcSizeBytes();

                        // Recreate.
                        res = Allocate(item);
                        ++outThreadResult->AllocationCount;
                        // Creation failed.
                        if(res != VK_SUCCESS)
                        {
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
            if(usedItems[i].Buf)
                vmaDestroyBuffer(g_hAllocator, usedItems[i].Buf, usedItems[i].Alloc);
            else
                vmaDestroyImage(g_hAllocator, usedItems[i].Image, usedItems[i].Alloc);
            ++outThreadResult->DeallocationCount;
        }
        for(size_t i = unusedItems.size(); i--; )
        {
            PoolDeallocationTimeRegisterObj timeRegisterOb(*outThreadResult);
            if(unusedItems[i].Buf)
                vmaDestroyBuffer(g_hAllocator, unusedItems[i].Buf, unusedItems[i].Alloc);
            else
                vmaDestroyImage(g_hAllocator, unusedItems[i].Image, unusedItems[i].Alloc);
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
    assert(config.ThreadCount <= MAXIMUM_WAIT_OBJECTS);
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
            assert(memTypeIndex != UINT32_MAX);
            VmaPoolCreateInfo poolInfo = {};
            poolInfo.memoryTypeIndex = memTypeIndex;
            res = vmaCreatePool(g_hAllocator, &poolInfo, &pool);
            assert(res == VK_SUCCESS);
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
            assert(res == VK_SUCCESS);
            assert(allocInfo.pMappedData == nullptr);
            memTypeIndex = allocInfo.memoryType;
        }
    
        // Map buffer 0.
        char* data00 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[0].Allocation, (void**)&data00);
        assert(res == VK_SUCCESS && data00 != nullptr);
        data00[0xFFFF] = data00[0];

        // Map buffer 0 second time.
        char* data01 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[0].Allocation, (void**)&data01);
        assert(res == VK_SUCCESS && data01 == data00);

        // Map buffer 1.
        char* data1 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[1].Allocation, (void**)&data1);
        assert(res == VK_SUCCESS && data1 != nullptr);
        assert(!MemoryRegionsOverlap(data00, (size_t)bufInfo.size, data1, (size_t)bufInfo.size));
        data1[0xFFFF] = data1[0];

        // Unmap buffer 0 two times.
        vmaUnmapMemory(g_hAllocator, bufferInfos[0].Allocation);
        vmaUnmapMemory(g_hAllocator, bufferInfos[0].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[0].Allocation, &allocInfo);
        assert(allocInfo.pMappedData == nullptr);

        // Unmap buffer 1.
        vmaUnmapMemory(g_hAllocator, bufferInfos[1].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[1].Allocation, &allocInfo);
        assert(allocInfo.pMappedData == nullptr);

        // Create 3rd buffer - persistently mapped.
        allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        res = vmaCreateBuffer(g_hAllocator, &bufInfo, &allocCreateInfo,
            &bufferInfos[2].Buffer, &bufferInfos[2].Allocation, &allocInfo);
        assert(res == VK_SUCCESS && allocInfo.pMappedData != nullptr);

        // Map buffer 2.
        char* data2 = nullptr;
        res = vmaMapMemory(g_hAllocator, bufferInfos[2].Allocation, (void**)&data2);
        assert(res == VK_SUCCESS && data2 == allocInfo.pMappedData);
        data2[0xFFFF] = data2[0];

        // Unmap buffer 2.
        vmaUnmapMemory(g_hAllocator, bufferInfos[2].Allocation);
        vmaGetAllocationInfo(g_hAllocator, bufferInfos[2].Allocation, &allocInfo);
        assert(allocInfo.pMappedData == data2);

        // Destroy all buffers.
        for(size_t i = 3; i--; )
            vmaDestroyBuffer(g_hAllocator, bufferInfos[i].Buffer, bufferInfos[i].Allocation);

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
            assert(memTypeIndex != UINT32_MAX);
            VmaPoolCreateInfo poolInfo = {};
            poolInfo.memoryTypeIndex = memTypeIndex;
            res = vmaCreatePool(g_hAllocator, &poolInfo, &pool);
            assert(res == VK_SUCCESS);
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
                    assert(res == VK_SUCCESS);
                    
                    if(memTypeIndex == UINT32_MAX)
                        memTypeIndex = allocInfo.memoryType;

                    char* data = nullptr;

                    if(mode == MODE::PERSISTENTLY_MAPPED)
                    {
                        data = (char*)allocInfo.pMappedData;
                        assert(data != nullptr);
                    }
                    else if(mode == MODE::MAP_FOR_MOMENT || mode == MODE::MAP_FOR_LONGER ||
                        mode == MODE::MAP_TWO_TIMES)
                    {
                        assert(data == nullptr);
                        res = vmaMapMemory(g_hAllocator, bufInfo.Allocation, (void**)&data);
                        assert(res == VK_SUCCESS && data != nullptr);

                        if(mode == MODE::MAP_TWO_TIMES)
                        {
                            char* data2 = nullptr;
                            res = vmaMapMemory(g_hAllocator, bufInfo.Allocation, (void**)&data2);
                            assert(res == VK_SUCCESS && data2 == data);
                        }
                    }
                    else if(mode == MODE::DONT_MAP)
                    {
                        assert(allocInfo.pMappedData == nullptr);
                    }
                    else
                        assert(0);

                    // Test if reading and writing from the beginning and end of mapped memory doesn't crash.
                    if(data)
                        data[0xFFFF] = data[0];

                    if(mode == MODE::MAP_FOR_MOMENT || mode == MODE::MAP_TWO_TIMES)
                    {
                        vmaUnmapMemory(g_hAllocator, bufInfo.Allocation);

                        VmaAllocationInfo allocInfo;
                        vmaGetAllocationInfo(g_hAllocator, bufInfo.Allocation, &allocInfo);
                        if(mode == MODE::MAP_FOR_MOMENT)
                            assert(allocInfo.pMappedData == nullptr);
                        else
                            assert(allocInfo.pMappedData == data);
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
                        assert(allocInfo.pMappedData == nullptr);
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
        "Code,Test,Time,"
        "Config,"
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

    time_t rawTime; time(&rawTime);
    struct tm timeInfo; localtime_s(&timeInfo, &rawTime);
    char timeStr[128];
    strftime(timeStr, _countof(timeStr), "%c", &timeInfo);

    fprintf(file,
        "%s,%s,%s,"
        "BeginBytesToAllocate=%I64u MaxBytesToAllocate=%I64u AdditionalOperationCount=%u ThreadCount=%u FreeOrder=%d,"
        "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%I64u,%I64u,%I64u\n",
        codeDescription,
        testDescription,
        timeStr,
        config.BeginBytesToAllocate, config.MaxBytesToAllocate, config.AdditionalOperationCount, config.ThreadCount, (uint32_t)config.FreeOrder,
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

    time_t rawTime; time(&rawTime);
    struct tm timeInfo; localtime_s(&timeInfo, &rawTime);
    char timeStr[128];
    strftime(timeStr, _countof(timeStr), "%c", &timeInfo);

    fprintf(file,
        "%s,%s,%s,"
        "ThreadCount=%u PoolSize=%llu FrameCount=%u TotalItemCount=%u UsedItemCount=%u...%u ItemsToMakeUnusedPercent=%u,"
        "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%I64u,%I64u,%I64u,%I64u\n",
        // General
        codeDescription,
        testDescription,
        timeStr,
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
    assert(res == VK_SUCCESS);
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

    g_MemoryAliasingWarningEnabled = false;
    PoolTestResult result = {};
    TestPool_Benchmark(result, config);
    g_MemoryAliasingWarningEnabled = true;

    WritePoolTestResult(file, "Code desc", "Test desc", config, result);
}

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
static const char* CODE_DESCRIPTION = "Foo";

static void PerformMainTests(FILE* file)
{
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
                            desc5 += " Allocate_100%";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate;
                            config.AdditionalOperationCount = 0;
                            break;
                        case 1:
                            desc5 += " Allocate_50%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate * 50 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        case 2:
                            desc5 += " Allocate_5%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate *  5 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        case 3:
                            desc5 += " Allocate_95%+Operations";
                            config.BeginBytesToAllocate = config.MaxBytesToAllocate * 95 / 100;
                            config.AdditionalOperationCount = 1024;
                            break;
                        default:
                            assert(0);
                        }

                        const char* testDescription = desc5.c_str();

                        for(size_t repeat = 0; repeat < repeatCount; ++repeat)
                        {
                            printf("%s Repeat %u\n", testDescription, (uint32_t)repeat);

                            Result result{};
                            VkResult res = MainTest(result, config);
                            assert(res == VK_SUCCESS);
                            WriteMainTestResult(file, CODE_DESCRIPTION, testDescription, config, result);
                        }
                    }
                }
            }
        }
    }
}

static void PerformPoolTests(FILE* file)
{
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
                            printf("%s Repeat %u\n", testDescription, (uint32_t)repeat);

                            PoolTestResult result{};
                            g_MemoryAliasingWarningEnabled = false;
                            TestPool_Benchmark(result, config);
                            g_MemoryAliasingWarningEnabled = true;
                            WritePoolTestResult(file, CODE_DESCRIPTION, testDescription, config, result);
                        }
                    }
                }
            }
        }
    }
}

void Test()
{
    wprintf(L"TESTING:\n");

    // TEMP tests

    // # Simple tests

    TestBasics();
#if VMA_DEBUG_MARGIN
    TestDebugMargin();
#else
    TestPool_SameSize();
    TestHeapSizeLimit();
#endif
#if VMA_DEBUG_INITIALIZE_ALLOCATIONS
    TestAllocationsInitialization();
#endif
    TestMapping();
    TestMappingMultithreaded();
    TestDefragmentationSimple();
    TestDefragmentationFull();

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

    wprintf(L"Done.\n");
}

#endif // #ifdef _WIN32
