//
// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "VmaUsage.h"
#include "Common.h"
#include "Constants.h"
#include <unordered_map>
#include <map>
#include <algorithm>

static VERBOSITY g_Verbosity = VERBOSITY::DEFAULT;

static const uint32_t VULKAN_API_VERSION = VK_API_VERSION_1_1;

namespace DetailedStats
{
    
struct Flag
{
    uint32_t setCount = 0;

    void PostValue(bool v)
    {
        if(v)
        {
            ++setCount;
        }
    }

    void Print(uint32_t totalCount) const
    {
        if(setCount)
        {
            printf(" %u (%.2f%%)\n", setCount, (double)setCount * 100.0 / (double)totalCount);
        }
        else
        {
            printf(" 0\n");
        }
    }
};

struct Enum
{
    Enum(size_t itemCount, const char* const* itemNames, const uint32_t* itemValues = nullptr) :
        m_ItemCount(itemCount),
        m_ItemNames(itemNames),
        m_ItemValues(itemValues)
    {
    }

    void PostValue(uint32_t v)
    {
        if(v < _countof(m_BaseCount))
        {
            ++m_BaseCount[v];
        }
        else
        {
            auto it = m_ExtendedCount.find(v);
            if(it != m_ExtendedCount.end())
            {
                ++it->second;
            }
            else
            {
                m_ExtendedCount.insert(std::make_pair(v, 1u));
            }
        }
    }

    void Print(uint32_t totalCount) const
    {
        if(totalCount &&
            (!m_ExtendedCount.empty() || std::count_if(m_BaseCount, m_BaseCount + _countof(m_BaseCount), [](uint32_t v) { return v > 0; })))
        {
            printf("\n");
            
            for(size_t i = 0; i < _countof(m_BaseCount); ++i)
            {
                const uint32_t currCount = m_BaseCount[i];
                if(currCount)
                {
                    PrintItem((uint32_t)i, currCount, totalCount);
                }
            }

            for(const auto& it : m_ExtendedCount)
            {
                PrintItem(it.first, it.second, totalCount);
            }
        }
        else
        {
            printf(" 0\n");
        }
    }

private:
    const size_t m_ItemCount;
    const char* const* const m_ItemNames;
    const uint32_t* const m_ItemValues;

    uint32_t m_BaseCount[32] = {};
    std::map<uint32_t, uint32_t> m_ExtendedCount;

    void PrintItem(uint32_t value, uint32_t count, uint32_t totalCount) const
    {
        size_t itemIndex = m_ItemCount;
        if(m_ItemValues)
        {
            for(itemIndex = 0; itemIndex < m_ItemCount; ++itemIndex)
            {
                if(m_ItemValues[itemIndex] == value)
                {
                    break;
                }
            }
        }
        else
        {
            if(value < m_ItemCount)
            {
                itemIndex = value;
            }
        }

        if(itemIndex < m_ItemCount)
        {
            printf("        %s: ", m_ItemNames[itemIndex]);
        }
        else
        {
            printf("        0x%X: ", value);
        }

        printf("%u (%.2f%%)\n", count, (double)count * 100.0 / (double)totalCount);
    }
};

struct FlagSet
{
    uint32_t count[32] = {};

    FlagSet(size_t count, const char* const* names, const uint32_t* values = nullptr) :
        m_Count(count),
        m_Names(names),
        m_Values(values)
    {
    }

    void PostValue(uint32_t v)
    {
        for(size_t i = 0; i < 32; ++i)
        {
            if((v & (1u << i)) != 0)
            {
                ++count[i];
            }
        }
    }

    void Print(uint32_t totalCount) const
    {
        if(totalCount &&
            std::count_if(count, count + _countof(count), [](uint32_t v) { return v > 0; }))
        {
            printf("\n");
            for(uint32_t bitIndex = 0; bitIndex < 32; ++bitIndex)
            {
                const uint32_t currCount = count[bitIndex];
                if(currCount)
                {
                    size_t itemIndex = m_Count;
                    if(m_Values)
                    {
                        for(itemIndex = 0; itemIndex < m_Count; ++itemIndex)
                        {
                            if(m_Values[itemIndex] == (1u << bitIndex))
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        if(bitIndex < m_Count)
                        {
                            itemIndex = bitIndex;
                        }
                    }

                    if(itemIndex < m_Count)
                    {
                        printf("        %s: ", m_Names[itemIndex]);
                    }
                    else
                    {
                        printf("        0x%X: ", 1u << bitIndex);
                    }

                    printf("%u (%.2f%%)\n", currCount, (double)currCount * 100.0 / (double)totalCount);
                }
            }
        }
        else
        {
            printf(" 0\n");
        }
    }

private:
    const size_t m_Count;
    const char* const* const m_Names;
    const uint32_t* const m_Values;
};

// T should be unsigned int
template<typename T>
struct MinMaxAvg
{
    T min = std::numeric_limits<T>::max();
    T max = 0;
    T sum = T();

    void PostValue(T v)
    {
        this->min = std::min(this->min, v);
        this->max = std::max(this->max, v);
        sum += v;
    }

    void Print(uint32_t totalCount) const
    {
        if(totalCount && sum > T())
        {
            if(this->min == this->max)
            {
                printf(" %llu\n", (uint64_t)this->max);
            }
            else
            {
                printf("\n        Min: %llu\n        Max: %llu\n        Avg: %llu\n",
                    (uint64_t)this->min,
                    (uint64_t)this->max,
                    round_div<uint64_t>(this->sum, totalCount));
            }
        }
        else
        {
            printf(" 0\n");
        }
    }
};

template<typename T>
struct BitMask
{
    uint32_t zeroCount = 0;
    uint32_t maxCount = 0;

    void PostValue(T v)
    {
        if(v)
        {
            if(v == std::numeric_limits<T>::max())
            {
                ++maxCount;
            }
        }
        else
        {
            ++zeroCount;
        }
    }

    void Print(uint32_t totalCount) const
    {
        if(totalCount > 0 && zeroCount < totalCount)
        {
            const uint32_t otherCount = totalCount - (zeroCount + maxCount);
            
            printf("\n        0: %u (%.2f%%)\n        Max: %u (%.2f%%)\n        Other: %u (%.2f%%)\n",
                zeroCount,  (double)zeroCount  * 100.0 / (double)totalCount,
                maxCount,   (double)maxCount   * 100.0 / (double)totalCount,
                otherCount, (double)otherCount * 100.0 / (double)totalCount);
        }
        else
        {
            printf(" 0\n");
        }
    }
};

struct CountPerMemType
{
    uint32_t count[VK_MAX_MEMORY_TYPES] = {};

    void PostValue(uint32_t v)
    {
        for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
        {
            if((v & (1u << i)) != 0)
            {
                ++count[i];
            }
        }
    }

    void Print(uint32_t totalCount) const
    {
        if(totalCount)
        {
            printf("\n");
            for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
            {
                if(count[i])
                {
                    printf("        %u: %u (%.2f%%)\n", i, count[i],
                        (double)count[i] * 100.0 / (double)totalCount);
                }
            }
        }
        else
        {
            printf(" 0\n");
        }
    }
};

struct StructureStats
{
    uint32_t totalCount = 0;
};

#define PRINT_FIELD(name) \
    printf("    " #name ":"); \
    (name).Print(totalCount);
#define PRINT_FIELD_NAMED(name, nameStr) \
    printf("    " nameStr ":"); \
    (name).Print(totalCount);

struct VmaPoolCreateInfoStats : public StructureStats
{
    CountPerMemType memoryTypeIndex;
    FlagSet flags;
    MinMaxAvg<VkDeviceSize> blockSize;
    MinMaxAvg<size_t> minBlockCount;
    MinMaxAvg<size_t> maxBlockCount;
    Flag minMaxBlockCountEqual;
    MinMaxAvg<uint32_t> frameInUseCount;

    VmaPoolCreateInfoStats() :
        flags(VMA_POOL_CREATE_FLAG_COUNT, VMA_POOL_CREATE_FLAG_NAMES, VMA_POOL_CREATE_FLAG_VALUES)
    {
    }

    void PostValue(const VmaPoolCreateInfo& v)
    {
        ++totalCount;

        memoryTypeIndex.PostValue(v.memoryTypeIndex);
        flags.PostValue(v.flags);
        blockSize.PostValue(v.blockSize);
        minBlockCount.PostValue(v.minBlockCount);
        maxBlockCount.PostValue(v.maxBlockCount);
        minMaxBlockCountEqual.PostValue(v.minBlockCount == v.maxBlockCount);
        frameInUseCount.PostValue(v.frameInUseCount);
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("VmaPoolCreateInfo (%u):\n", totalCount);

        PRINT_FIELD(memoryTypeIndex);
        PRINT_FIELD(flags);
        PRINT_FIELD(blockSize);
        PRINT_FIELD(minBlockCount);
        PRINT_FIELD(maxBlockCount);
        PRINT_FIELD_NAMED(minMaxBlockCountEqual, "minBlockCount == maxBlockCount");
        PRINT_FIELD(frameInUseCount);
    }
};

struct VkBufferCreateInfoStats : public StructureStats
{
    FlagSet flags;
    MinMaxAvg<VkDeviceSize> size;
    FlagSet usage;
    Enum sharingMode;

    VkBufferCreateInfoStats() :
        flags(VK_BUFFER_CREATE_FLAG_COUNT, VK_BUFFER_CREATE_FLAG_NAMES, VK_BUFFER_CREATE_FLAG_VALUES),
        usage(VK_BUFFER_USAGE_FLAG_COUNT, VK_BUFFER_USAGE_FLAG_NAMES, VK_BUFFER_USAGE_FLAG_VALUES),
        sharingMode(VK_SHARING_MODE_COUNT, VK_SHARING_MODE_NAMES)
    {
    }

    void PostValue(const VkBufferCreateInfo& v)
    {
        ++totalCount;

        flags.PostValue(v.flags);
        size.PostValue(v.size);
        usage.PostValue(v.usage);
        sharingMode.PostValue(v.sharingMode);
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("VkBufferCreateInfo (%u):\n", totalCount);

        PRINT_FIELD(flags);
        PRINT_FIELD(size);
        PRINT_FIELD(usage);
        PRINT_FIELD(sharingMode);
    }
};

struct VkImageCreateInfoStats : public StructureStats
{
    FlagSet flags;
    Enum imageType;
    Enum format;
    MinMaxAvg<uint32_t> width, height, depth, mipLevels, arrayLayers;
    Flag depthGreaterThanOne, mipLevelsGreaterThanOne, arrayLayersGreaterThanOne;
    Enum samples;
    Enum tiling;
    FlagSet usage;
    Enum sharingMode;
    Enum initialLayout;

    VkImageCreateInfoStats() :
        flags(VK_IMAGE_CREATE_FLAG_COUNT, VK_IMAGE_CREATE_FLAG_NAMES, VK_IMAGE_CREATE_FLAG_VALUES),
        imageType(VK_IMAGE_TYPE_COUNT, VK_IMAGE_TYPE_NAMES),
        format(VK_FORMAT_COUNT, VK_FORMAT_NAMES, VK_FORMAT_VALUES),
        samples(VK_SAMPLE_COUNT_COUNT, VK_SAMPLE_COUNT_NAMES, VK_SAMPLE_COUNT_VALUES),
        tiling(VK_IMAGE_TILING_COUNT, VK_IMAGE_TILING_NAMES),
        usage(VK_IMAGE_USAGE_FLAG_COUNT, VK_IMAGE_USAGE_FLAG_NAMES, VK_IMAGE_USAGE_FLAG_VALUES),
        sharingMode(VK_SHARING_MODE_COUNT, VK_SHARING_MODE_NAMES),
        initialLayout(VK_IMAGE_LAYOUT_COUNT, VK_IMAGE_LAYOUT_NAMES, VK_IMAGE_LAYOUT_VALUES)
    {
    }

    void PostValue(const VkImageCreateInfo& v)
    {
        ++totalCount;

        flags.PostValue(v.flags);
        imageType.PostValue(v.imageType);
        format.PostValue(v.format);
        width.PostValue(v.extent.width);
        height.PostValue(v.extent.height);
        depth.PostValue(v.extent.depth);
        mipLevels.PostValue(v.mipLevels);
        arrayLayers.PostValue(v.arrayLayers);
        depthGreaterThanOne.PostValue(v.extent.depth > 1);
        mipLevelsGreaterThanOne.PostValue(v.mipLevels > 1);
        arrayLayersGreaterThanOne.PostValue(v.arrayLayers > 1);
        samples.PostValue(v.samples);
        tiling.PostValue(v.tiling);
        usage.PostValue(v.usage);
        sharingMode.PostValue(v.sharingMode);
        initialLayout.PostValue(v.initialLayout);
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("VkImageCreateInfo (%u):\n", totalCount);

        PRINT_FIELD(flags);
        PRINT_FIELD(imageType);
        PRINT_FIELD(format);
        PRINT_FIELD(width);
        PRINT_FIELD(height);
        PRINT_FIELD(depth);
        PRINT_FIELD(mipLevels);
        PRINT_FIELD(arrayLayers);
        PRINT_FIELD_NAMED(depthGreaterThanOne, "depth > 1");
        PRINT_FIELD_NAMED(mipLevelsGreaterThanOne, "mipLevels > 1");
        PRINT_FIELD_NAMED(arrayLayersGreaterThanOne, "arrayLayers > 1");
        PRINT_FIELD(samples);
        PRINT_FIELD(tiling);
        PRINT_FIELD(usage);
        PRINT_FIELD(sharingMode);
        PRINT_FIELD(initialLayout);
    }
};

struct VmaAllocationCreateInfoStats : public StructureStats
{
    FlagSet flags;
    Enum usage;
    FlagSet requiredFlags, preferredFlags;
    Flag requiredFlagsNotZero, preferredFlagsNotZero;
    BitMask<uint32_t> memoryTypeBits;
    Flag poolNotNull;
    Flag userDataNotNull;

    VmaAllocationCreateInfoStats() :
        flags(VMA_ALLOCATION_CREATE_FLAG_COUNT, VMA_ALLOCATION_CREATE_FLAG_NAMES, VMA_ALLOCATION_CREATE_FLAG_VALUES),
        usage(VMA_MEMORY_USAGE_COUNT, VMA_MEMORY_USAGE_NAMES),
        requiredFlags(VK_MEMORY_PROPERTY_FLAG_COUNT, VK_MEMORY_PROPERTY_FLAG_NAMES, VK_MEMORY_PROPERTY_FLAG_VALUES),
        preferredFlags(VK_MEMORY_PROPERTY_FLAG_COUNT, VK_MEMORY_PROPERTY_FLAG_NAMES, VK_MEMORY_PROPERTY_FLAG_VALUES)
    {
    }

    void PostValue(const VmaAllocationCreateInfo& v, size_t count = 1)
    {
        totalCount += (uint32_t)count;

        for(size_t i = 0; i < count; ++i)
        {
            flags.PostValue(v.flags);
            usage.PostValue(v.usage);
            requiredFlags.PostValue(v.requiredFlags);
            preferredFlags.PostValue(v.preferredFlags);
            requiredFlagsNotZero.PostValue(v.requiredFlags != 0);
            preferredFlagsNotZero.PostValue(v.preferredFlags != 0);
            memoryTypeBits.PostValue(v.memoryTypeBits);
            poolNotNull.PostValue(v.pool != VK_NULL_HANDLE);
            userDataNotNull.PostValue(v.pUserData != nullptr);
        }
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("VmaAllocationCreateInfo (%u):\n", totalCount);

        PRINT_FIELD(flags);
        PRINT_FIELD(usage);
        PRINT_FIELD(requiredFlags);
        PRINT_FIELD(preferredFlags);
        PRINT_FIELD_NAMED(requiredFlagsNotZero, "requiredFlags != 0");
        PRINT_FIELD_NAMED(preferredFlagsNotZero, "preferredFlags != 0");
        PRINT_FIELD(memoryTypeBits);
        PRINT_FIELD_NAMED(poolNotNull, "pool != VK_NULL_HANDLE");
        PRINT_FIELD_NAMED(userDataNotNull, "pUserData != nullptr");
    }
};

struct VmaAllocateMemoryPagesStats : public StructureStats
{
    MinMaxAvg<size_t> allocationCount;

    void PostValue(size_t allocationCount)
    {
        this->allocationCount.PostValue(allocationCount);
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("vmaAllocateMemoryPages (%u):\n", totalCount);

        PRINT_FIELD(allocationCount);
    }
};

struct VmaDefragmentationInfo2Stats : public StructureStats
{
    BitMask<VkDeviceSize> maxCpuBytesToMove;
    BitMask<uint32_t> maxCpuAllocationsToMove;
    BitMask<VkDeviceSize> maxGpuBytesToMove;
    BitMask<uint32_t> maxGpuAllocationsToMove;
    Flag commandBufferNotNull;
    MinMaxAvg<uint32_t> allocationCount;
    Flag allocationCountNotZero;
    MinMaxAvg<uint32_t> poolCount;
    Flag poolCountNotZero;

    void PostValue(const VmaDefragmentationInfo2& info)
    {
        ++totalCount;

        maxCpuBytesToMove.PostValue(info.maxCpuBytesToMove);
        maxCpuAllocationsToMove.PostValue(info.maxCpuAllocationsToMove);
        maxGpuBytesToMove.PostValue(info.maxGpuBytesToMove);
        maxGpuAllocationsToMove.PostValue(info.maxGpuAllocationsToMove);
        commandBufferNotNull.PostValue(info.commandBuffer != VK_NULL_HANDLE);
        allocationCount.PostValue(info.allocationCount);
        allocationCountNotZero.PostValue(info.allocationCount != 0);
        poolCount.PostValue(info.poolCount);
        poolCountNotZero.PostValue(info.poolCount != 0);
    }

    void Print() const
    {
        if(totalCount == 0)
        {
            return;
        }

        printf("VmaDefragmentationInfo2 (%u):\n", totalCount);

        PRINT_FIELD(maxCpuBytesToMove);
        PRINT_FIELD(maxCpuAllocationsToMove);
        PRINT_FIELD(maxGpuBytesToMove);
        PRINT_FIELD(maxGpuAllocationsToMove);
        PRINT_FIELD_NAMED(commandBufferNotNull, "commandBuffer != VK_NULL_HANDLE");
        PRINT_FIELD(allocationCount);
        PRINT_FIELD_NAMED(allocationCountNotZero, "allocationCount > 0");
        PRINT_FIELD(poolCount);
        PRINT_FIELD_NAMED(poolCountNotZero, "poolCount > 0");
    }
};

#undef PRINT_FIELD_NAMED
#undef PRINT_FIELD

} // namespace DetailedStats

// Set this to false to disable deleting leaked VmaAllocation, VmaPool objects
// and let VMA report asserts about them.
static const bool CLEANUP_LEAKED_OBJECTS = true;

static std::string g_FilePath;
// Most significant 16 bits are major version, least significant 16 bits are minor version.
static uint32_t g_FileVersion;

inline uint32_t MakeVersion(uint32_t major, uint32_t minor) { return (major << 16) | minor; }
inline uint32_t GetVersionMajor(uint32_t version) { return version >> 16; }
inline uint32_t GetVersionMinor(uint32_t version) { return version & 0xFFFF; }

static size_t g_IterationCount = 1;
static uint32_t g_PhysicalDeviceIndex = 0;
static RangeSequence<size_t> g_LineRanges;
static bool g_UserDataEnabled = true;
static bool g_MemStatsEnabled = false;
VULKAN_EXTENSION_REQUEST g_VK_LAYER_KHRONOS_validation           = VULKAN_EXTENSION_REQUEST::DEFAULT;
VULKAN_EXTENSION_REQUEST g_VK_EXT_memory_budget_request          = VULKAN_EXTENSION_REQUEST::DEFAULT;
VULKAN_EXTENSION_REQUEST g_VK_AMD_device_coherent_memory_request = VULKAN_EXTENSION_REQUEST::DEFAULT;

struct StatsAfterLineEntry
{
    size_t line;
    bool detailed;

    bool operator<(const StatsAfterLineEntry& rhs) const { return line < rhs.line; }
    bool operator==(const StatsAfterLineEntry& rhs) const { return line == rhs.line; }
};
static std::vector<StatsAfterLineEntry> g_DumpStatsAfterLine;
static std::vector<size_t> g_DefragmentAfterLine;
static uint32_t g_DefragmentationFlags = 0;
static size_t g_DumpStatsAfterLineNextIndex = 0;
static size_t g_DefragmentAfterLineNextIndex = 0;

static bool ValidateFileVersion()
{
    if(GetVersionMajor(g_FileVersion) == 1 &&
        GetVersionMinor(g_FileVersion) <= 8)
    {
        return true;
    }

    return false;
}

static bool ParseFileVersion(const StrRange& s)
{
    CsvSplit csvSplit;
    csvSplit.Set(s, 2);
    uint32_t major, minor;
    if(csvSplit.GetCount() == 2 &&
        StrRangeToUint(csvSplit.GetRange(0), major) &&
        StrRangeToUint(csvSplit.GetRange(1), minor))
    {
        g_FileVersion = (major << 16) | minor;
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
// class Statistics

class Statistics
{
public:
    static uint32_t BufferUsageToClass(uint32_t usage);
    static uint32_t ImageUsageToClass(uint32_t usage);

    Statistics();
    ~Statistics();
    void Init(uint32_t memHeapCount, uint32_t memTypeCount);
    void PrintDeviceMemStats() const;
    void PrintMemStats() const;
    void PrintDetailedStats() const;

    const size_t* GetFunctionCallCount() const { return m_FunctionCallCount; }
    size_t GetImageCreationCount(uint32_t imgClass) const { return m_ImageCreationCount[imgClass]; }
    size_t GetLinearImageCreationCount() const { return m_LinearImageCreationCount; }
    size_t GetBufferCreationCount(uint32_t bufClass) const { return m_BufferCreationCount[bufClass]; }
    size_t GetAllocationCreationCount() const { return (size_t)m_VmaAllocationCreateInfo.totalCount + m_CreateLostAllocationCount; }
    size_t GetPoolCreationCount() const { return m_VmaPoolCreateInfo.totalCount; }
    size_t GetBufferCreationCount() const { return (size_t)m_VkBufferCreateInfo.totalCount; }

    void RegisterFunctionCall(VMA_FUNCTION func);
    void RegisterCreateImage(const VkImageCreateInfo& info);
    void RegisterCreateBuffer(const VkBufferCreateInfo& info);
    void RegisterCreatePool(const VmaPoolCreateInfo& info);
    void RegisterCreateAllocation(const VmaAllocationCreateInfo& info, size_t allocCount = 1);
    void RegisterCreateLostAllocation() { ++m_CreateLostAllocationCount; }
    void RegisterAllocateMemoryPages(size_t allocCount) { m_VmaAllocateMemoryPages.PostValue(allocCount); }
    void RegisterDefragmentation(const VmaDefragmentationInfo2& info);

    void RegisterDeviceMemoryAllocation(uint32_t memoryType, VkDeviceSize size);
    void UpdateMemStats(const VmaStats& currStats);

private:
    uint32_t m_MemHeapCount = 0;
    uint32_t m_MemTypeCount = 0;

    size_t m_FunctionCallCount[(size_t)VMA_FUNCTION::Count] = {};
    size_t m_ImageCreationCount[4] = { };
    size_t m_LinearImageCreationCount = 0;
    size_t m_BufferCreationCount[4] = { };

    struct DeviceMemStatInfo
    {
        size_t allocationCount;
        VkDeviceSize allocationTotalSize;
    };
    struct DeviceMemStats
    {
        DeviceMemStatInfo memoryType[VK_MAX_MEMORY_TYPES];
        DeviceMemStatInfo total;
    } m_DeviceMemStats;
    
    // Structure similar to VmaStatInfo, but not the same.
    struct MemStatInfo
    {
        uint32_t blockCount;
        uint32_t allocationCount;
        uint32_t unusedRangeCount;
        VkDeviceSize usedBytes;
        VkDeviceSize unusedBytes;
        VkDeviceSize totalBytes;
    };
    struct MemStats
    {
        MemStatInfo memoryType[VK_MAX_MEMORY_TYPES];
        MemStatInfo memoryHeap[VK_MAX_MEMORY_HEAPS];
        MemStatInfo total;
    } m_PeakMemStats;

    DetailedStats::VmaPoolCreateInfoStats m_VmaPoolCreateInfo;
    DetailedStats::VkBufferCreateInfoStats m_VkBufferCreateInfo;
    DetailedStats::VkImageCreateInfoStats m_VkImageCreateInfo;
    DetailedStats::VmaAllocationCreateInfoStats m_VmaAllocationCreateInfo;
    size_t m_CreateLostAllocationCount = 0;
    DetailedStats::VmaAllocateMemoryPagesStats m_VmaAllocateMemoryPages;
    DetailedStats::VmaDefragmentationInfo2Stats m_VmaDefragmentationInfo2;

    void UpdateMemStatInfo(MemStatInfo& inoutPeakInfo, const VmaStatInfo& currInfo);
    static void PrintMemStatInfo(const MemStatInfo& info);
};

// Hack for global AllocateDeviceMemoryCallback.
static Statistics* g_Statistics;

static void VKAPI_CALL AllocateDeviceMemoryCallback(
    VmaAllocator      allocator,
    uint32_t          memoryType,
    VkDeviceMemory    memory,
    VkDeviceSize      size,
    void*             pUserData)
{
    g_Statistics->RegisterDeviceMemoryAllocation(memoryType, size);
}

/// Callback function called before vkFreeMemory.
static void VKAPI_CALL FreeDeviceMemoryCallback(
    VmaAllocator      allocator,
    uint32_t          memoryType,
    VkDeviceMemory    memory,
    VkDeviceSize      size,
    void*             pUserData)
{
    // Nothing.
}

uint32_t Statistics::BufferUsageToClass(uint32_t usage)
{
    // Buffer is used as source of data for fixed-function stage of graphics pipeline.
    // It's indirect, vertex, or index buffer.
    if ((usage & (VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT)) != 0)
    {
        return 0;
    }
    // Buffer is accessed by shaders for load/store/atomic.
    // Aka "UAV"
    else if ((usage & (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) != 0)
    {
        return 1;
    }
    // Buffer is accessed by shaders for reading uniform data.
    // Aka "constant buffer"
    else if ((usage & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
    VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)) != 0)
    {
        return 2;
    }
    // Any other type of buffer.
    // Notice that VK_BUFFER_USAGE_TRANSFER_SRC_BIT and VK_BUFFER_USAGE_TRANSFER_DST_BIT
    // flags are intentionally ignored.
    else
    {
        return 3;
    }
}

uint32_t Statistics::ImageUsageToClass(uint32_t usage)
{
    // Image is used as depth/stencil "texture/surface".
    if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
    {
        return 0;
    }
    // Image is used as other type of attachment.
    // Aka "render target"
    else if ((usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) != 0)
    {
        return 1;
    }
    // Image is accessed by shaders for sampling.
    // Aka "texture"
    else if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
    {
        return 2;
    }
    // Any other type of image.
    // Notice that VK_IMAGE_USAGE_TRANSFER_SRC_BIT and VK_IMAGE_USAGE_TRANSFER_DST_BIT
    // flags are intentionally ignored.
    else
    {
        return 3;
    }
}

Statistics::Statistics()
{
    ZeroMemory(&m_DeviceMemStats, sizeof(m_DeviceMemStats));
    ZeroMemory(&m_PeakMemStats, sizeof(m_PeakMemStats));

    assert(g_Statistics == nullptr);
    g_Statistics = this;
}

Statistics::~Statistics()
{
    assert(g_Statistics == this);
    g_Statistics = nullptr;
}

void Statistics::Init(uint32_t memHeapCount, uint32_t memTypeCount)
{
    m_MemHeapCount = memHeapCount;
    m_MemTypeCount = memTypeCount;
}

void Statistics::PrintDeviceMemStats() const
{
    printf("Successful device memory allocations:\n");
    printf("    Total: count = %zu, total size = %llu\n",
        m_DeviceMemStats.total.allocationCount, m_DeviceMemStats.total.allocationTotalSize);
    for(uint32_t i = 0; i < m_MemTypeCount; ++i)
    {
        printf("    Memory type %u: count = %zu, total size = %llu\n",
            i, m_DeviceMemStats.memoryType[i].allocationCount, m_DeviceMemStats.memoryType[i].allocationTotalSize);
    }
}

void Statistics::PrintMemStats() const
{
    printf("Memory statistics:\n");

    printf("    Total:\n");
    PrintMemStatInfo(m_PeakMemStats.total);

    for(uint32_t i = 0; i < m_MemHeapCount; ++i)
    {
        const MemStatInfo& info = m_PeakMemStats.memoryHeap[i];
        if(info.blockCount > 0 || info.totalBytes > 0)
        {
            printf("    Heap %u:\n", i);
            PrintMemStatInfo(info);
        }
    }

    for(uint32_t i = 0; i < m_MemTypeCount; ++i)
    {
        const MemStatInfo& info = m_PeakMemStats.memoryType[i];
        if(info.blockCount > 0 || info.totalBytes > 0)
        {
            printf("    Type %u:\n", i);
            PrintMemStatInfo(info);
        }
    }
}

void Statistics::PrintDetailedStats() const
{
    m_VmaPoolCreateInfo.Print();
    m_VmaAllocationCreateInfo.Print();
    m_VmaAllocateMemoryPages.Print();
    m_VkBufferCreateInfo.Print();
    m_VkImageCreateInfo.Print();
    m_VmaDefragmentationInfo2.Print();
}

void Statistics::RegisterFunctionCall(VMA_FUNCTION func)
{
    ++m_FunctionCallCount[(size_t)func];
}

void Statistics::RegisterCreateImage(const VkImageCreateInfo& info)
{
    if(info.tiling == VK_IMAGE_TILING_LINEAR)
        ++m_LinearImageCreationCount;
    else
    {
        const uint32_t imgClass = ImageUsageToClass(info.usage);
        ++m_ImageCreationCount[imgClass];
    }

    m_VkImageCreateInfo.PostValue(info);
}

void Statistics::RegisterCreateBuffer(const VkBufferCreateInfo& info)
{
    const uint32_t bufClass = BufferUsageToClass(info.usage);
    ++m_BufferCreationCount[bufClass];

    m_VkBufferCreateInfo.PostValue(info);
}

void Statistics::RegisterCreatePool(const VmaPoolCreateInfo& info)
{
    m_VmaPoolCreateInfo.PostValue(info);
}

void Statistics::RegisterCreateAllocation(const VmaAllocationCreateInfo& info, size_t allocCount)
{
    m_VmaAllocationCreateInfo.PostValue(info, allocCount);
}

void Statistics::RegisterDefragmentation(const VmaDefragmentationInfo2& info)
{
    m_VmaDefragmentationInfo2.PostValue(info);
}

void Statistics::UpdateMemStats(const VmaStats& currStats)
{
    UpdateMemStatInfo(m_PeakMemStats.total, currStats.total);
    
    for(uint32_t i = 0; i < m_MemHeapCount; ++i)
    {
        UpdateMemStatInfo(m_PeakMemStats.memoryHeap[i], currStats.memoryHeap[i]);
    }

    for(uint32_t i = 0; i < m_MemTypeCount; ++i)
    {
        UpdateMemStatInfo(m_PeakMemStats.memoryType[i], currStats.memoryType[i]);
    }
}

void Statistics::RegisterDeviceMemoryAllocation(uint32_t memoryType, VkDeviceSize size)
{
    ++m_DeviceMemStats.total.allocationCount;
    m_DeviceMemStats.total.allocationTotalSize += size;

    ++m_DeviceMemStats.memoryType[memoryType].allocationCount;
    m_DeviceMemStats.memoryType[memoryType].allocationTotalSize += size;
}

void Statistics::UpdateMemStatInfo(MemStatInfo& inoutPeakInfo, const VmaStatInfo& currInfo)
{
#define SET_PEAK(inoutDst, src) \
    if((src) > (inoutDst)) \
    { \
        (inoutDst) = (src); \
    }

    SET_PEAK(inoutPeakInfo.blockCount, currInfo.blockCount);
    SET_PEAK(inoutPeakInfo.allocationCount, currInfo.allocationCount);
    SET_PEAK(inoutPeakInfo.unusedRangeCount, currInfo.unusedRangeCount);
    SET_PEAK(inoutPeakInfo.usedBytes, currInfo.usedBytes);
    SET_PEAK(inoutPeakInfo.unusedBytes, currInfo.unusedBytes);
    SET_PEAK(inoutPeakInfo.totalBytes, currInfo.usedBytes + currInfo.unusedBytes);

#undef SET_PEAK
}

void Statistics::PrintMemStatInfo(const MemStatInfo& info)
{
    printf("        Peak blocks %u, allocations %u, unused ranges %u\n",
        info.blockCount,
        info.allocationCount,
        info.unusedRangeCount);
    printf("        Peak total bytes %llu, used bytes %llu, unused bytes %llu\n",
        info.totalBytes,
        info.usedBytes,
        info.unusedBytes);
}

////////////////////////////////////////////////////////////////////////////////
// class ConfigurationParser

class ConfigurationParser
{
public:
    ConfigurationParser();
    
    bool Parse(LineSplit& lineSplit);

    void Compare(
        const VkPhysicalDeviceProperties& currDevProps,
        const VkPhysicalDeviceMemoryProperties& currMemProps,
        uint32_t vulkanApiVersion,
        bool currMemoryBudgetEnabled);

private:
    enum class OPTION
    {
        VulkanApiVersion,
        PhysicalDevice_apiVersion,
        PhysicalDevice_driverVersion,
        PhysicalDevice_vendorID,
        PhysicalDevice_deviceID,
        PhysicalDevice_deviceType,
        PhysicalDevice_deviceName,
        PhysicalDeviceLimits_maxMemoryAllocationCount,
        PhysicalDeviceLimits_bufferImageGranularity,
        PhysicalDeviceLimits_nonCoherentAtomSize,
        Extension_VK_KHR_dedicated_allocation,
        Extension_VK_KHR_bind_memory2,
        Extension_VK_EXT_memory_budget,
        Extension_VK_AMD_device_coherent_memory,
        Macro_VMA_DEBUG_ALWAYS_DEDICATED_MEMORY,
        Macro_VMA_DEBUG_ALIGNMENT,
        Macro_VMA_DEBUG_MARGIN,
        Macro_VMA_DEBUG_INITIALIZE_ALLOCATIONS,
        Macro_VMA_DEBUG_DETECT_CORRUPTION,
        Macro_VMA_DEBUG_GLOBAL_MUTEX,
        Macro_VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY,
        Macro_VMA_SMALL_HEAP_MAX_SIZE,
        Macro_VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE,
        Count
    };

    std::vector<bool> m_OptionSet;
    std::vector<std::string> m_OptionValue;
    VkPhysicalDeviceMemoryProperties m_MemProps;

    bool m_WarningHeaderPrinted = false;

    void SetOption(
        size_t lineNumber,
        OPTION option,
        const StrRange& str);
    void EnsureWarningHeader();
    void CompareOption(VERBOSITY minVerbosity, const char* name,
        OPTION option, uint32_t currValue);
    void CompareOption(VERBOSITY minVerbosity, const char* name,
        OPTION option, uint64_t currValue);
    void CompareOption(VERBOSITY minVerbosity, const char* name,
        OPTION option, bool currValue);
    void CompareOption(VERBOSITY minVerbosity, const char* name,
        OPTION option, const char* currValue);
    void CompareMemProps(
        const VkPhysicalDeviceMemoryProperties& currMemProps);
};

ConfigurationParser::ConfigurationParser() :
    m_OptionSet((size_t)OPTION::Count),
    m_OptionValue((size_t)OPTION::Count)
{
    ZeroMemory(&m_MemProps, sizeof(m_MemProps));
}

bool ConfigurationParser::Parse(LineSplit& lineSplit)
{
    for(auto& it : m_OptionSet)
    {
        it = false;
    }
    for(auto& it : m_OptionValue)
    {
        it.clear();
    }

    StrRange line;

    if(!lineSplit.GetNextLine(line) && !StrRangeEq(line, "Config,Begin"))
    {
        return false;
    }

    CsvSplit csvSplit;
    while(lineSplit.GetNextLine(line))
    {
        if(StrRangeEq(line, "Config,End"))
        {
            break;
        }

        const size_t currLineNumber = lineSplit.GetNextLineIndex();

        csvSplit.Set(line);
        if(csvSplit.GetCount() == 0)
        {
            return false;
        }

        const StrRange optionName = csvSplit.GetRange(0);
        if(StrRangeEq(optionName, "VulkanApiVersion"))
        {
            SetOption(currLineNumber, OPTION::VulkanApiVersion, StrRange{csvSplit.GetRange(1).beg, csvSplit.GetRange(2).end});
        }
        else if(StrRangeEq(optionName, "PhysicalDevice"))
        {
            if(csvSplit.GetCount() >= 3)
            {
                const StrRange subOptionName = csvSplit.GetRange(1);
                if(StrRangeEq(subOptionName, "apiVersion"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_apiVersion, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "driverVersion"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_driverVersion, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "vendorID"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_vendorID, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "deviceID"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_deviceID, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "deviceType"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_deviceType, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "deviceName"))
                    SetOption(currLineNumber, OPTION::PhysicalDevice_deviceName, StrRange(csvSplit.GetRange(2).beg, line.end));
                else
                    printf("Line %zu: Unrecognized configuration option.\n", currLineNumber);
            }
            else
                printf("Line %zu: Too few columns.\n", currLineNumber);
        }
        else if(StrRangeEq(optionName, "PhysicalDeviceLimits"))
        {
            if(csvSplit.GetCount() >= 3)
            {
                const StrRange subOptionName = csvSplit.GetRange(1);
                if(StrRangeEq(subOptionName, "maxMemoryAllocationCount"))
                    SetOption(currLineNumber, OPTION::PhysicalDeviceLimits_maxMemoryAllocationCount, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "bufferImageGranularity"))
                    SetOption(currLineNumber, OPTION::PhysicalDeviceLimits_bufferImageGranularity, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "nonCoherentAtomSize"))
                    SetOption(currLineNumber, OPTION::PhysicalDeviceLimits_nonCoherentAtomSize, csvSplit.GetRange(2));
                else
                    printf("Line %zu: Unrecognized configuration option.\n", currLineNumber);
            }
            else
                printf("Line %zu: Too few columns.\n", currLineNumber);
        }
        else if(StrRangeEq(optionName, "Extension"))
        {
            if(csvSplit.GetCount() >= 3)
            {
                const StrRange subOptionName = csvSplit.GetRange(1);
                if(StrRangeEq(subOptionName, "VK_KHR_dedicated_allocation"))
                {
                    // Ignore because this extension is promoted to Vulkan 1.1.
                }
                else if(StrRangeEq(subOptionName, "VK_KHR_bind_memory2"))
                    SetOption(currLineNumber, OPTION::Extension_VK_KHR_bind_memory2, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VK_EXT_memory_budget"))
                    SetOption(currLineNumber, OPTION::Extension_VK_EXT_memory_budget, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VK_AMD_device_coherent_memory"))
                    SetOption(currLineNumber, OPTION::Extension_VK_AMD_device_coherent_memory, csvSplit.GetRange(2));
                else
                    printf("Line %zu: Unrecognized configuration option.\n", currLineNumber);
            }
            else
                printf("Line %zu: Too few columns.\n", currLineNumber);
        }
        else if(StrRangeEq(optionName, "Macro"))
        {
            if(csvSplit.GetCount() >= 3)
            {
                const StrRange subOptionName = csvSplit.GetRange(1);
                if(StrRangeEq(subOptionName, "VMA_DEBUG_ALWAYS_DEDICATED_MEMORY"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_ALWAYS_DEDICATED_MEMORY, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_ALIGNMENT"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_ALIGNMENT, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_MARGIN"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_MARGIN, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_INITIALIZE_ALLOCATIONS"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_INITIALIZE_ALLOCATIONS, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_DETECT_CORRUPTION"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_DETECT_CORRUPTION, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_GLOBAL_MUTEX"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_GLOBAL_MUTEX, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_SMALL_HEAP_MAX_SIZE"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_SMALL_HEAP_MAX_SIZE, csvSplit.GetRange(2));
                else if(StrRangeEq(subOptionName, "VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE"))
                    SetOption(currLineNumber, OPTION::Macro_VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE, csvSplit.GetRange(2));
                else
                    printf("Line %zu: Unrecognized configuration option.\n", currLineNumber);
            }
            else
                printf("Line %zu: Too few columns.\n", currLineNumber);
        }
        else if(StrRangeEq(optionName, "PhysicalDeviceMemory"))
        {
            uint32_t value = 0;
            if(csvSplit.GetCount() == 3 && StrRangeEq(csvSplit.GetRange(1), "HeapCount") &&
                StrRangeToUint(csvSplit.GetRange(2), value))
            {
                m_MemProps.memoryHeapCount = value;
            }
            else if(csvSplit.GetCount() == 3 && StrRangeEq(csvSplit.GetRange(1), "TypeCount") &&
                StrRangeToUint(csvSplit.GetRange(2), value))
            {
                m_MemProps.memoryTypeCount = value;
            }
            else if(csvSplit.GetCount() == 5 && StrRangeEq(csvSplit.GetRange(1), "Heap") &&
                StrRangeToUint(csvSplit.GetRange(2), value) &&
                value < m_MemProps.memoryHeapCount)
            {
                if(StrRangeEq(csvSplit.GetRange(3), "size") &&
                    StrRangeToUint(csvSplit.GetRange(4), m_MemProps.memoryHeaps[value].size))
                {
                     // Parsed.
                }
                else if(StrRangeEq(csvSplit.GetRange(3), "flags") &&
                    StrRangeToUint(csvSplit.GetRange(4), m_MemProps.memoryHeaps[value].flags))
                {
                     // Parsed.
                }
                else
                    printf("Line %zu: Invalid configuration option.\n", currLineNumber);
            }
            else if(csvSplit.GetCount() == 5 && StrRangeEq(csvSplit.GetRange(1), "Type") &&
                StrRangeToUint(csvSplit.GetRange(2), value) &&
                value < m_MemProps.memoryTypeCount)
            {
                if(StrRangeEq(csvSplit.GetRange(3), "heapIndex") &&
                    StrRangeToUint(csvSplit.GetRange(4), m_MemProps.memoryTypes[value].heapIndex))
                {
                     // Parsed.
                }
                else if(StrRangeEq(csvSplit.GetRange(3), "propertyFlags") &&
                    StrRangeToUint(csvSplit.GetRange(4), m_MemProps.memoryTypes[value].propertyFlags))
                {
                     // Parsed.
                }
                else
                    printf("Line %zu: Invalid configuration option.\n", currLineNumber);
            }
            else
                printf("Line %zu: Invalid configuration option.\n", currLineNumber);
        }
        else
            printf("Line %zu: Unrecognized configuration option.\n", currLineNumber);
    }

    return true;
}

void ConfigurationParser::Compare(
    const VkPhysicalDeviceProperties& currDevProps,
    const VkPhysicalDeviceMemoryProperties& currMemProps,
    uint32_t vulkanApiVersion,
    bool currMemoryBudgetEnabled)
{
    char vulkanApiVersionStr[32];
    sprintf_s(vulkanApiVersionStr, "%u,%u", VK_VERSION_MAJOR(vulkanApiVersion), VK_VERSION_MINOR(vulkanApiVersion));
    CompareOption(VERBOSITY::DEFAULT, "VulkanApiVersion",
        OPTION::VulkanApiVersion, vulkanApiVersionStr);

    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice apiVersion",
        OPTION::PhysicalDevice_apiVersion, currDevProps.apiVersion);
    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice driverVersion",
        OPTION::PhysicalDevice_driverVersion, currDevProps.driverVersion);
    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice vendorID",
        OPTION::PhysicalDevice_vendorID, currDevProps.vendorID);
    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice deviceID",
        OPTION::PhysicalDevice_deviceID, currDevProps.deviceID);
    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice deviceType",
        OPTION::PhysicalDevice_deviceType, (uint32_t)currDevProps.deviceType);
    CompareOption(VERBOSITY::MAXIMUM, "PhysicalDevice deviceName",
        OPTION::PhysicalDevice_deviceName, currDevProps.deviceName);

    CompareOption(VERBOSITY::DEFAULT, "PhysicalDeviceLimits maxMemoryAllocationCount",
        OPTION::PhysicalDeviceLimits_maxMemoryAllocationCount, currDevProps.limits.maxMemoryAllocationCount);
    CompareOption(VERBOSITY::DEFAULT, "PhysicalDeviceLimits bufferImageGranularity",
        OPTION::PhysicalDeviceLimits_bufferImageGranularity, currDevProps.limits.bufferImageGranularity);
    CompareOption(VERBOSITY::DEFAULT, "PhysicalDeviceLimits nonCoherentAtomSize",
        OPTION::PhysicalDeviceLimits_nonCoherentAtomSize, currDevProps.limits.nonCoherentAtomSize);

    CompareMemProps(currMemProps);
}

void ConfigurationParser::SetOption(
    size_t lineNumber,
    OPTION option,
    const StrRange& str)
{
    if(m_OptionSet[(size_t)option])
    {
        printf("Line %zu: Option already specified.\n" ,lineNumber);
    }

    m_OptionSet[(size_t)option] = true;

    std::string val;
    str.to_str(val);
    m_OptionValue[(size_t)option] = std::move(val);
}

void ConfigurationParser::EnsureWarningHeader()
{
    if(!m_WarningHeaderPrinted)
    {
        printf("WARNING: Following configuration parameters don't match:\n");
        m_WarningHeaderPrinted = true;
    }
}

void ConfigurationParser::CompareOption(VERBOSITY minVerbosity, const char* name,
    OPTION option, uint32_t currValue)
{
    if(m_OptionSet[(size_t)option] &&
        g_Verbosity >= minVerbosity)
    {
        uint32_t origValue;
        if(StrRangeToUint(StrRange(m_OptionValue[(size_t)option]), origValue))
        {
            if(origValue != currValue)
            {
                EnsureWarningHeader();
                printf("    %s: original %u, current %u\n", name, origValue, currValue);
            }
        }
    }
}

void ConfigurationParser::CompareOption(VERBOSITY minVerbosity, const char* name,
    OPTION option, uint64_t currValue)
{
    if(m_OptionSet[(size_t)option] &&
        g_Verbosity >= minVerbosity)
    {
        uint64_t origValue;
        if(StrRangeToUint(StrRange(m_OptionValue[(size_t)option]), origValue))
        {
            if(origValue != currValue)
            {
                EnsureWarningHeader();
                printf("    %s: original %llu, current %llu\n", name, origValue, currValue);
            }
        }
    }
}

void ConfigurationParser::CompareOption(VERBOSITY minVerbosity, const char* name,
    OPTION option, bool currValue)
{
    if(m_OptionSet[(size_t)option] &&
        g_Verbosity >= minVerbosity)
    {
        bool origValue;
        if(StrRangeToBool(StrRange(m_OptionValue[(size_t)option]), origValue))
        {
            if(origValue != currValue)
            {
                EnsureWarningHeader();
                printf("    %s: original %u, current %u\n", name,
                    origValue ? 1 : 0,
                    currValue ? 1 : 0);
            }
        }
    }
}

void ConfigurationParser::CompareOption(VERBOSITY minVerbosity, const char* name,
    OPTION option, const char* currValue)
{
    if(m_OptionSet[(size_t)option] &&
        g_Verbosity >= minVerbosity)
    {
        const std::string& origValue = m_OptionValue[(size_t)option];
        if(origValue != currValue)
        {
            EnsureWarningHeader();
            printf("    %s: original \"%s\", current \"%s\"\n", name, origValue.c_str(), currValue);
        }
    }
}

void ConfigurationParser::CompareMemProps(
    const VkPhysicalDeviceMemoryProperties& currMemProps)
{
    if(g_Verbosity < VERBOSITY::DEFAULT)
    {
        return;
    }

    bool memoryMatch =
        currMemProps.memoryHeapCount == m_MemProps.memoryHeapCount &&
        currMemProps.memoryTypeCount == m_MemProps.memoryTypeCount;

    for(uint32_t i = 0; memoryMatch && i < currMemProps.memoryHeapCount; ++i)
    {
        memoryMatch =
            currMemProps.memoryHeaps[i].flags == m_MemProps.memoryHeaps[i].flags;
    }
    for(uint32_t i = 0; memoryMatch && i < currMemProps.memoryTypeCount; ++i)
    {
        memoryMatch =
            currMemProps.memoryTypes[i].heapIndex == m_MemProps.memoryTypes[i].heapIndex &&
            currMemProps.memoryTypes[i].propertyFlags == m_MemProps.memoryTypes[i].propertyFlags;
    }

    if(memoryMatch && g_Verbosity == VERBOSITY::MAXIMUM)
    {
        bool memorySizeMatch = true;
        for(uint32_t i = 0; memorySizeMatch && i < currMemProps.memoryHeapCount; ++i)
        {
            memorySizeMatch =
                currMemProps.memoryHeaps[i].size == m_MemProps.memoryHeaps[i].size;
        }

        if(!memorySizeMatch)
        {
            printf("WARNING: Sizes of original memory heaps are different from current ones.\n");
        }
    }
    else
    {
        printf("WARNING: Layout of original memory heaps and types is different from current one.\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
// class Player

static const char* const VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

static VkBool32 VKAPI_PTR MyDebugReportCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData)
{
    assert(pCallbackData && pCallbackData->pMessageIdName && pCallbackData->pMessage);
    printf("%s \xBA %s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
    return VK_FALSE;
}

static bool IsLayerSupported(const VkLayerProperties* pProps, size_t propCount, const char* pLayerName)
{
    const VkLayerProperties* propsEnd = pProps + propCount;
    return std::find_if(
        pProps,
        propsEnd,
        [pLayerName](const VkLayerProperties& prop) -> bool {
            return strcmp(pLayerName, prop.layerName) == 0;
        }) != propsEnd;
}

static const size_t FIRST_PARAM_INDEX = 4;

static void InitVulkanFeatures(
    VkPhysicalDeviceFeatures& outFeatures,
    const VkPhysicalDeviceFeatures& supportedFeatures)
{
    ZeroMemory(&outFeatures, sizeof(outFeatures));

    // Enable something what may interact with memory/buffer/image support.

    outFeatures.fullDrawIndexUint32 = supportedFeatures.fullDrawIndexUint32;
    outFeatures.imageCubeArray = supportedFeatures.imageCubeArray;
    outFeatures.geometryShader = supportedFeatures.geometryShader;
    outFeatures.tessellationShader = supportedFeatures.tessellationShader;
    outFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    outFeatures.textureCompressionETC2 = supportedFeatures.textureCompressionETC2;
    outFeatures.textureCompressionASTC_LDR = supportedFeatures.textureCompressionASTC_LDR;
    outFeatures.textureCompressionBC = supportedFeatures.textureCompressionBC;
}

class Player
{
public:
    Player();
    int Init();
    ~Player();

    void ApplyConfig(ConfigurationParser& configParser);
    void ExecuteLine(size_t lineNumber, const StrRange& line);
    void DumpStats(const char* fileNameFormat, size_t lineNumber, bool detailed);
    void Defragment();

    void PrintStats();

private:
    static const size_t MAX_WARNINGS_TO_SHOW = 64;

    size_t m_WarningCount = 0;
    bool m_AllocateForBufferImageWarningIssued = false;

    VkInstance m_VulkanInstance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t m_TransferQueueFamilyIndex = UINT32_MAX;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_TransferQueue = VK_NULL_HANDLE;
    VmaAllocator m_Allocator = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    bool m_MemoryBudgetEnabled = false;
    const VkPhysicalDeviceProperties* m_DevProps = nullptr;
    const VkPhysicalDeviceMemoryProperties* m_MemProps = nullptr;

    PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;
    VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;

    uint32_t m_VmaFrameIndex = 0;

    // Any of these handles null can mean it was created in original but couldn't be created now.
    struct Pool
    {
        VmaPool pool;
    };
    struct Allocation
    {
        uint32_t allocationFlags = 0;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
    };
    std::unordered_map<uint64_t, Pool> m_Pools;
    std::unordered_map<uint64_t, Allocation> m_Allocations;
    std::unordered_map<uint64_t, VmaDefragmentationContext> m_DefragmentationContexts;

    struct Thread
    {
        uint32_t callCount;
    };
    std::unordered_map<uint32_t, Thread> m_Threads;

    // Copy of column [1] from previously parsed line.
    std::string m_LastLineTimeStr;
    Statistics m_Stats;

    std::vector<char> m_UserDataTmpStr;

    void Destroy(const Allocation& alloc);

    // Finds VmaPool bu original pointer.
    // If origPool = null, returns true and outPool = null.
    // If failed, prints warning, returns false and outPool = null.
    bool FindPool(size_t lineNumber, uint64_t origPool, VmaPool& outPool);
    // If allocation with that origPtr already exists, prints warning and replaces it.
    void AddAllocation(size_t lineNumber, uint64_t origPtr, VkResult res, const char* functionName, Allocation&& allocDesc);

    // Increments warning counter. Returns true if warning message should be printed.
    bool IssueWarning();

    int InitVulkan();
    void FinalizeVulkan();
    void RegisterDebugCallbacks();
    void UnregisterDebugCallbacks();

    // If parmeter count doesn't match, issues warning and returns false.
    bool ValidateFunctionParameterCount(size_t lineNumber, const CsvSplit& csvSplit, size_t expectedParamCount, bool lastUnbound);

    // If failed, prints warning, returns false, and sets allocCreateInfo.pUserData to null.
    bool PrepareUserData(size_t lineNumber, uint32_t allocCreateFlags, const StrRange& userDataColumn, const StrRange& wholeLine, void*& outUserData);

    void UpdateMemStats();

    void ExecuteCreatePool(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteDestroyPool(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteSetAllocationUserData(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteCreateBuffer(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteDestroyBuffer(size_t lineNumber, const CsvSplit& csvSplit) { m_Stats.RegisterFunctionCall(VMA_FUNCTION::DestroyBuffer); DestroyAllocation(lineNumber, csvSplit, "vmaDestroyBuffer"); }
    void ExecuteCreateImage(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteDestroyImage(size_t lineNumber, const CsvSplit& csvSplit) { m_Stats.RegisterFunctionCall(VMA_FUNCTION::DestroyImage); DestroyAllocation(lineNumber, csvSplit, "vmaDestroyImage"); }
    void ExecuteFreeMemory(size_t lineNumber, const CsvSplit& csvSplit) { m_Stats.RegisterFunctionCall(VMA_FUNCTION::FreeMemory); DestroyAllocation(lineNumber, csvSplit, "vmaFreeMemory"); }
    void ExecuteFreeMemoryPages(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteCreateLostAllocation(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteAllocateMemory(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteAllocateMemoryPages(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteAllocateMemoryForBufferOrImage(size_t lineNumber, const CsvSplit& csvSplit, OBJECT_TYPE objType);
    void ExecuteMapMemory(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteUnmapMemory(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteFlushAllocation(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteInvalidateAllocation(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteTouchAllocation(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteGetAllocationInfo(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteMakePoolAllocationsLost(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteResizeAllocation(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteDefragmentationBegin(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteDefragmentationEnd(size_t lineNumber, const CsvSplit& csvSplit);
    void ExecuteSetPoolName(size_t lineNumber, const CsvSplit& csvSplit);

    void DestroyAllocation(size_t lineNumber, const CsvSplit& csvSplit, const char* functionName);

    void PrintStats(const VmaStats& stats, const char* suffix);
    void PrintStatInfo(const VmaStatInfo& info);
};

Player::Player()
{
}

int Player::Init()
{
    int result = InitVulkan();
    
    if(result == 0)
    {
        m_Stats.Init(m_MemProps->memoryHeapCount, m_MemProps->memoryTypeCount);
        UpdateMemStats();
    }
    
    return result;
}

Player::~Player()
{
    FinalizeVulkan();

    if(g_Verbosity < VERBOSITY::MAXIMUM && m_WarningCount > MAX_WARNINGS_TO_SHOW)
        printf("WARNING: %zu more warnings not shown.\n", m_WarningCount - MAX_WARNINGS_TO_SHOW);
}

void Player::ApplyConfig(ConfigurationParser& configParser)
{
    configParser.Compare(*m_DevProps, *m_MemProps,
        VULKAN_API_VERSION,
        m_MemoryBudgetEnabled);
}

void Player::ExecuteLine(size_t lineNumber, const StrRange& line)
{
    CsvSplit csvSplit;
    csvSplit.Set(line);

    if(csvSplit.GetCount() >= FIRST_PARAM_INDEX)
    {
        // Check thread ID.
        uint32_t threadId;
        if(StrRangeToUint(csvSplit.GetRange(0), threadId))
        {
            const auto it = m_Threads.find(threadId);
            if(it != m_Threads.end())
            {
                ++it->second.callCount;
            }
            else
            {
                Thread threadInfo{};
                threadInfo.callCount = 1;
                m_Threads[threadId] = threadInfo;
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Incorrect thread ID.\n", lineNumber);
            }
        }

        // Save time.
        csvSplit.GetRange(1).to_str(m_LastLineTimeStr);

        // Update VMA current frame index.
        StrRange frameIndexStr = csvSplit.GetRange(2);
        uint32_t frameIndex;
        if(StrRangeToUint(frameIndexStr, frameIndex))
        {
            if(frameIndex != m_VmaFrameIndex)
            {
                vmaSetCurrentFrameIndex(m_Allocator, frameIndex);
                m_VmaFrameIndex = frameIndex;
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Incorrect frame index.\n", lineNumber);
            }
        }

        StrRange functionName = csvSplit.GetRange(3);

        if(StrRangeEq(functionName, "vmaCreateAllocator"))
        {
            if(ValidateFunctionParameterCount(lineNumber, csvSplit, 0, false))
            {
                // Nothing.
            }
        }
        else if(StrRangeEq(functionName, "vmaDestroyAllocator"))
        {
            if(ValidateFunctionParameterCount(lineNumber, csvSplit, 0, false))
            {
                // Nothing.
            }
        }
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::CreatePool]))
            ExecuteCreatePool(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::DestroyPool]))
            ExecuteDestroyPool(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::SetAllocationUserData]))
            ExecuteSetAllocationUserData(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::CreateBuffer]))
            ExecuteCreateBuffer(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::DestroyBuffer]))
            ExecuteDestroyBuffer(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::CreateImage]))
            ExecuteCreateImage(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::DestroyImage]))
            ExecuteDestroyImage(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::FreeMemory]))
            ExecuteFreeMemory(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::FreeMemoryPages]))
            ExecuteFreeMemoryPages(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::CreateLostAllocation]))
            ExecuteCreateLostAllocation(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::AllocateMemory]))
            ExecuteAllocateMemory(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::AllocateMemoryPages]))
            ExecuteAllocateMemoryPages(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::AllocateMemoryForBuffer]))
            ExecuteAllocateMemoryForBufferOrImage(lineNumber, csvSplit, OBJECT_TYPE::BUFFER);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::AllocateMemoryForImage]))
            ExecuteAllocateMemoryForBufferOrImage(lineNumber, csvSplit, OBJECT_TYPE::IMAGE);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::MapMemory]))
            ExecuteMapMemory(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::UnmapMemory]))
            ExecuteUnmapMemory(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::FlushAllocation]))
            ExecuteFlushAllocation(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::InvalidateAllocation]))
            ExecuteInvalidateAllocation(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::TouchAllocation]))
            ExecuteTouchAllocation(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::GetAllocationInfo]))
            ExecuteGetAllocationInfo(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::MakePoolAllocationsLost]))
            ExecuteMakePoolAllocationsLost(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::ResizeAllocation]))
            ExecuteResizeAllocation(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::DefragmentationBegin]))
            ExecuteDefragmentationBegin(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::DefragmentationEnd]))
            ExecuteDefragmentationEnd(lineNumber, csvSplit);
        else if(StrRangeEq(functionName, VMA_FUNCTION_NAMES[(uint32_t)VMA_FUNCTION::SetPoolName]))
            ExecuteSetPoolName(lineNumber, csvSplit);
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Unknown function.\n", lineNumber);
            }
        }
    }
    else
    {
        if(IssueWarning())
        {
            printf("Line %zu: Too few columns.\n", lineNumber);
        }
    }
}

void Player::DumpStats(const char* fileNameFormat, size_t lineNumber, bool detailed)
{
    char* pStatsString = nullptr;
    vmaBuildStatsString(m_Allocator, &pStatsString, detailed ? VK_TRUE : VK_FALSE);

    char fileName[MAX_PATH];
    sprintf_s(fileName, fileNameFormat, lineNumber);

    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fileName, "wb");
    if(err == 0)
    {
        fwrite(pStatsString, 1, strlen(pStatsString), file);
        fclose(file);
    }
    else
    {
        printf("ERROR: Failed to write file: %s\n", fileName);
    }

    vmaFreeStatsString(m_Allocator, pStatsString);
}

void Player::Destroy(const Allocation& alloc)
{
    if(alloc.buffer)
    {
        assert(alloc.image == VK_NULL_HANDLE);
        vmaDestroyBuffer(m_Allocator, alloc.buffer, alloc.allocation);
    }
    else if(alloc.image)
    {
        vmaDestroyImage(m_Allocator, alloc.image, alloc.allocation);
    }
    else
        vmaFreeMemory(m_Allocator, alloc.allocation);
}

bool Player::FindPool(size_t lineNumber, uint64_t origPool, VmaPool& outPool)
{
    outPool = VK_NULL_HANDLE;

    if(origPool != 0)
    {
        const auto poolIt = m_Pools.find(origPool);
        if(poolIt != m_Pools.end())
        {
            outPool = poolIt->second.pool;
            return true;
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Pool %llX not found.\n", lineNumber, origPool);
            }
        }
    }

    return true;
}

void Player::AddAllocation(size_t lineNumber, uint64_t origPtr, VkResult res, const char* functionName, Allocation&& allocDesc)
{
    if(origPtr)
    {
        if(res == VK_SUCCESS)
        {
            // Originally succeeded, currently succeeded.
            // Just save pointer (done below).
        }
        else
        {
            // Originally succeeded, currently failed.
            // Print warning. Save null pointer.
            if(IssueWarning())
            {
                printf("Line %zu: %s failed (%d), while originally succeeded.\n", lineNumber, functionName, res);
            }
        }

        const auto existingIt = m_Allocations.find(origPtr);
        if(existingIt != m_Allocations.end())
        {
            if(IssueWarning())
            {
                printf("Line %zu: Allocation %llX already exists.\n", lineNumber, origPtr);
            }
        }
        m_Allocations[origPtr] = std::move(allocDesc);
    }
    else
    {
        if(res == VK_SUCCESS)
        {
            // Originally failed, currently succeeded.
            // Print warning, destroy the object.
            if(IssueWarning())
            {
                printf("Line %zu: %s succeeded, originally failed.\n", lineNumber, functionName);
            }

            Destroy(allocDesc);
        }
        else
        {
            // Originally failed, currently failed.
            // Print warning.
            if(IssueWarning())
            {
                printf("Line %zu: %s failed (%d), originally also failed.\n", lineNumber, functionName, res);
            }
        }
    }
}

bool Player::IssueWarning()
{
    if(g_Verbosity < VERBOSITY::MAXIMUM)
    {
        return m_WarningCount++ < MAX_WARNINGS_TO_SHOW;
    }
    else
    {
        ++m_WarningCount;
        return true;
    }
}

int Player::InitVulkan()
{
    if(g_Verbosity == VERBOSITY::MAXIMUM)
    {
        printf("Initializing Vulkan...\n");
    }

    uint32_t instanceLayerPropCount = 0;
    VkResult res = vkEnumerateInstanceLayerProperties(&instanceLayerPropCount, nullptr);
    assert(res == VK_SUCCESS);

    std::vector<VkLayerProperties> instanceLayerProps(instanceLayerPropCount);
    if(instanceLayerPropCount > 0)
    {
        res = vkEnumerateInstanceLayerProperties(&instanceLayerPropCount, instanceLayerProps.data());
        assert(res == VK_SUCCESS);
    }

    const bool validationLayersAvailable =
        IsLayerSupported(instanceLayerProps.data(), instanceLayerProps.size(), VALIDATION_LAYER_NAME);

    bool validationLayersEnabled = false;
    switch(g_VK_LAYER_KHRONOS_validation)
    {
    case VULKAN_EXTENSION_REQUEST::DISABLED:
        break;
    case VULKAN_EXTENSION_REQUEST::DEFAULT:
        validationLayersEnabled = validationLayersAvailable;
        break;
    case VULKAN_EXTENSION_REQUEST::ENABLED:
        validationLayersEnabled = validationLayersAvailable;
        if(!validationLayersAvailable)
        {
            printf("WARNING: %s layer cannot be enabled.\n", VALIDATION_LAYER_NAME);
        }
        break;
    default: assert(0);
    }

    uint32_t availableInstanceExtensionCount = 0;
    res = vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, nullptr);
    assert(res == VK_SUCCESS);
    std::vector<VkExtensionProperties> availableInstanceExtensions(availableInstanceExtensionCount);
    if(availableInstanceExtensionCount > 0)
    {
        res = vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, availableInstanceExtensions.data());
        assert(res == VK_SUCCESS);
    }

    std::vector<const char*> enabledInstanceExtensions;
    //enabledInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    //enabledInstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    std::vector<const char*> instanceLayers;
    if(validationLayersEnabled)
    {
        instanceLayers.push_back(VALIDATION_LAYER_NAME);
    }

    bool VK_KHR_get_physical_device_properties2_enabled = false;
    bool VK_EXT_debug_utils_enabled = false;
    for(const auto& extensionProperties : availableInstanceExtensions)
    {
        if(strcmp(extensionProperties.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
        {
            enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            VK_KHR_get_physical_device_properties2_enabled = true;
        }
        else if(strcmp(extensionProperties.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            if(validationLayersEnabled)
            {
                enabledInstanceExtensions.push_back("VK_EXT_debug_utils");
                VK_EXT_debug_utils_enabled = true;
            }
        }
    }

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "VmaReplay";
    appInfo.applicationVersion = VK_MAKE_VERSION(2, 3, 0);
    appInfo.pEngineName = "Vulkan Memory Allocator";
    appInfo.engineVersion = VK_MAKE_VERSION(2, 3, 0);
    appInfo.apiVersion = VULKAN_API_VERSION;

    VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = (uint32_t)enabledInstanceExtensions.size();
    instInfo.ppEnabledExtensionNames = enabledInstanceExtensions.data();
    instInfo.enabledLayerCount = (uint32_t)instanceLayers.size();
    instInfo.ppEnabledLayerNames = instanceLayers.data();

    res = vkCreateInstance(&instInfo, NULL, &m_VulkanInstance);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkCreateInstance failed (%d)\n", res);
        return RESULT_ERROR_VULKAN;
    }

    if(VK_EXT_debug_utils_enabled)
    {
        RegisterDebugCallbacks();
    }

    // Find physical device

    uint32_t physicalDeviceCount = 0;
    res = vkEnumeratePhysicalDevices(m_VulkanInstance, &physicalDeviceCount, nullptr);
    assert(res == VK_SUCCESS);
    if(physicalDeviceCount == 0)
    {
        printf("ERROR: No Vulkan physical devices found.\n");
        return RESULT_ERROR_VULKAN;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    res = vkEnumeratePhysicalDevices(m_VulkanInstance, &physicalDeviceCount, physicalDevices.data());
    assert(res == VK_SUCCESS);

    if(g_PhysicalDeviceIndex >= physicalDeviceCount)
    {
        printf("ERROR: Incorrect Vulkan physical device index %u. System has %u physical devices.\n",
            g_PhysicalDeviceIndex,
            physicalDeviceCount);
        return RESULT_ERROR_VULKAN;
    }

    m_PhysicalDevice = physicalDevices[0];

    // Find queue family index

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    if(queueFamilyCount)
    {
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());
        for(uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            if(queueFamilies[i].queueCount > 0)
            {
                if(m_GraphicsQueueFamilyIndex == UINT32_MAX &&
                    (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    m_GraphicsQueueFamilyIndex = i;
                }
                if(m_TransferQueueFamilyIndex == UINT32_MAX &&
                    (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
                {
                    m_TransferQueueFamilyIndex = i;
                }
            }
        }
    }
    if(m_GraphicsQueueFamilyIndex == UINT_MAX)
    {
        printf("ERROR: Couldn't find graphics queue.\n");
        return RESULT_ERROR_VULKAN;
    }
    if(m_TransferQueueFamilyIndex == UINT_MAX)
    {
        printf("ERROR: Couldn't find transfer queue.\n");
        return RESULT_ERROR_VULKAN;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &supportedFeatures);

    // Create logical device

    const float queuePriority = 1.f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo[2] = {};
    deviceQueueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo[0].queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    deviceQueueCreateInfo[0].queueCount = 1;
    deviceQueueCreateInfo[0].pQueuePriorities = &queuePriority;

    if(m_TransferQueueFamilyIndex != m_GraphicsQueueFamilyIndex)
    {
        deviceQueueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo[1].queueFamilyIndex = m_TransferQueueFamilyIndex;
        deviceQueueCreateInfo[1].queueCount = 1;
        deviceQueueCreateInfo[1].pQueuePriorities = &queuePriority;
    }

    // Enable something what may interact with memory/buffer/image support.
    VkPhysicalDeviceFeatures enabledFeatures;
    InitVulkanFeatures(enabledFeatures, supportedFeatures);

    bool VK_KHR_get_memory_requirements2_available = false;

    // Determine list of device extensions to enable.
    std::vector<const char*> enabledDeviceExtensions;
    //enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    bool memoryBudgetAvailable = false;
    {
        uint32_t propertyCount = 0;
        res = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &propertyCount, nullptr);
        assert(res == VK_SUCCESS);

        if(propertyCount)
        {
            std::vector<VkExtensionProperties> properties{propertyCount};
            res = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &propertyCount, properties.data());
            assert(res == VK_SUCCESS);

            for(uint32_t i = 0; i < propertyCount; ++i)
            {
                if(strcmp(properties[i].extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
                {
                    VK_KHR_get_memory_requirements2_available = true;
                }
                else if(strcmp(properties[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
                {
                    if(VK_KHR_get_physical_device_properties2_enabled)
                    {
                        memoryBudgetAvailable = true;
                    }
                }
            }
        }
    }

    switch(g_VK_EXT_memory_budget_request)
    {
    case VULKAN_EXTENSION_REQUEST::DISABLED:
        break;
    case VULKAN_EXTENSION_REQUEST::DEFAULT:
        m_MemoryBudgetEnabled = memoryBudgetAvailable;
        break;
    case VULKAN_EXTENSION_REQUEST::ENABLED:
        m_MemoryBudgetEnabled = memoryBudgetAvailable;
        if(!memoryBudgetAvailable)
        {
            printf("WARNING: VK_EXT_memory_budget extension cannot be enabled.\n");
        }
        break;
    default: assert(0);
    }

    if(g_VK_AMD_device_coherent_memory_request == VULKAN_EXTENSION_REQUEST::ENABLED)
    {
        printf("WARNING: AMD_device_coherent_memory requested but not currently supported by the player.\n");
    }

    if(m_MemoryBudgetEnabled)
    {
        enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = !enabledDeviceExtensions.empty() ? enabledDeviceExtensions.data() : nullptr;
    deviceCreateInfo.queueCreateInfoCount = m_TransferQueueFamilyIndex != m_GraphicsQueueFamilyIndex ? 2 : 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    res = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkCreateDevice failed (%d)\n", res);
        return RESULT_ERROR_VULKAN;
    }

    // Fetch queues
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_TransferQueueFamilyIndex, 0, &m_TransferQueue);

    // Create memory allocator

    VmaDeviceMemoryCallbacks deviceMemoryCallbacks = {};
    deviceMemoryCallbacks.pfnAllocate = AllocateDeviceMemoryCallback;
    deviceMemoryCallbacks.pfnFree = FreeDeviceMemoryCallback;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.instance = m_VulkanInstance;
    allocatorInfo.physicalDevice = m_PhysicalDevice;
    allocatorInfo.device = m_Device;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    allocatorInfo.pDeviceMemoryCallbacks = &deviceMemoryCallbacks;
    allocatorInfo.vulkanApiVersion = VULKAN_API_VERSION;

    if(m_MemoryBudgetEnabled)
    {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    }

    res = vmaCreateAllocator(&allocatorInfo, &m_Allocator);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vmaCreateAllocator failed (%d)\n", res);
        return RESULT_ERROR_VULKAN;
    }

    vmaGetPhysicalDeviceProperties(m_Allocator, &m_DevProps);
    vmaGetMemoryProperties(m_Allocator, &m_MemProps);

    // Create command pool

    VkCommandPoolCreateInfo cmdPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmdPoolCreateInfo.queueFamilyIndex = m_TransferQueueFamilyIndex;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    res = vkCreateCommandPool(m_Device, &cmdPoolCreateInfo, nullptr, &m_CommandPool);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkCreateCommandPool failed (%d)\n", res);
        return RESULT_ERROR_VULKAN;
    }

    // Create command buffer

    VkCommandBufferAllocateInfo cmdBufAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBufAllocInfo.commandBufferCount = 1;
    cmdBufAllocInfo.commandPool = m_CommandPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    res = vkAllocateCommandBuffers(m_Device, &cmdBufAllocInfo, &m_CommandBuffer);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkAllocateCommandBuffers failed (%d)\n", res);
        return RESULT_ERROR_VULKAN;
    }

    return 0;
}

void Player::FinalizeVulkan()
{
    if(!m_DefragmentationContexts.empty())
    {
        printf("WARNING: Defragmentation contexts not destroyed: %zu.\n", m_DefragmentationContexts.size());

        if(CLEANUP_LEAKED_OBJECTS)
        {
            for(const auto& it : m_DefragmentationContexts)
            {
                vmaDefragmentationEnd(m_Allocator, it.second);
            }
        }

        m_DefragmentationContexts.clear();
    }

    if(!m_Allocations.empty())
    {
        printf("WARNING: Allocations not destroyed: %zu.\n", m_Allocations.size());

        if(CLEANUP_LEAKED_OBJECTS)
        {
            for(const auto it : m_Allocations)
            {
                Destroy(it.second);
            }
        }

        m_Allocations.clear();
    }

    if(!m_Pools.empty())
    {
        printf("WARNING: Custom pools not destroyed: %zu.\n", m_Pools.size());

        if(CLEANUP_LEAKED_OBJECTS)
        {
            for(const auto it : m_Pools)
            {
                vmaDestroyPool(m_Allocator, it.second.pool);
            }
        }

        m_Pools.clear();
    }

    vkDeviceWaitIdle(m_Device);

    if(m_CommandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CommandBuffer);
        m_CommandBuffer = VK_NULL_HANDLE;
    }

    if(m_CommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }

    if(m_Allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_Allocator);
        m_Allocator = nullptr;
    }

    if(m_Device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_Device, nullptr);
        m_Device = nullptr;
    }

    UnregisterDebugCallbacks();

    if(m_VulkanInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_VulkanInstance, NULL);
        m_VulkanInstance = VK_NULL_HANDLE;
    }
}

void Player::RegisterDebugCallbacks()
{
    static const VkDebugUtilsMessageSeverityFlagsEXT DEBUG_UTILS_MESSENGER_MESSAGE_SEVERITY =
        //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    static const VkDebugUtilsMessageTypeFlagsEXT DEBUG_UTILS_MESSENGER_MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    m_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        m_VulkanInstance, "vkCreateDebugUtilsMessengerEXT");
    m_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        m_VulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
    assert(m_vkCreateDebugUtilsMessengerEXT);
    assert(m_vkDestroyDebugUtilsMessengerEXT);

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    messengerCreateInfo.messageSeverity = DEBUG_UTILS_MESSENGER_MESSAGE_SEVERITY;
    messengerCreateInfo.messageType = DEBUG_UTILS_MESSENGER_MESSAGE_TYPE;
    messengerCreateInfo.pfnUserCallback = MyDebugReportCallback;
    VkResult res = m_vkCreateDebugUtilsMessengerEXT(m_VulkanInstance, &messengerCreateInfo, nullptr, &m_DebugUtilsMessenger);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkCreateDebugUtilsMessengerEXT failed (%d)\n", res);
        m_DebugUtilsMessenger = VK_NULL_HANDLE;
    }
}

void Player::UnregisterDebugCallbacks()
{
    if(m_DebugUtilsMessenger)
    {
        m_vkDestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugUtilsMessenger, nullptr);
    }
}

void Player::Defragment()
{
    VmaStats stats;
    vmaCalculateStats(m_Allocator, &stats);
    PrintStats(stats, "before defragmentation");

    const size_t allocCount = m_Allocations.size();
    std::vector<VmaAllocation> allocations(allocCount);
    size_t notNullAllocCount = 0;
    for(const auto& it : m_Allocations)
    {
        if(it.second.allocation != VK_NULL_HANDLE)
        {
            allocations[notNullAllocCount] = it.second.allocation;
            ++notNullAllocCount;
        }
    }
    if(notNullAllocCount == 0)
    {
        printf("    Nothing to defragment.\n");
        return;
    }

    allocations.resize(notNullAllocCount);
    std::vector<VkBool32> allocationsChanged(notNullAllocCount);

    VmaDefragmentationStats defragStats = {};

    VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult res = vkBeginCommandBuffer(m_CommandBuffer, &cmdBufBeginInfo);
    if(res != VK_SUCCESS)
    {
        printf("ERROR: vkBeginCommandBuffer failed (%d)\n", res);
        return;
    }

    const time_point timeBeg = std::chrono::high_resolution_clock::now();

    VmaDefragmentationInfo2 defragInfo = {};
    defragInfo.allocationCount = (uint32_t)notNullAllocCount;
    defragInfo.pAllocations = allocations.data();
    defragInfo.pAllocationsChanged = allocationsChanged.data();
    defragInfo.maxCpuAllocationsToMove = UINT32_MAX;
    defragInfo.maxCpuBytesToMove = VK_WHOLE_SIZE;
    defragInfo.maxGpuAllocationsToMove = UINT32_MAX;
    defragInfo.maxGpuBytesToMove = VK_WHOLE_SIZE;
    defragInfo.flags = g_DefragmentationFlags;
    defragInfo.commandBuffer = m_CommandBuffer;

    VmaDefragmentationContext defragCtx = VK_NULL_HANDLE;
    res = vmaDefragmentationBegin(m_Allocator, &defragInfo, &defragStats, &defragCtx);
    
    const time_point timeAfterDefragBegin = std::chrono::high_resolution_clock::now();

    vkEndCommandBuffer(m_CommandBuffer);

    if(res >= VK_SUCCESS)
    {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;
        vkQueueSubmit(m_TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_TransferQueue);

        const time_point timeAfterGpu = std::chrono::high_resolution_clock::now();

        vmaDefragmentationEnd(m_Allocator, defragCtx);

        const time_point timeAfterDefragEnd = std::chrono::high_resolution_clock::now();

        const duration defragDurationBegin = timeAfterDefragBegin - timeBeg;
        const duration defragDurationGpu   = timeAfterGpu - timeAfterDefragBegin;
        const duration defragDurationEnd   = timeAfterDefragEnd - timeAfterGpu;

        // If anything changed.
        if(defragStats.allocationsMoved > 0)
        {
            // Go over allocation that changed and destroy their buffers and images.
            size_t i = 0;
            for(auto& it : m_Allocations)
            {
                if(allocationsChanged[i] != VK_FALSE)
                {
                    if(it.second.buffer != VK_NULL_HANDLE)
                    {
                        vkDestroyBuffer(m_Device, it.second.buffer, nullptr);
                        it.second.buffer = VK_NULL_HANDLE;
                    }
                    if(it.second.image != VK_NULL_HANDLE)
                    {
                        vkDestroyImage(m_Device, it.second.image, nullptr);
                        it.second.image = VK_NULL_HANDLE;
                    }
                }
                ++i;
            }
        }

        // Print statistics
        std::string defragDurationBeginStr;
        std::string defragDurationGpuStr;
        std::string defragDurationEndStr;
        SecondsToFriendlyStr(ToFloatSeconds(defragDurationBegin), defragDurationBeginStr);
        SecondsToFriendlyStr(ToFloatSeconds(defragDurationGpu), defragDurationGpuStr);
        SecondsToFriendlyStr(ToFloatSeconds(defragDurationEnd), defragDurationEndStr);

        printf("    Defragmentation took:\n");
        printf("        vmaDefragmentationBegin: %s\n", defragDurationBeginStr.c_str());
        printf("        GPU: %s\n", defragDurationGpuStr.c_str());
        printf("        vmaDefragmentationEnd: %s\n", defragDurationEndStr.c_str());
        printf("    VmaDefragmentationStats:\n");
        printf("        bytesMoved: %llu\n", defragStats.bytesMoved);
        printf("        bytesFreed: %llu\n", defragStats.bytesFreed);
        printf("        allocationsMoved: %u\n", defragStats.allocationsMoved);
        printf("        deviceMemoryBlocksFreed: %u\n", defragStats.deviceMemoryBlocksFreed);

        vmaCalculateStats(m_Allocator, &stats);
        PrintStats(stats, "after defragmentation");
    }
    else
    {
        printf("vmaDefragmentationBegin failed (%d).\n", res);
    }

    vkResetCommandPool(m_Device, m_CommandPool, 0);
}

void Player::PrintStats()
{
    if(g_Verbosity == VERBOSITY::MINIMUM)
    {
        return;
    }

    m_Stats.PrintDeviceMemStats();

    printf("Statistics:\n");
    if(m_Stats.GetAllocationCreationCount() > 0)
    {
        printf("    Total allocations created: %zu\n", m_Stats.GetAllocationCreationCount());
    }

    // Buffers
    if(m_Stats.GetBufferCreationCount())
    {
        printf("    Total buffers created: %zu\n", m_Stats.GetBufferCreationCount());
        if(g_Verbosity == VERBOSITY::MAXIMUM)
        {
            printf("        Class 0 (indirect/vertex/index): %zu\n", m_Stats.GetBufferCreationCount(0));
            printf("        Class 1 (storage): %zu\n", m_Stats.GetBufferCreationCount(1));
            printf("        Class 2 (uniform): %zu\n", m_Stats.GetBufferCreationCount(2));
            printf("        Class 3 (other): %zu\n", m_Stats.GetBufferCreationCount(3));
        }
    }
    
    // Images
    const size_t imageCreationCount =
        m_Stats.GetImageCreationCount(0) +
        m_Stats.GetImageCreationCount(1) +
        m_Stats.GetImageCreationCount(2) +
        m_Stats.GetImageCreationCount(3) +
        m_Stats.GetLinearImageCreationCount();
    if(imageCreationCount > 0)
    {
        printf("    Total images created: %zu\n", imageCreationCount);
        if(g_Verbosity == VERBOSITY::MAXIMUM)
        {
            printf("        Class 0 (depth/stencil): %zu\n", m_Stats.GetImageCreationCount(0));
            printf("        Class 1 (attachment): %zu\n", m_Stats.GetImageCreationCount(1));
            printf("        Class 2 (sampled): %zu\n", m_Stats.GetImageCreationCount(2));
            printf("        Class 3 (other): %zu\n", m_Stats.GetImageCreationCount(3));
            if(m_Stats.GetLinearImageCreationCount() > 0)
            {
                printf("        LINEAR tiling: %zu\n", m_Stats.GetLinearImageCreationCount());
            }
        }
    }
    
    if(m_Stats.GetPoolCreationCount() > 0)
    {
        printf("    Total custom pools created: %zu\n", m_Stats.GetPoolCreationCount());
    }

    float lastTime;
    if(!m_LastLineTimeStr.empty() && StrRangeToFloat(StrRange(m_LastLineTimeStr), lastTime))
    {
        std::string origTimeStr;
        SecondsToFriendlyStr(lastTime, origTimeStr);
        printf("    Original recording time: %s\n", origTimeStr.c_str());
    }

    // Thread statistics.
    const size_t threadCount = m_Threads.size();
    if(threadCount > 1)
    {
        uint32_t threadCallCountMax = 0;
        uint32_t threadCallCountSum = 0;
        for(const auto& it : m_Threads)
        {
            threadCallCountMax = std::max(threadCallCountMax, it.second.callCount);
            threadCallCountSum += it.second.callCount;
        }
        printf("    Threads making calls to VMA: %zu\n", threadCount);
        printf("        %.2f%% calls from most active thread.\n",
            (float)threadCallCountMax * 100.f / (float)threadCallCountSum);
    }
    else
    {
        printf("    VMA used from only one thread.\n");
    }

    // Function call count
    if(g_Verbosity == VERBOSITY::MAXIMUM)
    {
        printf("    Function call count:\n");
        const size_t* const functionCallCount = m_Stats.GetFunctionCallCount();
        for(size_t i = 0; i < (size_t)VMA_FUNCTION::Count; ++i)
        {
            if(functionCallCount[i] > 0)
            {
                printf("        %s %zu\n", VMA_FUNCTION_NAMES[i], functionCallCount[i]);
            }
        }
    }

    // Detailed stats
    if(g_Verbosity == VERBOSITY::MAXIMUM)
    {
        m_Stats.PrintDetailedStats();
    }

    if(g_MemStatsEnabled)
    {
        m_Stats.PrintMemStats();
    }
}

bool Player::ValidateFunctionParameterCount(size_t lineNumber, const CsvSplit& csvSplit, size_t expectedParamCount, bool lastUnbound)
{
    bool ok;
    if(lastUnbound)
        ok = csvSplit.GetCount() >= FIRST_PARAM_INDEX + expectedParamCount - 1;
    else
        ok = csvSplit.GetCount() == FIRST_PARAM_INDEX + expectedParamCount;

    if(!ok)
    {
        if(IssueWarning())
        {
            printf("Line %zu: Incorrect number of function parameters.\n", lineNumber);
        }
    }

    return ok;
}

bool Player::PrepareUserData(size_t lineNumber, uint32_t allocCreateFlags, const StrRange& userDataColumn, const StrRange& wholeLine, void*& outUserData)
{
    if(!g_UserDataEnabled)
    {
        outUserData = nullptr;
        return true;
    }

    // String
    if((allocCreateFlags & VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT) != 0)
    {
        const size_t len = wholeLine.end - userDataColumn.beg;
        m_UserDataTmpStr.resize(len + 1);
        memcpy(m_UserDataTmpStr.data(), userDataColumn.beg, len);
        m_UserDataTmpStr[len] = '\0';
        outUserData = m_UserDataTmpStr.data();
        return true;
    }
    // Pointer
    else
    {
        uint64_t pUserData = 0;
        if(StrRangeToPtr(userDataColumn, pUserData))
        {
            outUserData = (void*)(uintptr_t)pUserData;
            return true;
        }
    }

    if(IssueWarning())
    {
        printf("Line %zu: Invalid pUserData.\n", lineNumber);
    }
    outUserData = 0;
    return false;
}

void Player::UpdateMemStats()
{
    if(!g_MemStatsEnabled)
    {
        return;
    }

    VmaStats stats;
    vmaCalculateStats(m_Allocator, &stats);
    m_Stats.UpdateMemStats(stats);
}

void Player::ExecuteCreatePool(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::CreatePool);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 7, false))
    {
        VmaPoolCreateInfo poolCreateInfo = {};
        uint64_t origPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), poolCreateInfo.memoryTypeIndex) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), poolCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), poolCreateInfo.blockSize) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), poolCreateInfo.minBlockCount) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), poolCreateInfo.maxBlockCount) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), poolCreateInfo.frameInUseCount) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), origPtr))
        {
            m_Stats.RegisterCreatePool(poolCreateInfo);

            Pool poolDesc = {};
            VkResult res = vmaCreatePool(m_Allocator, &poolCreateInfo, &poolDesc.pool);

            if(origPtr)
            {
                if(res == VK_SUCCESS)
                {
                    // Originally succeeded, currently succeeded.
                    // Just save pointer (done below).
                }
                else
                {
                    // Originally succeeded, currently failed.
                    // Print warning. Save null pointer.
                    if(IssueWarning())
                    {
                        printf("Line %zu: vmaCreatePool failed (%d), while originally succeeded.\n", lineNumber, res);
                    }
               }

                const auto existingIt = m_Pools.find(origPtr);
                if(existingIt != m_Pools.end())
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Pool %llX already exists.\n", lineNumber, origPtr);
                    }
                }
                m_Pools[origPtr] = poolDesc;
            }
            else
            {
                if(res == VK_SUCCESS)
                {
                    // Originally failed, currently succeeded.
                    // Print warning, destroy the pool.
                    if(IssueWarning())
                    {
                        printf("Line %zu: vmaCreatePool succeeded, originally failed.\n", lineNumber);
                    }

                    vmaDestroyPool(m_Allocator, poolDesc.pool);
                }
                else
                {
                    // Originally failed, currently failed.
                    // Print warning.
                    if(IssueWarning())
                    {
                        printf("Line %zu: vmaCreatePool failed (%d), originally also failed.\n", lineNumber, res);
                    }
                }
            }

            UpdateMemStats();
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaCreatePool.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteDestroyPool(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::DestroyPool);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_Pools.find(origPtr);
                if(it != m_Pools.end())
                {
                    vmaDestroyPool(m_Allocator, it->second.pool);
                    UpdateMemStats();
                    m_Pools.erase(it);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Pool %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaDestroyPool.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteSetAllocationUserData(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::SetAllocationUserData);

    if(!g_UserDataEnabled)
    {
        return;
    }

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 2, true))
    {
        uint64_t origPtr = 0;
        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            const auto it = m_Allocations.find(origPtr);
            if(it != m_Allocations.end())
            {
                void* pUserData = nullptr;
                if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 1)
                {
                    PrepareUserData(
                        lineNumber,
                        it->second.allocationFlags,
                        csvSplit.GetRange(FIRST_PARAM_INDEX + 1),
                        csvSplit.GetLine(),
                        pUserData);
                }

                vmaSetAllocationUserData(m_Allocator, it->second.allocation, pUserData);
            }
            else
            {
                if(IssueWarning())
                {
                    printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaSetAllocationUserData.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteCreateBuffer(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::CreateBuffer);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 12, true))
    {
        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        VmaAllocationCreateInfo allocCreateInfo = {};
        uint64_t origPool = 0;
        uint64_t origPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), bufCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), bufCreateInfo.size) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), bufCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), (uint32_t&)bufCreateInfo.sharingMode) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), allocCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), (uint32_t&)allocCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), allocCreateInfo.requiredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), allocCreateInfo.preferredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), allocCreateInfo.memoryTypeBits) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 9), origPool) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 10), origPtr))
        {
            FindPool(lineNumber, origPool, allocCreateInfo.pool);

            if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 11)
            {
                PrepareUserData(
                    lineNumber,
                    allocCreateInfo.flags,
                    csvSplit.GetRange(FIRST_PARAM_INDEX + 11),
                    csvSplit.GetLine(),
                    allocCreateInfo.pUserData);
            }

            m_Stats.RegisterCreateBuffer(bufCreateInfo);
            m_Stats.RegisterCreateAllocation(allocCreateInfo);

            // Forcing VK_SHARING_MODE_EXCLUSIVE because we use only one queue anyway.
            bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            Allocation allocDesc = { };
            allocDesc.allocationFlags = allocCreateInfo.flags;
            VkResult res = vmaCreateBuffer(m_Allocator, &bufCreateInfo, &allocCreateInfo, &allocDesc.buffer, &allocDesc.allocation, nullptr);
            UpdateMemStats();
            AddAllocation(lineNumber, origPtr, res, "vmaCreateBuffer", std::move(allocDesc));
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaCreateBuffer.\n", lineNumber);
            }
        }
    }
}

void Player::DestroyAllocation(size_t lineNumber, const CsvSplit& csvSplit, const char* functionName)
{
    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origAllocPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origAllocPtr))
        {
            if(origAllocPtr != 0)
            {
                const auto it = m_Allocations.find(origAllocPtr);
                if(it != m_Allocations.end())
                {
                    Destroy(it->second);
                    UpdateMemStats();
                    m_Allocations.erase(it);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origAllocPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for %s.\n", lineNumber, functionName);
            }
        }
    }
}

void Player::PrintStats(const VmaStats& stats, const char* suffix)
{
    printf("    VmaStats %s:\n", suffix);
    printf("        total:\n");
    PrintStatInfo(stats.total);

    if(g_Verbosity == VERBOSITY::MAXIMUM)
    {
        for(uint32_t i = 0; i < m_MemProps->memoryHeapCount; ++i)
        {
            printf("        memoryHeap[%u]:\n", i);
            PrintStatInfo(stats.memoryHeap[i]);
        }
        for(uint32_t i = 0; i < m_MemProps->memoryTypeCount; ++i)
        {
            printf("        memoryType[%u]:\n", i);
            PrintStatInfo(stats.memoryType[i]);
        }
    }
}

void Player::PrintStatInfo(const VmaStatInfo& info)
{
    printf("            blockCount: %u\n", info.blockCount);
    printf("            allocationCount: %u\n", info.allocationCount);
    printf("            unusedRangeCount: %u\n", info.unusedRangeCount);
    printf("            usedBytes: %llu\n", info.usedBytes);
    printf("            unusedBytes: %llu\n", info.unusedBytes);
    printf("            allocationSizeMin: %llu\n", info.allocationSizeMin);
    printf("            allocationSizeAvg: %llu\n", info.allocationSizeAvg);
    printf("            allocationSizeMax: %llu\n", info.allocationSizeMax);
    printf("            unusedRangeSizeMin: %llu\n", info.unusedRangeSizeMin);
    printf("            unusedRangeSizeAvg: %llu\n", info.unusedRangeSizeAvg);
    printf("            unusedRangeSizeMax: %llu\n", info.unusedRangeSizeMax);
}

void Player::ExecuteCreateImage(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::CreateImage);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 21, true))
    {
        VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        VmaAllocationCreateInfo allocCreateInfo = {};
        uint64_t origPool = 0;
        uint64_t origPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), imageCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), (uint32_t&)imageCreateInfo.imageType) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), (uint32_t&)imageCreateInfo.format) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), imageCreateInfo.extent.width) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), imageCreateInfo.extent.height) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), imageCreateInfo.extent.depth) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), imageCreateInfo.mipLevels) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), imageCreateInfo.arrayLayers) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), (uint32_t&)imageCreateInfo.samples) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 9), (uint32_t&)imageCreateInfo.tiling) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 10), imageCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 11), (uint32_t&)imageCreateInfo.sharingMode) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 12), (uint32_t&)imageCreateInfo.initialLayout) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 13), allocCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 14), (uint32_t&)allocCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 15), allocCreateInfo.requiredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 16), allocCreateInfo.preferredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 17), allocCreateInfo.memoryTypeBits) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 18), origPool) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 19), origPtr))
        {
            FindPool(lineNumber, origPool, allocCreateInfo.pool);

            if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 20)
            {
                PrepareUserData(
                    lineNumber,
                    allocCreateInfo.flags,
                    csvSplit.GetRange(FIRST_PARAM_INDEX + 20),
                    csvSplit.GetLine(),
                    allocCreateInfo.pUserData);
            }

            m_Stats.RegisterCreateImage(imageCreateInfo);
            m_Stats.RegisterCreateAllocation(allocCreateInfo);

            // Forcing VK_SHARING_MODE_EXCLUSIVE because we use only one queue anyway.
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            Allocation allocDesc = {};
            allocDesc.allocationFlags = allocCreateInfo.flags;
            VkResult res = vmaCreateImage(m_Allocator, &imageCreateInfo, &allocCreateInfo, &allocDesc.image, &allocDesc.allocation, nullptr);
            UpdateMemStats();
            AddAllocation(lineNumber, origPtr, res, "vmaCreateImage", std::move(allocDesc));
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaCreateImage.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteFreeMemoryPages(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::FreeMemoryPages);
    
    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        std::vector<uint64_t> origAllocPtrs;
        if(StrRangeToPtrList(csvSplit.GetRange(FIRST_PARAM_INDEX), origAllocPtrs))
        {
            const size_t allocCount = origAllocPtrs.size();
            size_t notNullCount = 0;
            for(size_t i = 0; i < allocCount; ++i)
            {
                const uint64_t origAllocPtr = origAllocPtrs[i];
                if(origAllocPtr != 0)
                {
                    const auto it = m_Allocations.find(origAllocPtr);
                    if(it != m_Allocations.end())
                    {
                        Destroy(it->second);
                        m_Allocations.erase(it);
                        ++notNullCount;
                    }
                    else
                    {
                        if(IssueWarning())
                        {
                            printf("Line %zu: Allocation %llX not found.\n", lineNumber, origAllocPtr);
                        }
                    }
                }
            }
            if(notNullCount)
            {
                UpdateMemStats();
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaFreeMemoryPages.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteCreateLostAllocation(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::CreateLostAllocation);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            Allocation allocDesc = {};
            vmaCreateLostAllocation(m_Allocator, &allocDesc.allocation);
            UpdateMemStats();
            m_Stats.RegisterCreateLostAllocation();

            AddAllocation(lineNumber, origPtr, VK_SUCCESS, "vmaCreateLostAllocation", std::move(allocDesc));
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaCreateLostAllocation.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteAllocateMemory(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::AllocateMemory);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 11, true))
    {
        VkMemoryRequirements memReq = {};
        VmaAllocationCreateInfo allocCreateInfo = {};
        uint64_t origPool = 0;
        uint64_t origPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), memReq.size) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), memReq.alignment) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), memReq.memoryTypeBits) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), allocCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), (uint32_t&)allocCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), allocCreateInfo.requiredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), allocCreateInfo.preferredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), allocCreateInfo.memoryTypeBits) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), origPool) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 9), origPtr))
        {
            FindPool(lineNumber, origPool, allocCreateInfo.pool);

            if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 10)
            {
                PrepareUserData(
                    lineNumber,
                    allocCreateInfo.flags,
                    csvSplit.GetRange(FIRST_PARAM_INDEX + 10),
                    csvSplit.GetLine(),
                    allocCreateInfo.pUserData);
            }

            UpdateMemStats();
            m_Stats.RegisterCreateAllocation(allocCreateInfo);

            Allocation allocDesc = {};
            allocDesc.allocationFlags = allocCreateInfo.flags;
            VkResult res = vmaAllocateMemory(m_Allocator, &memReq, &allocCreateInfo, &allocDesc.allocation, nullptr);
            AddAllocation(lineNumber, origPtr, res, "vmaAllocateMemory", std::move(allocDesc));
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaAllocateMemory.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteAllocateMemoryPages(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::AllocateMemoryPages);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 11, true))
    {
        VkMemoryRequirements memReq = {};
        VmaAllocationCreateInfo allocCreateInfo = {};
        uint64_t origPool = 0;
        std::vector<uint64_t> origPtrs;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), memReq.size) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), memReq.alignment) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), memReq.memoryTypeBits) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), allocCreateInfo.flags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), (uint32_t&)allocCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), allocCreateInfo.requiredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), allocCreateInfo.preferredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), allocCreateInfo.memoryTypeBits) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), origPool) &&
            StrRangeToPtrList(csvSplit.GetRange(FIRST_PARAM_INDEX + 9), origPtrs))
        {
            const size_t allocCount = origPtrs.size();
            if(allocCount > 0)
            {
                FindPool(lineNumber, origPool, allocCreateInfo.pool);

                if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 10)
                {
                    PrepareUserData(
                        lineNumber,
                        allocCreateInfo.flags,
                        csvSplit.GetRange(FIRST_PARAM_INDEX + 10),
                        csvSplit.GetLine(),
                        allocCreateInfo.pUserData);
                }

                UpdateMemStats();
                m_Stats.RegisterCreateAllocation(allocCreateInfo, allocCount);
                m_Stats.RegisterAllocateMemoryPages(allocCount);

                std::vector<VmaAllocation> allocations(allocCount);

                VkResult res = vmaAllocateMemoryPages(m_Allocator, &memReq, &allocCreateInfo, allocCount, allocations.data(), nullptr);
                for(size_t i = 0; i < allocCount; ++i)
                {
                    Allocation allocDesc = {};
                    allocDesc.allocationFlags = allocCreateInfo.flags;
                    allocDesc.allocation = allocations[i];
                    AddAllocation(lineNumber, origPtrs[i], res, "vmaAllocateMemoryPages", std::move(allocDesc));
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaAllocateMemoryPages.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteAllocateMemoryForBufferOrImage(size_t lineNumber, const CsvSplit& csvSplit, OBJECT_TYPE objType)
{
    switch(objType)
    {
    case OBJECT_TYPE::BUFFER:
        m_Stats.RegisterFunctionCall(VMA_FUNCTION::AllocateMemoryForBuffer);
        break;
    case OBJECT_TYPE::IMAGE:
        m_Stats.RegisterFunctionCall(VMA_FUNCTION::AllocateMemoryForImage);
        break;
    default: assert(0);
    }

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 13, true))
    {
        VkMemoryRequirements memReq = {};
        VmaAllocationCreateInfo allocCreateInfo = {};
        bool requiresDedicatedAllocation = false;
        bool prefersDedicatedAllocation = false;
        uint64_t origPool = 0;
        uint64_t origPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), memReq.size) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), memReq.alignment) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), memReq.memoryTypeBits) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), allocCreateInfo.flags) &&
            StrRangeToBool(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), requiresDedicatedAllocation) &&
            StrRangeToBool(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), prefersDedicatedAllocation) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), (uint32_t&)allocCreateInfo.usage) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), allocCreateInfo.requiredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), allocCreateInfo.preferredFlags) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 9), allocCreateInfo.memoryTypeBits) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 10), origPool) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 11), origPtr))
        {
            FindPool(lineNumber, origPool, allocCreateInfo.pool);

            if(csvSplit.GetCount() > FIRST_PARAM_INDEX + 12)
            {
                PrepareUserData(
                    lineNumber,
                    allocCreateInfo.flags,
                    csvSplit.GetRange(FIRST_PARAM_INDEX + 12),
                    csvSplit.GetLine(),
                    allocCreateInfo.pUserData);
            }

            UpdateMemStats();
            m_Stats.RegisterCreateAllocation(allocCreateInfo);

            if(requiresDedicatedAllocation || prefersDedicatedAllocation)
            {
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            }

            if(!m_AllocateForBufferImageWarningIssued)
            {
                if(IssueWarning())
                {
                    printf("Line %zu: vmaAllocateMemoryForBuffer or vmaAllocateMemoryForImage cannot be replayed accurately. Using vmaCreateAllocation instead.\n", lineNumber);
                }
                m_AllocateForBufferImageWarningIssued = true;
            }

            Allocation allocDesc = {};
            allocDesc.allocationFlags = allocCreateInfo.flags;
            VkResult res = vmaAllocateMemory(m_Allocator, &memReq, &allocCreateInfo, &allocDesc.allocation, nullptr);
            AddAllocation(lineNumber, origPtr, res, "vmaAllocateMemory (called as vmaAllocateMemoryForBuffer or vmaAllocateMemoryForImage)", std::move(allocDesc));
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaAllocateMemoryForBuffer or vmaAllocateMemoryForImage.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteMapMemory(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::MapMemory);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_Allocations.find(origPtr);
                if(it != m_Allocations.end())
                {
                    if(it->second.allocation)
                    {
                        void* pData;
                        VkResult res = vmaMapMemory(m_Allocator, it->second.allocation, &pData);
                        if(res != VK_SUCCESS)
                        {
                            printf("Line %zu: vmaMapMemory failed (%d)\n", lineNumber, res);
                        }
                    }
                    else
                    {
                        if(IssueWarning())
                        {
                            printf("Line %zu: Cannot call vmaMapMemory - allocation is null.\n", lineNumber);
                        }
                    }
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaMapMemory.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteUnmapMemory(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::UnmapMemory);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_Allocations.find(origPtr);
                if(it != m_Allocations.end())
                {
                    if(it->second.allocation)
                    {
                        vmaUnmapMemory(m_Allocator, it->second.allocation);
                    }
                    else
                    {
                        if(IssueWarning())
                        {
                            printf("Line %zu: Cannot call vmaUnmapMemory - allocation is null.\n", lineNumber);
                        }
                    }
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaMapMemory.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteFlushAllocation(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::FlushAllocation);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 3, false))
    {
        uint64_t origPtr = 0;
        uint64_t offset = 0;
        uint64_t size = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), offset) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), size))
        {
            if(origPtr != 0)
            {
                const auto it = m_Allocations.find(origPtr);
                if(it != m_Allocations.end())
                {
                    if(it->second.allocation)
                    {
                        vmaFlushAllocation(m_Allocator, it->second.allocation, offset, size);
                    }
                    else
                    {
                        if(IssueWarning())
                        {
                            printf("Line %zu: Cannot call vmaFlushAllocation - allocation is null.\n", lineNumber);
                        }
                    }
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaFlushAllocation.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteInvalidateAllocation(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::InvalidateAllocation);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 3, false))
    {
        uint64_t origPtr = 0;
        uint64_t offset = 0;
        uint64_t size = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), offset) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), size))
        {
            if(origPtr != 0)
            {
                const auto it = m_Allocations.find(origPtr);
                if(it != m_Allocations.end())
                {
                    if(it->second.allocation)
                    {
                        vmaInvalidateAllocation(m_Allocator, it->second.allocation, offset, size);
                    }
                    else
                    {
                        if(IssueWarning())
                        {
                            printf("Line %zu: Cannot call vmaInvalidateAllocation - allocation is null.\n", lineNumber);
                        }
                    }
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaInvalidateAllocation.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteTouchAllocation(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::TouchAllocation);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;
        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            const auto it = m_Allocations.find(origPtr);
            if(it != m_Allocations.end())
            {
                if(it->second.allocation)
                {
                    vmaTouchAllocation(m_Allocator, it->second.allocation);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Cannot call vmaTouchAllocation - allocation is null.\n", lineNumber);
                    }
                }
            }
            else
            {
                if(IssueWarning())
                {
                    printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaTouchAllocation.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteGetAllocationInfo(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::GetAllocationInfo);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;
        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            const auto it = m_Allocations.find(origPtr);
            if(it != m_Allocations.end())
            {
                if(it->second.allocation)
                {
                    VmaAllocationInfo allocInfo;
                    vmaGetAllocationInfo(m_Allocator, it->second.allocation, &allocInfo);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Cannot call vmaGetAllocationInfo - allocation is null.\n", lineNumber);
                    }
                }
            }
            else
            {
                if(IssueWarning())
                {
                    printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaGetAllocationInfo.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteMakePoolAllocationsLost(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::MakePoolAllocationsLost);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_Pools.find(origPtr);
                if(it != m_Pools.end())
                {
                    vmaMakePoolAllocationsLost(m_Allocator, it->second.pool, nullptr);
                    UpdateMemStats();
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Pool %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaMakePoolAllocationsLost.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteResizeAllocation(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::ResizeAllocation);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 2, false))
    {
        uint64_t origPtr = 0;
        uint64_t newSize = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), newSize))
        {
            if(origPtr != 0)
            {
                const auto it = m_Allocations.find(origPtr);
                if(it != m_Allocations.end())
                {
                    vmaResizeAllocation(m_Allocator, it->second.allocation, newSize);
                    UpdateMemStats();
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Allocation %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaResizeAllocation.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteDefragmentationBegin(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::DefragmentationBegin);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 9, false))
    {
        VmaDefragmentationInfo2 defragInfo = {};
        std::vector<uint64_t> allocationOrigPtrs;
        std::vector<uint64_t> poolOrigPtrs;
        uint64_t cmdBufOrigPtr = 0;
        uint64_t defragCtxOrigPtr = 0;

        if(StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX), defragInfo.flags) &&
            StrRangeToPtrList(csvSplit.GetRange(FIRST_PARAM_INDEX + 1), allocationOrigPtrs) &&
            StrRangeToPtrList(csvSplit.GetRange(FIRST_PARAM_INDEX + 2), poolOrigPtrs) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 3), defragInfo.maxCpuBytesToMove) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 4), defragInfo.maxCpuAllocationsToMove) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 5), defragInfo.maxGpuBytesToMove) &&
            StrRangeToUint(csvSplit.GetRange(FIRST_PARAM_INDEX + 6), defragInfo.maxGpuAllocationsToMove) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 7), cmdBufOrigPtr) &&
            StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX + 8), defragCtxOrigPtr))
        {
            const size_t allocationOrigPtrCount = allocationOrigPtrs.size();
            std::vector<VmaAllocation> allocations;
            allocations.reserve(allocationOrigPtrCount);
            for(size_t i = 0; i < allocationOrigPtrCount; ++i)
            {
                const auto it = m_Allocations.find(allocationOrigPtrs[i]);
                if(it != m_Allocations.end() && it->second.allocation)
                {
                    allocations.push_back(it->second.allocation);
                }
            }
            if(!allocations.empty())
            {
                defragInfo.allocationCount = (uint32_t)allocations.size();
                defragInfo.pAllocations = allocations.data();
            }

            const size_t poolOrigPtrCount = poolOrigPtrs.size();
            std::vector<VmaPool> pools;
            pools.reserve(poolOrigPtrCount);
            for(size_t i = 0; i < poolOrigPtrCount; ++i)
            {
                const auto it = m_Pools.find(poolOrigPtrs[i]);
                if(it != m_Pools.end() && it->second.pool)
                {
                    pools.push_back(it->second.pool);
                }
            }
            if(!pools.empty())
            {
                defragInfo.poolCount = (uint32_t)pools.size();
                defragInfo.pPools = pools.data();
            }

            if(allocations.size() != allocationOrigPtrCount ||
                pools.size() != poolOrigPtrCount)
            {
                if(IssueWarning())
                {
                    printf("Line %zu: Passing %zu allocations and %zu pools to vmaDefragmentationBegin, while originally %zu allocations and %zu pools were passed.\n",
                        lineNumber,
                        allocations.size(), pools.size(),
                        allocationOrigPtrCount, poolOrigPtrCount);
                }
            }

            if(cmdBufOrigPtr)
            {
                VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
                cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                VkResult res = vkBeginCommandBuffer(m_CommandBuffer, &cmdBufBeginInfo);
                if(res == VK_SUCCESS)
                {
                    defragInfo.commandBuffer = m_CommandBuffer;
                }
                else
                {
                    printf("Line %zu: vkBeginCommandBuffer failed (%d)\n", lineNumber, res);
                }
            }

            m_Stats.RegisterDefragmentation(defragInfo);

            VmaDefragmentationContext defragCtx = nullptr;
            VkResult res = vmaDefragmentationBegin(m_Allocator, &defragInfo, nullptr, &defragCtx);

            if(defragInfo.commandBuffer)
            {
                vkEndCommandBuffer(m_CommandBuffer);

                VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &m_CommandBuffer;
                vkQueueSubmit(m_TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(m_TransferQueue);
            }

            if(res >= VK_SUCCESS)
            {
                if(defragCtx)
                {
                    if(defragCtxOrigPtr)
                    {
                        // We have defragmentation context, originally had defragmentation context: Store it.
                        m_DefragmentationContexts[defragCtxOrigPtr] = defragCtx;
                    }
                    else
                    {
                        // We have defragmentation context, originally it was null: End immediately.
                        vmaDefragmentationEnd(m_Allocator, defragCtx);
                    }
                }
                else
                {
                    if(defragCtxOrigPtr)
                    {
                        // We have no defragmentation context, originally there was one: Store null.
                        m_DefragmentationContexts[defragCtxOrigPtr] = nullptr;
                    }
                    else
                    {
                        // We have no defragmentation context, originally there wasn't as well - nothing to do.
                    }
                }
            }
            else
            {
                if(defragCtxOrigPtr)
                {
                    // Currently failed, originally succeeded.
                    if(IssueWarning())
                    {
                        printf("Line %zu: vmaDefragmentationBegin failed (%d), while originally succeeded.\n", lineNumber, res);
                    }
                }
                else
                {
                    // Currently failed, originally don't know.
                    if(IssueWarning())
                    {
                        printf("Line %zu: vmaDefragmentationBegin failed (%d).\n", lineNumber, res);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaDefragmentationBegin.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteDefragmentationEnd(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::DefragmentationEnd);

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 1, false))
    {
        uint64_t origPtr = 0;

        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_DefragmentationContexts.find(origPtr);
                if(it != m_DefragmentationContexts.end())
                {
                    vmaDefragmentationEnd(m_Allocator, it->second);
                    m_DefragmentationContexts.erase(it);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Defragmentation context %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaDefragmentationEnd.\n", lineNumber);
            }
        }
    }
}

void Player::ExecuteSetPoolName(size_t lineNumber, const CsvSplit& csvSplit)
{
    m_Stats.RegisterFunctionCall(VMA_FUNCTION::SetPoolName);

    if(!g_UserDataEnabled)
    {
        return;
    }

    if(ValidateFunctionParameterCount(lineNumber, csvSplit, 2, true))
    {
        uint64_t origPtr = 0;
        if(StrRangeToPtr(csvSplit.GetRange(FIRST_PARAM_INDEX), origPtr))
        {
            if(origPtr != 0)
            {
                const auto it = m_Pools.find(origPtr);
                if(it != m_Pools.end())
                {
                    std::string poolName;
                    csvSplit.GetRange(FIRST_PARAM_INDEX + 1).to_str(poolName);
                    vmaSetPoolName(m_Allocator, it->second.pool, !poolName.empty() ? poolName.c_str() : nullptr);
                }
                else
                {
                    if(IssueWarning())
                    {
                        printf("Line %zu: Pool %llX not found.\n", lineNumber, origPtr);
                    }
                }
            }
        }
        else
        {
            if(IssueWarning())
            {
                printf("Line %zu: Invalid parameters for vmaSetPoolName.\n", lineNumber);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Main functions

static void PrintCommandLineSyntax()
{
    printf(
        "Command line syntax:\n"
        "    VmaReplay [Options] <SrcFile.csv>\n"
        "Available options:\n"
        "    -v <Number> - Verbosity level:\n"
        "        0 - Minimum verbosity. Prints only warnings and errors.\n"
        "        1 - Default verbosity. Prints important messages and statistics.\n"
        "        2 - Maximum verbosity. Prints a lot of information.\n"
        "    -i <Number> - Repeat playback given number of times (iterations)\n"
        "        Default is 1. Vulkan is reinitialized with every iteration.\n"
        "    --MemStats <Value> - 0 to disable or 1 to enable memory statistics.\n"
        "        Default is 0. Enabling it may negatively impact playback performance.\n"
        "    --DumpStatsAfterLine <Line> - Dump VMA statistics to JSON file after specified source file line finishes execution.\n"
        "        File is written to current directory with name: VmaReplay_Line####.json.\n"
        "        This parameter can be repeated.\n"
        "    --DumpDetailedStatsAfterLine <Line> - Like command above, but includes detailed map.\n"
        "    --DefragmentAfterLine <Line> - Defragment memory after specified source file line and print statistics.\n"
        "        It also prints detailed statistics to files VmaReplay_Line####_Defragment*.json\n"
        "    --DefragmentationFlags <Flags> - Flags to be applied when using DefragmentAfterLine.\n"
        "    --Lines <Ranges> - Replay only limited set of lines from file\n"
        "        Ranges is comma-separated list of ranges, e.g. \"-10,15,18-25,31-\".\n"
        "    --PhysicalDevice <Index> - Choice of Vulkan physical device. Default: 0.\n"
        "    --UserData <Value> - 0 to disable or 1 to enable setting pUserData during playback.\n"
        "        Default is 1. Affects both creation of buffers and images, as well as calls to vmaSetAllocationUserData.\n"
        "    --VK_LAYER_KHRONOS_validation <Value> - 0 to disable or 1 to enable validation layers.\n"
        "        By default the layers are silently enabled if available.\n"
        "    --VK_EXT_memory_budget <Value> - 0 to disable or 1 to enable this extension.\n"
        "        By default the extension is silently enabled if available.\n"
    );
}

static int ProcessFile(size_t iterationIndex, const char* data, size_t numBytes, duration& outDuration)
{
    outDuration = duration::max();

    const bool useLineRanges = !g_LineRanges.IsEmpty();
    const bool useDumpStatsAfterLine = !g_DumpStatsAfterLine.empty();
    const bool useDefragmentAfterLine = !g_DefragmentAfterLine.empty();

    LineSplit lineSplit(data, numBytes);
    StrRange line;

    if(!lineSplit.GetNextLine(line) ||
        !StrRangeEq(line, "Vulkan Memory Allocator,Calls recording"))
    {
        printf("ERROR: Incorrect file format.\n");
        return RESULT_ERROR_FORMAT;
    }

    if(!lineSplit.GetNextLine(line) || !ParseFileVersion(line) || !ValidateFileVersion())
    {
        printf("ERROR: Incorrect file format version.\n");
        return RESULT_ERROR_FORMAT;
    }

    if(g_Verbosity == VERBOSITY::MAXIMUM)
    {
        printf("Format version: %u,%u\n",
            GetVersionMajor(g_FileVersion),
            GetVersionMinor(g_FileVersion));
    }

    // Parse configuration
    const bool configEnabled = g_FileVersion >= MakeVersion(1, 3);
    ConfigurationParser configParser;
    if(configEnabled)
    {
        if(!configParser.Parse(lineSplit))
        {
            return RESULT_ERROR_FORMAT;
        }
    }

    Player player;
    int result = player.Init();

    if(configEnabled)
    {
        player.ApplyConfig(configParser);
    }

    size_t executedLineCount = 0;
    if(result == 0)
    {
        if(g_Verbosity > VERBOSITY::MINIMUM)
        {
            if(useLineRanges)
            {
                printf("Playing #%zu (limited range of lines)...\n", iterationIndex + 1);
            }
            else
            {
                printf("Playing #%zu...\n", iterationIndex + 1);
            }
        }

        const time_point timeBeg = std::chrono::high_resolution_clock::now();

        while(lineSplit.GetNextLine(line))
        {
            const size_t currLineNumber = lineSplit.GetNextLineIndex();

            bool execute = true;
            if(useLineRanges)
            {
                execute = g_LineRanges.Includes(currLineNumber);
            }

            if(execute)
            {
                player.ExecuteLine(currLineNumber, line);
                ++executedLineCount;
            }

            while(useDumpStatsAfterLine &&
                g_DumpStatsAfterLineNextIndex < g_DumpStatsAfterLine.size() &&
                currLineNumber >= g_DumpStatsAfterLine[g_DumpStatsAfterLineNextIndex].line)
            {
                const size_t requestedLine = g_DumpStatsAfterLine[g_DumpStatsAfterLineNextIndex].line;
                const bool detailed = g_DumpStatsAfterLine[g_DumpStatsAfterLineNextIndex].detailed;
                
                if(g_Verbosity == VERBOSITY::MAXIMUM)
                {
                    printf("Dumping %sstats after line %zu actual line %zu...\n",
                        detailed ? "detailed " : "",
                        requestedLine,
                        currLineNumber);
                }

                player.DumpStats("VmaReplay_Line%04zu.json", requestedLine, detailed);
                
                ++g_DumpStatsAfterLineNextIndex;
            }

            while(useDefragmentAfterLine &&
                g_DefragmentAfterLineNextIndex < g_DefragmentAfterLine.size() &&
                currLineNumber >= g_DefragmentAfterLine[g_DefragmentAfterLineNextIndex])
            {
                const size_t requestedLine = g_DefragmentAfterLine[g_DefragmentAfterLineNextIndex];
                if(g_Verbosity >= VERBOSITY::DEFAULT)
                {
                    printf("Defragmenting after line %zu actual line %zu...\n",
                        requestedLine,
                        currLineNumber);
                }

                player.DumpStats("VmaReplay_Line%04zu_Defragment_1Before.json", requestedLine, true);
                player.Defragment();
                player.DumpStats("VmaReplay_Line%04zu_Defragment_2After.json", requestedLine, true);
                
                ++g_DefragmentAfterLineNextIndex;
            }
        }

        const duration playDuration = std::chrono::high_resolution_clock::now() - timeBeg;
        outDuration = playDuration;

        // End stats.
        if(g_Verbosity > VERBOSITY::MINIMUM)
        {
            std::string playDurationStr;
            SecondsToFriendlyStr(ToFloatSeconds(playDuration), playDurationStr);

            printf("Done.\n");
            printf("Playback took: %s\n", playDurationStr.c_str());
        }
        if(g_Verbosity == VERBOSITY::MAXIMUM)
        {
            printf("File lines: %zu\n", lineSplit.GetNextLineIndex());
            printf("Executed %zu file lines\n", executedLineCount);
        }

        player.PrintStats();
    }

    return result;
}

static int ProcessFile()
{
    if(g_Verbosity > VERBOSITY::MINIMUM)
    {
        printf("Loading file \"%s\"...\n", g_FilePath.c_str());
    }
    int result = 0;

    FILE* file = nullptr;
    const errno_t err = fopen_s(&file, g_FilePath.c_str(), "rb");
    if(err == 0)
    {
        _fseeki64(file, 0, SEEK_END);
        const size_t fileSize = (size_t)_ftelli64(file);
        _fseeki64(file, 0, SEEK_SET);

        if(fileSize > 0)
        {
            std::vector<char> fileContents(fileSize);
            fread(fileContents.data(), 1, fileSize, file);

            // Begin stats.
            if(g_Verbosity == VERBOSITY::MAXIMUM)
            {
                printf("File size: %zu B\n", fileSize);
            }

            duration durationSum = duration::zero();
            for(size_t i = 0; i < g_IterationCount; ++i)
            {
                duration currDuration;
                ProcessFile(i, fileContents.data(), fileContents.size(), currDuration);
                durationSum += currDuration;
            }

            if(g_IterationCount > 1)
            {
                std::string playDurationStr;
                SecondsToFriendlyStr(ToFloatSeconds(durationSum / g_IterationCount), playDurationStr);
                printf("Average playback time from %zu iterations: %s\n", g_IterationCount, playDurationStr.c_str());
            }
        }
        else
        {
            printf("ERROR: Source file is empty.\n");
            result = RESULT_ERROR_SOURCE_FILE;
        }

        fclose(file);
    }
    else
    {
        printf("ERROR: Couldn't open file (%i).\n", err);
        result = RESULT_ERROR_SOURCE_FILE;
    }

    return result;
}

static int main2(int argc, char** argv)
{
    CmdLineParser cmdLineParser(argc, argv);

    cmdLineParser.RegisterOpt(CMD_LINE_OPT_VERBOSITY, 'v', true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_ITERATIONS, 'i', true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_LINES, "Lines", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_PHYSICAL_DEVICE, "PhysicalDevice", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_USER_DATA, "UserData", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_VK_EXT_MEMORY_BUDGET, "VK_EXT_memory_budget", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_VK_LAYER_KHRONOS_VALIDATION, VALIDATION_LAYER_NAME, true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_MEM_STATS, "MemStats", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_DUMP_STATS_AFTER_LINE, "DumpStatsAfterLine", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_DEFRAGMENT_AFTER_LINE, "DefragmentAfterLine", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_DEFRAGMENTATION_FLAGS, "DefragmentationFlags", true);
    cmdLineParser.RegisterOpt(CMD_LINE_OPT_DUMP_DETAILED_STATS_AFTER_LINE, "DumpDetailedStatsAfterLine", true);

    CmdLineParser::RESULT res;
    while((res = cmdLineParser.ReadNext()) != CmdLineParser::RESULT_END)
    {
        switch(res)
        {
        case CmdLineParser::RESULT_OPT:
            switch(cmdLineParser.GetOptId())
            {
            case CMD_LINE_OPT_VERBOSITY:
                {
                    uint32_t verbosityVal = UINT32_MAX;
                    if(StrRangeToUint(StrRange(cmdLineParser.GetParameter()), verbosityVal) &&
                        verbosityVal < (uint32_t)VERBOSITY::COUNT)
                    {
                        g_Verbosity = (VERBOSITY)verbosityVal;
                    }
                    else
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            case CMD_LINE_OPT_ITERATIONS:
                if(!StrRangeToUint(StrRange(cmdLineParser.GetParameter()), g_IterationCount))
                {
                    PrintCommandLineSyntax();
                    return RESULT_ERROR_COMMAND_LINE;
                }
                break;
            case CMD_LINE_OPT_LINES:
                if(!g_LineRanges.Parse(StrRange(cmdLineParser.GetParameter())))
                {
                    PrintCommandLineSyntax();
                    return RESULT_ERROR_COMMAND_LINE;
                }
                break;
            case CMD_LINE_OPT_PHYSICAL_DEVICE:
                if(!StrRangeToUint(StrRange(cmdLineParser.GetParameter()), g_PhysicalDeviceIndex))
                {
                    PrintCommandLineSyntax();
                    return RESULT_ERROR_COMMAND_LINE;
                }
                break;
            case CMD_LINE_OPT_USER_DATA:
                if(!StrRangeToBool(StrRange(cmdLineParser.GetParameter()), g_UserDataEnabled))
                {
                    PrintCommandLineSyntax();
                    return RESULT_ERROR_COMMAND_LINE;
                }
                break;
            case CMD_LINE_OPT_VK_EXT_MEMORY_BUDGET:
                {
                    bool newValue;
                    if(StrRangeToBool(StrRange(cmdLineParser.GetParameter()), newValue))
                    {
                        g_VK_EXT_memory_budget_request = newValue ?
                            VULKAN_EXTENSION_REQUEST::ENABLED :
                            VULKAN_EXTENSION_REQUEST::DISABLED;
                    }
                    else
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            case CMD_LINE_OPT_VK_LAYER_KHRONOS_VALIDATION:
                {
                    bool newValue;
                    if(StrRangeToBool(StrRange(cmdLineParser.GetParameter()), newValue))
                    {
                        g_VK_LAYER_KHRONOS_validation = newValue ?
                            VULKAN_EXTENSION_REQUEST::ENABLED :
                            VULKAN_EXTENSION_REQUEST::DISABLED;
                    }
                    else
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            case CMD_LINE_OPT_MEM_STATS:
                if(!StrRangeToBool(StrRange(cmdLineParser.GetParameter()), g_MemStatsEnabled))
                {
                    PrintCommandLineSyntax();
                    return RESULT_ERROR_COMMAND_LINE;
                }
                break;
            case CMD_LINE_OPT_DUMP_STATS_AFTER_LINE:
            case CMD_LINE_OPT_DUMP_DETAILED_STATS_AFTER_LINE:
                {
                    size_t line;
                    if(StrRangeToUint(StrRange(cmdLineParser.GetParameter()), line))
                    {
                        const bool detailed =
                            cmdLineParser.GetOptId() == CMD_LINE_OPT_DUMP_DETAILED_STATS_AFTER_LINE;
                        g_DumpStatsAfterLine.push_back({line, detailed});
                    }
                    else
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            case CMD_LINE_OPT_DEFRAGMENT_AFTER_LINE:
                {
                    size_t line;
                    if(StrRangeToUint(StrRange(cmdLineParser.GetParameter()), line))
                    {
                        g_DefragmentAfterLine.push_back(line);
                    }
                    else
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            case CMD_LINE_OPT_DEFRAGMENTATION_FLAGS:
                {
                    if(!StrRangeToUint(StrRange(cmdLineParser.GetParameter()), g_DefragmentationFlags))
                    {
                        PrintCommandLineSyntax();
                        return RESULT_ERROR_COMMAND_LINE;
                    }
                }
                break;
            default:
                assert(0);
            }
            break;
        case CmdLineParser::RESULT_PARAMETER:
            if(g_FilePath.empty())
            {
                g_FilePath = cmdLineParser.GetParameter();
            }
            else
            {
                PrintCommandLineSyntax();
                return RESULT_ERROR_COMMAND_LINE;
            }
            break;
        case CmdLineParser::RESULT_ERROR:
            PrintCommandLineSyntax();
            return RESULT_ERROR_COMMAND_LINE;
            break;
        default:
            assert(0);
        }
    }

    // Postprocess command line parameters.

    if(g_FilePath.empty())
    {
        PrintCommandLineSyntax();
        return RESULT_ERROR_COMMAND_LINE;
    }

    // Sort g_DumpStatsAfterLine and make unique.
    std::sort(g_DumpStatsAfterLine.begin(), g_DumpStatsAfterLine.end());
    g_DumpStatsAfterLine.erase(
        std::unique(g_DumpStatsAfterLine.begin(), g_DumpStatsAfterLine.end()),
        g_DumpStatsAfterLine.end());

    // Sort g_DefragmentAfterLine and make unique.
    std::sort(g_DefragmentAfterLine.begin(), g_DefragmentAfterLine.end());
    g_DefragmentAfterLine.erase(
        std::unique(g_DefragmentAfterLine.begin(), g_DefragmentAfterLine.end()),
        g_DefragmentAfterLine.end());

    return ProcessFile();
}

int main(int argc, char** argv)
{
    try
    {
        return main2(argc, argv);
    }
    catch(const std::exception& e)
    {
        printf("ERROR: %s\n", e.what());
        return RESULT_EXCEPTION;
    }
    catch(...)
    {
        printf("UNKNOWN ERROR\n");
        return RESULT_EXCEPTION;
    }
}
