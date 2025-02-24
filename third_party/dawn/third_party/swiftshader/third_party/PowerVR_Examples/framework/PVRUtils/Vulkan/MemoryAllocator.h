/*!
\brief
\file PVRUtils/Vulkan/MemoryAllocator.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/ApiObjectsVk.h"
#include "PVRCore/Log.h"
/// <summary>Specifies that function pointers for the Vulkan functions will be retrieved external to VMA.</summary>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "PVRVk/pvrvk_vulkan_wrapper.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <list>
#include <cassert> // for assert
#include <algorithm> // for min, max
#include <mutex> // for std::mutex
#include <atomic> // for std::atomic
#include <cstdlib>

#if (_ANDROID)
inline void* aligned_alloc(size_t size, size_t alignment)
{
	void* ret = memalign(alignment, size);
	return ret;
}

#endif
namespace pvr {
namespace utils {
namespace vma {
namespace impl {
//!\cond NO_DOXYGEN
class Pool_;
class Allocation_;
class Allocator_;
class DeviceMemoryWrapper_;
class DeviceMemoryCallbackDispatcher_;
#ifdef DEBUG
// Enable the following defines for improved validation of memory usage in debug builds
// VMA_DEBUG_INITIALIZE_ALLOCATIONS:
//		Makes memory of all new allocations initialized to bit pattern `0xDCDCDCDC`.
//		Before an allocation is destroyed, its memory is filled with bit pattern `0xEFEFEFEF`.
//		If you find these values while debugging your program, good chances are that you incorrectly
//		read Vulkan memory that is allocated but not initialized, or already freed, respectively.
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
// VMA_DEBUG_MARGIN:
//		Enforces a specified number of bytes as a margin before and after every allocation.
//		If your bug goes away after enabling margins, it means it may be caused by memory
//		being overwritten outside of allocation boundaries.It is not100 % certain though. Change in application behavior may also be caused by different order and
//		distribution of allocations across memory blocks after margins are applied.
#define VMA_DEBUG_MARGIN 4
// VMA_DEBUG_DETECT_CORRUPTION:
//		If this feature is enabled, number of bytes specified as `VMA_DEBUG_MARGIN`
//		(it must be multiply of 4) before and after every allocation is filled with a magic number. This idea is also know as "canary".
//		Memory is automatically mapped and unmapped if necessary. This number is validated automatically when the allocation is destroyed.
//		If it's not equal to the expected value, `VMA_ASSERT()` is executed. It clearly means that either CPU or GPU overwritten the
//		memory outside of boundaries of the allocation, which indicates a serious bug.
#define VMA_DEBUG_DETECT_CORRUPTION 1
#endif

// The v2.2.0 release of VulkanMemoryAllocator uses an incorrect ifdef guard - this issue has already been fixed on master.
// See here for more details: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/issues/52
#define VMA_USE_STL_SHARED_MUTEX 0
#include "vk_mem_alloc.h"
//!\endcond
} // namespace impl

/// <summary>Forwared-declared reference-counted handle to a Pool. For detailed documentation, see below</summary>
typedef std::shared_ptr<impl::Pool_> Pool;
/// <summary>Forwared-declared reference-counted handle to a Allocation. For detailed documentation, see below</summary>
typedef std::shared_ptr<impl::Allocation_> Allocation;
/// <summary>Forwared-declared reference-counted handle to a Allocator. For detailed documentation, see below</summary>
typedef std::shared_ptr<impl::Allocator_> Allocator;
/// <summary>Forwared-declared reference-counted handle to a Allocator. For detailed documentation, see below</summary>
typedef std::weak_ptr<impl::Allocator_> AllocatorWeakPtr;
/// <summary>Forwared-declared reference-counted handle to a DeviceMemoryWrapper. For detailed documentation, see below</summary>
typedef std::shared_ptr<impl::DeviceMemoryWrapper_> DeviceMemoryWrapper;

/// <summary>Callback function called after successful vkAllocateMemory.</summary>
typedef void(VKAPI_PTR* PFN_AllocateDeviceMemoryFunction)(Allocator allocator, uint32_t memoryType, pvrvk::DeviceMemory memory, VkDeviceSize size);

/// <summary>Callback function called before vkFreeMemory.</summary>
typedef void(VKAPI_PTR* PFN_FreeDeviceMemoryFunction)(Allocator allocator, uint32_t memoryType, pvrvk::DeviceMemory memory, VkDeviceSize size);

/// <summary>The DeviceMemoryCallbacks struct defines a set of callbacks that the library will call for `vkAllocateMemory` and `vkFreeMemory`.
/// Provided for informative purpose, e.g. to gather statistics about number of allocations or total amount of memory allocated in Vulkan.
/// Used in AllocatorCreateInfo::pDeviceMemoryCallbacks.</summary>
struct DeviceMemoryCallbacks
{
	/// <summary>Optional, can be null.</summary>
	PFN_AllocateDeviceMemoryFunction pfnAllocate;
	/// <summary>Optional, can be null.</summary>
	PFN_FreeDeviceMemoryFunction pfnFree;

	/// <summary>Default Constructor.</summary>
	DeviceMemoryCallbacks() : pfnAllocate(nullptr), pfnFree(nullptr) {}
};

/// <summary>The AllocationCreateFlags enum set defines a set of flags which may effect the way in which the device memory is allocated.</summary>
enum class AllocationCreateFlags
{
	e_NONE = 0,
	/// <summary>
	/// brief Set this flag if the allocation should have its own memory block.
	/// Use it for special, big resources, like fullscreen images used as attachments.
	/// This flag must also be used for host visible resources that you want to map
	/// simultaneously because otherwise they might end up as regions of the same
	/// DeviceMemory, while mapping same DeviceMemory multiple times
	/// simultaneously is illegal.
	/// You should not use this flag if AllocationCreateInfo::pool is not null.</summary>
	e_DEDICATED_MEMORY_BIT = impl::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,

	/// <summary>
	/// brief Set this flag to only try to allocate from existing DeviceMemory blocks and never create new such block.
	/// If new allocation cannot be placed in any of the existing blocks, allocation
	/// fails with `pvrvk::Error::e_OUT_OF_DEVICE_MEMORY` error. You should not use e_DEDICATED_MEMORY_BIT and
	/// e_NEVER_ALLOCATE_BIT at the same time. It makes no sense. If VmaAllocationCreateInfo::pool is not null, this flag is implied and ignored.</summary>
	e_NEVER_ALLOCATE_BIT = impl::VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,

	/// <summary>
	/// Set this flag to use a memory that will be persistently mapped and retrieve pointer to it.
	/// Pointer to mapped memory will be returned through VmaAllocationInfo::pMappedData. You cannot
	/// map the memory on your own as multiple mappings of a single `VkDeviceMemory` are
	/// illegal.
	/// If VmaAllocationCreateInfo::pool is not null, usage of this flag must match
	/// usage of flag `PoolCreateFlags::e_PERSISTENT_MAP_BIT` used during pool creation.
	/// Is it valid to use this flag for allocation made from memory type that is not
	///`HOST_VISIBLE`. This flag is then ignored and memory is not mapped. This is
	/// useful if you need an allocation that is efficient to use on GPU
	/// (`DEVICE_LOCAL`) and still want to map it directly if possible on platforms that
	/// support it (e.g. Intel GPU).</summary>
	e_MAPPED_BIT = impl::VMA_ALLOCATION_CREATE_MAPPED_BIT,

	/// <summary>
	/// Allocation created with this flag can become lost as a result of another
	/// allocation with `AllocationCreateFlags::_CAN_MAKE_OTHER_LOST_BIT` flag, so you
	/// must check it before use.
	/// To check if allocation is not lost, call isAllocationLost() on the allocation.
	/// For details about supporting lost allocations, see Lost Allocations
	/// chapter of User Guide on Main Page.</summary>
	e_CAN_BECOME_LOST_BIT = impl::VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT,

	/// <summary>
	/// While creating allocation using this flag, other allocations that were
	/// created with flag `AllocationCreateFlags::e_CAN_BECOME_LOST_BIT` can become lost.
	/// For details about supporting lost allocations, see Lost Allocations
	/// chapter of User Guide on Main Page.</summary>
	e_CAN_MAKE_OTHER_LOST_BIT = impl::VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT,

	/// <summary>
	/// Set this flag to treat VmaAllocationCreateInfo::pUserData as pointer to a
	///	null-terminated string. Instead of copying pointer value, a local copy of the
	///	string is made and stored in allocation's pUserData. The string is automatically
	///	freed together with the allocation. It is also used in vmaBuildStatsString().</summary>
	e_USER_DATA_COPY_STRING_BIT = impl::VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
	e_FLAG_BITS_MAX_ENUM = impl::VMA_ALLOCATION_CREATE_FLAG_BITS_MAX_ENUM
};
DEFINE_ENUM_BITWISE_OPERATORS(AllocationCreateFlags)

/// <summary>The AllocatorCreateFlags enum set defines a set of flags modifying the way in which a VmaAllocator will function.</summary>
enum class AllocatorCreateFlags
{
	e_NONE = 0x00000000,

	/// <summary>
	/// Allocator and all objects created from it will not be synchronized internally, so you must guarantee they are used from only one thread at a time or synchronized
	/// externally by you.
	/// Using this flag may increase performance because internal mutexes are not used.</summary>
	e_EXTERNALLY_SYNCHRONIZED_BIT = impl::VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,

	/// <summary>
	/// Enables usage of VK_KHR_dedicated_allocation extension.
	/// Using this extenion will automatically allocate dedicated blocks of memory for
	/// some buffers and images instead of suballocating place for them out of bigger
	/// memory blocks (as if you explicitly used VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
	/// flag) when it is recommended by the driver. It may improve performance on some
	/// GPUs.
	/// You may set this flag only if you found out that following device extensions are
	/// supported, you enabled them while creating Vulkan device passed as
	/// VmaAllocatorCreateInfo::device, and you want them to be used internally by this
	/// library:
	/// - VK_KHR_get_memory_requirements2
	/// - VK_KHR_dedicated_allocation
	/// If this flag is enabled, you must also provide
	/// VmaAllocatorCreateInfo::pVulkanFunctions and fill at least members:
	///  VmaVulkanFunctions::vkGetBufferMemoryRequirements2KHR,
	/// VmaVulkanFunctions::vkGetImageMemoryRequirements2KHR, because they are never
	/// imported statically.
	/// When this flag is set, you can experience following warnings reported by Vulkan
	/// validation layer. You can ignore them.
	/// vkBindBufferMemory(): Binding memory to buffer 0x2d but vkGetBufferMemoryRequirements() has not been called on that buffer.</summary>
	e_KHR_DEDICATED_ALLOCATION_BIT = impl::VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,

	e_FLAG_BITS_MAX_ENUM = impl::VMA_ALLOCATOR_CREATE_FLAG_BITS_MAX_ENUM
};
DEFINE_ENUM_BITWISE_OPERATORS(AllocatorCreateFlags)

/// <summary>The MemoryUsage enum set provides a high level mechanism for specifying how created memory will be used</summary>
enum class MemoryUsage
{
	/// <summary>No intended memory usage specified. Use other members of VmaAllocationCreateInfo to specify your requirements.</summary>
	e_UNKNOWN = impl::VMA_MEMORY_USAGE_UNKNOWN,
	/// <summary>Memory will be used on device only, so faster access from the device is preferred. No need to be mappable on host.</summary>
	e_GPU_ONLY = impl::VMA_MEMORY_USAGE_GPU_ONLY,
	/// <summary>Memory will be mapped on host. Could be used for transfer to/from device.
	/// Guarantees to be HOST_VISIBLE` and `HOST_COHERENT.</summary>
	e_CPU_ONLY = impl::VMA_MEMORY_USAGE_CPU_ONLY,
	/// <summary>Memory will be used for frequent (dynamic) updates from host and reads on device (upload).
	/// Guarantees to be `HOST_VISIBLE`.</summary>
	e_CPU_TO_GPU = impl::VMA_MEMORY_USAGE_CPU_TO_GPU,
	/// <summary>Memory will be used for frequent writing on device and readback on host (download).
	/// Guarantees to be HOST_VISIBLE.</summary>
	e_GPU_TO_CPU = impl::VMA_MEMORY_USAGE_GPU_TO_CPU,
	e_MAX_ENUM = impl::VMA_MEMORY_USAGE_MAX_ENUM
};

/// <summary>The PoolCreateFlags enum provides a set of flags which will contorl the way in which a VMA pool will function</summary>
enum class PoolCreateFlags
{
	/// <summary>
	/// Use this flag if you always allocate only buffers and linear images or only optimal images out of this pool and so Buffer-Image Granularity can be ignored.
	/// This is an optional optimization flag.
	/// If you always allocate using MemoryAllocator::createBuffer(), MemoryAllocator::createImage(),
	/// MemoryAllocator::allocateMemoryForBuffer(), then you don't need to use it because allocator
	/// knows exact type of your allocations so it can handle Buffer-Image Granularity
	/// in the optimal way.
	/// If you also allocate using allocateMemoryForImage() or allocateMemory(),
	/// exact type of such allocations is not known, so allocator must be conservative
	/// in handling Buffer-Image Granularity, which can lead to suboptimal allocation
	/// (wasted memory). In that case, if you can make sure you always allocate only
	/// buffers and linear images or only optimal images out of this pool, use this flag
	/// to make allocator disregard Buffer-Image Granularity and so make allocations
	/// more optimal.
	/// </summary>
	e_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT = impl::VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,

	e_FLAG_BITS_MAX_ENUM = impl::VMA_POOL_CREATE_FLAG_BITS_MAX_ENUM
};
DEFINE_ENUM_BITWISE_OPERATORS(PoolCreateFlags)

/// <summary>Debug Report flags to be used when creating the VmaAllocator object. These flags will effect the way in which vma will provide debug logging.<summary>
enum class DebugReportFlags
{
	/// <summary>Report none</summary>
	None = 0,
	/// <summary>Report when allocating and freeing device memory<summary>
	DeviceMemory = 0x1,
	/// <summary>Report when allocating and freeing allocation<summary>
	Allocation = 0x2,
	/// <summary>Report when defragmenting the allocation</summary>
	Defragments = 0x4,
	All = DeviceMemory | Allocation | Defragments,
};
DEFINE_ENUM_BITWISE_OPERATORS(DebugReportFlags)

/// <summary>The AllocationCreateInfo structure will control the way in which any one particular allocation is made.</summary>
struct AllocationCreateInfo
{
	/// <summary>Use AllocationCreateFlags enum.</summary>
	AllocationCreateFlags flags;

	/// <summary>Intended usage of memory.
	/// Leave `MemoryUsage::e_UNKNOWN` if you specify `requiredFlags`. You can also use both.
	/// If `pool` is not null, this member is ignored.</summary>
	MemoryUsage usage;

	/// <summary>Flags that must be set in a Memory Type chosen for an allocation.
	/// Leave 0 if you specify requirement via usage.
	/// If `pool` is not null, this member is ignored.</summary>
	pvrvk::MemoryPropertyFlags requiredFlags;

	/// <summary>Flags that preferably should be set in a Memory Type chosen for an allocation.
	/// Set to 0 if no additional flags are prefered and only `requiredFlags` should be used.
	/// If not 0, it must be a superset or equal to `requiredFlags`.
	/// If `pool` is not null, this member is ignored.</summary>
	pvrvk::MemoryPropertyFlags preferredFlags;

	/// <summary>Bitmask containing one bit set for every memory type acceptable for this allocation.
	/// Value 0 is equivalent to `UINT32_MAX` - it means any memory type is accepted if
	/// it meets other requirements specified by this structure, with no further
	/// restrictions on memory type index.
	/// If pool is not null, this member is ignored.</summary>
	uint32_t memoryTypeBits;

	/// <summary>Pool that this allocation should be created in.
	/// Leave `VK_NULL_HANDLE` to allocate from general memory.</summary>
	Pool pool;

	/// <summary>Custom general-purpose pointer that will be stored in VmaAllocation, can be read as VmaAllocationInfo::pUserData and changed using
	/// vmaSetAllocationUserData().</summary>
	void* pUserData;

	/// <summary>Default Constructor.</summary>
	AllocationCreateInfo()
		: flags(AllocationCreateFlags::e_NONE), usage(MemoryUsage::e_UNKNOWN), requiredFlags(pvrvk::MemoryPropertyFlags::e_NONE),
		  preferredFlags(pvrvk::MemoryPropertyFlags::e_NONE), memoryTypeBits(0), pUserData(nullptr)
	{}
};

/// <summary>The AllocatorCreateInfo struct specifies the way in which a VmaAllocator will be created.</summary>
struct AllocatorCreateInfo
{
	/// <summary>Flags for created allocator. Use VmaAllocatorCreateFlags enum.</summary>
	AllocatorCreateFlags flags;

	/// <summary>Vulkan device.
	/// It must be valid throughout whole lifetime of created allocator.</summary>
	pvrvk::Device device;

	/// <summary>Preferred size of a single `pvrvk::DeviceMemory` block to be allocated from large heaps.
	/// Set to 0 to use default, which is currently 256 MB.</summary>
	pvrvk::DeviceSize preferredLargeHeapBlockSize;

	/// <summary>Custom CPU memory allocation callbacks.
	/// Optional, can be null. When specified, will also be used for all CPU-side memory allocations.</summary>
	const pvrvk::AllocationCallbacks* pAllocationCallbacks;

	/// <summary>Informative callbacks for vkAllocateMemory, vkFreeMemory.
	/// Optional, can be null.</summary>
	const DeviceMemoryCallbacks* pDeviceMemoryCallbacks;

	/// <summary>Maximum number of additional frames that are in use at the same time as current frame.
	/// This value is used only when you make allocations with
	/// AllocationCreateFlags::e_CAN_BECOME_LOST_BIT flag. Such allocation cannot become
	/// lost if allocation.lastUseFrameIndex >= allocator.currentFrameIndex - frameInUseCount.
	/// For example, if you double-buffer your command buffers, so resources used for
	/// rendering in previous frame may still be in use by the GPU at the moment you
	/// allocate resources needed for the current frame, set this value to 1.
	/// If you want to allow any allocations other than used in the current frame to
	/// become lost, set this value to 0.</summary>
	uint32_t frameInUseCount;

	/// <summary>Either NULL or a pointer to an array of limits on maximum number of bytes that can be allocated out of particular Vulkan memory heap.
	/// If not NULL, it must be a pointer to an array of
	/// `pvrvk::PhysicalDeviceMemoryProperties::memoryHeapCount` elements, defining limit on
	/// maximum number of bytes that can be allocated out of particular Vulkan memory
	/// heap.
	///  Any of the elements may be equal to `VK_WHOLE_SIZE`, which means no limit on that
	/// heap. This is also the default in case of `pHeapSizeLimit` = NULL.
	/// If there is a limit defined for a heap:
	/// - If user tries to allocate more memory from that heap using this allocator,
	///   the allocation fails with `pvrvk::Error::e_OUT_OF_DEVICE_MEMORY`.
	/// - If the limit is smaller than heap size reported in `pvrvk::MemoryHeap::size`, the
	///   value of this limit will be reported instead when using vmaGetMemoryProperties().</summary>
	const pvrvk::DeviceSize* pHeapSizeLimit;

	/// <summary>Flags which will effect the way in which the debug report mechanism will function.</summary>
	DebugReportFlags reportFlags;

	/// <summary>Default Constructor.</summary>
	AllocatorCreateInfo()
		: flags(AllocatorCreateFlags(0)), preferredLargeHeapBlockSize(0), pAllocationCallbacks(nullptr), pDeviceMemoryCallbacks(nullptr), frameInUseCount(0),
		  pHeapSizeLimit(nullptr), reportFlags(DebugReportFlags(0))
	{}

	/// <summary>Constructor.</summary>
	/// <param name="device">The device to be used for allocating memory by this allocator</param>
	/// <param name="preferredLargeHeapBlockSize">The preferred size of any single allocation from large heaps</param>
	/// <param name="flags">Flags which will effect the way in which the allocator functions</param>
	/// <param name="debugReportFlags">A set of debug report flags which will effect the way in which the allocator providing debug logging</param>
	/// <param name="frameInUseCount">Maximum number of additional frames that are in use at the same time as current frame</param>
	/// <param name="pHeapSizeLimit">Either NULL or a pointer to an array of limits on maximum number of bytes that can be allocated out of particular Vulkan memory heap</param>
	/// <param name="pAllocationCallbacks">Custom CPU memory allocation callbacks</param>
	/// <param name="pDeviceMemoryCallbacks">Informative callbacks for vkAllocateMemory, vkFreeMemory</param>
	AllocatorCreateInfo(pvrvk::Device& device, pvrvk::DeviceSize preferredLargeHeapBlockSize = 0, AllocatorCreateFlags flags = AllocatorCreateFlags::e_NONE,
		DebugReportFlags debugReportFlags = DebugReportFlags::None, uint32_t frameInUseCount = 0, const pvrvk::DeviceSize* pHeapSizeLimit = nullptr,
		const pvrvk::AllocationCallbacks* pAllocationCallbacks = nullptr, const DeviceMemoryCallbacks* pDeviceMemoryCallbacks = nullptr)
		: flags(flags), device(device), preferredLargeHeapBlockSize(preferredLargeHeapBlockSize), pAllocationCallbacks(pAllocationCallbacks),
		  pDeviceMemoryCallbacks(pDeviceMemoryCallbacks), frameInUseCount(frameInUseCount), pHeapSizeLimit(pHeapSizeLimit), reportFlags(debugReportFlags)
	{
		if (device->getEnabledExtensionTable().khrDedicatedAllocationEnabled && device->getEnabledExtensionTable().khrGetMemoryRequirements2Enabled)
		{ flags |= AllocatorCreateFlags::e_KHR_DEDICATED_ALLOCATION_BIT; }
	}
};

/// <summary>Optional configuration parameters to be passed to function vmaDefragment().</summary>
struct DefragmentationInfo : private impl::VmaDefragmentationInfo
{
	/// <summary>Maximum total numbers of bytes that can be copied while moving allocations to different places.
	/// Default is `VK_WHOLE_SIZE`, which means no limit.</summary>
	/// <returns>The maximum total number of bytes which can be copied while moving allocations</returns>
	pvrvk::DeviceSize getMaxBytesToMove() const { return maxBytesToMove; }
	/// <summary>Maximum number of allocations that can be moved to different place.
	/// Default is `UINT32_MAX`, which means no limit.</summary>
	/// <returns>The maximum number of allocation which can be moved</returns>
	uint32_t getMaxAllocationsToMove() const { return maxAllocationsToMove; }

	/// <summary>Setter for controlling the maximum number of bytes which can be copied when moving allocations.</summary>
	/// <param name="bytesToMove">The maximum number of bytes which can be copied when moving allocations</param>
	/// <returns>This - allows chaining</returns>
	DefragmentationInfo& setMaxBytesToMove(pvrvk::DeviceSize bytesToMove)
	{
		maxBytesToMove = bytesToMove;
		return *this;
	}

	/// <summary>Setter for controlling the maximum number of allocations that can be moved to different place.</summary>
	/// <param name="allocationToMove">The maximum number of allocations that can be moved to different place</param>
	/// <returns>This - allows chaining</returns>
	DefragmentationInfo& setMaxAllocationsToMove(uint32_t allocationToMove)
	{
		maxAllocationsToMove = allocationToMove;
		return *this;
	}
};

/// <summary>Calculated statistics of memory usage in entire allocator.</summary>
struct StatInfo : private impl::VmaStatInfo
{
public:
	/// <summary>Getter for the number of `VkDeviceMemory` Vulkan memory blocks allocated.</summary>
	/// <returns>The number of `VkDeviceMemory` Vulkan memory blocks allocated</returns>
	uint32_t getBlockCount() const { return blockCount; }
	/// <summary>Getter for the number of `VmaAllocation` allocation objects allocated.</summary>
	/// <returns>The number of `VmaAllocation` allocation objects allocated.</returns>
	uint32_t getAllocationCount() const { return allocationCount; }
	/// <summary>Getter for the number of free ranges of memory between allocations.</summary>
	/// <returns>The number of free ranges of memory between allocations.</returns>
	uint32_t getUnusedRangeCount() const { return unusedRangeCount; }
	/// <summary>Getter for the total number of bytes occupied.</summary>
	/// <returns>The total number of bytes occupied</returns>
	VkDeviceSize getUsedBytes() const { return usedBytes; }
	/// <summary>Getter for the total number of bytes occupied by unused ranges.</summary>
	/// <returns>The total number of bytes occupied by unused ranges</returns>
	VkDeviceSize getUnusedBytes() const { return unusedBytes; }
	/// <summary>Getter for the minimum allocation size.</summary>
	/// <returns>The minimum allocation size</returns>
	VkDeviceSize getAllocationSizeMin() const { return allocationSizeMin; }
	/// <summary>Getter for the average allocation size.</summary>
	/// <returns>The average allocation size</returns>
	VkDeviceSize getAllocationSizeAvg() const { return allocationSizeAvg; }
	/// <summary>Getter for the maximum allocation size.</summary>
	/// <returns>The maximum allocation size</returns>
	VkDeviceSize getAllocationSizeMax() const { return allocationSizeMax; }
	/// <summary>Getter for the minimum number of bytes occupied by unused ranges.</summary>
	/// <returns>The minimum number of bytes occupied by unused ranges</returns>
	VkDeviceSize getUnusedRangeSizeMin() const { return unusedRangeSizeMin; }
	/// <summary>Getter for the average number of bytes occupied by unused ranges.</summary>
	/// <returns>The average number of bytes occupied by unused ranges</returns>
	VkDeviceSize getUnusedRangeSizeAvg() const { return unusedRangeSizeAvg; }
	/// <summary>Getter for the maximum number of bytes occupied by unused ranges.</summary>
	/// <returns>The maximum number of bytes occupied by unused ranges</returns>
	VkDeviceSize getUnusedRangeSizeMax() const { return unusedRangeSizeMax; }
};

/// <summary>General statistics from current state of Allocator.</summary>
struct Stats
{
	friend class impl::Allocator_;
	/// <summary>The set of memory types supported.</summary>
	StatInfo memoryType[VK_MAX_MEMORY_TYPES];
	/// <summary>The set of memory heaps supported.</summary>
	StatInfo memoryHeap[VK_MAX_MEMORY_HEAPS];
	/// <summary>The total set of statistics.</summary>
	StatInfo total;

private:
	Stats(const impl::VmaStats& vmaStats)
	{
		memcpy(memoryType, vmaStats.memoryType, sizeof(vmaStats.memoryType));
		memcpy(memoryHeap, vmaStats.memoryHeap, sizeof(vmaStats.memoryHeap));
		memcpy(&total, &vmaStats.total, sizeof(vmaStats.total));
	}
};

/// <summary>Statistics returned by function vmaDefragment().</summary>
struct DefragmentationStats : impl::VmaDefragmentationStats
{
	/// <summary>Total number of bytes that have been copied while moving allocations to different places.</summary>
	pvrvk::DeviceSize getBytesMoved() const;

	/// <summary>Total number of bytes that have been released to the system by freeing empty `pvrvk::DeviceMemory` objects.</summary>
	pvrvk::DeviceSize getBytesFreed() const;

	/// <summary>Number of allocations that have been moved to different places.</summary>
	uint32_t getAllocationsMoved() const;

	/// <summary>Number of empty `pvrvk::DeviceMemory` objects that have been released to the system.</summary>
	uint32_t getDeviceMemoryBlocksFreed() const;
};

/// <summary>Describes parameter of existing `VmaPool`.</summary>
struct PoolStats : private impl::VmaPoolStats
{
public:
	/// <summary>Total number of bytes in the pool not used by any `Allocation`.</summary>
	/// <returns>Returns the number of bytes not used by any allocation.</returns>
	pvrvk::DeviceSize getUnusedSize() const;

	/// <summary>Number of VmaAllocation objects created from this pool that were not destroyed or lost.</summary>
	/// <returns>Returns the number of VmaAllocation objects created from this pool.</returns>
	size_t getAllocationCount() const;

	/// <summary>Number of continuous memory ranges in the pool not used by any `VmaAllocation`.</summary>
	/// <returns>Returns the number of continuous memory ranges in the pool not used by any `VmaAllocation`.</returns>
	pvrvk::DeviceSize getUnusedRangeSizeMax() const;

	/// <summary>Size of the largest continuous free memory region.
	/// Making a new allocation of that size is not guaranteed to succeed because of
	/// possible additional margin required to respect alignment and buffer/image
	/// granularity.</summary>
	/// <returns>Returns the size of the largest continuous free memory region.</returns>
	size_t getUnusedRangeCount() const;

	/// <summary>Total amount of `pvrvk::DeviceMemory` allocated from Vulkan for this pool, in bytes.</summary>
	/// <returns>Returns the total amount of `pvrvk::DeviceMemory` allocated from Vulkan for this pool, in bytes</returns>
	pvrvk::DeviceSize getSize() const;
};

/// <summary>PoolCreateInfo</summary>
struct PoolCreateInfo
{
	/// <summary>Vulkan memory type index to allocate this pool from.</summary>
	uint32_t memoryTypeIndex;

	/// <summary>Use combination of `PoolCreateFlags`.</summary>
	PoolCreateFlags flags;

	/// <summary>Size of a single `pvrvk::DeviceMemory` block to be allocated as part of this pool, in bytes.
	/// Optional. Leave 0 to use default.</summary>
	pvrvk::DeviceSize blockSize;

	/// <summary>Minimum number of blocks to be always allocated in this pool, even if they stay empty.
	/// Set to 0 to have no preallocated blocks and let the pool be completely empty.</summary>
	size_t minBlockCount;

	/// <summary>Maximum number of blocks that can be allocated in this pool.
	/// Optional. Set to 0 to use `SIZE_MAX`, which means no limit.
	/// Set to same value as minBlockCount to have fixed amount of memory allocated
	/// throuout whole lifetime of this pool.</summary>
	size_t maxBlockCount;

	/// <summary>Maximum number of additional frames that are in use at the same time as current frame.
	/// This value is used only when you make allocations with
	/// `AllocationCreateFlags::e_CAN_BECOME_LOST_BIT` flag. Such allocation cannot become
	/// lost if allocation.lastUseFrameIndex >= allocator.currentFrameIndex - frameInUseCount.
	/// For example, if you double-buffer your command buffers, so resources used for
	/// rendering in previous frame may still be in use by the GPU at the moment you
	/// allocate resources needed for the current frame, set this value to 1.
	/// If you want to allow any allocations other than used in the current frame to
	/// become lost, set this value to 0.</summary>
	uint32_t frameInUseCount;

	/// <summary>Constructor for a pool creation info structure.</summary>
	PoolCreateInfo() : memoryTypeIndex(static_cast<uint32_t>(-1)), flags(PoolCreateFlags(0)), blockSize(0), minBlockCount(0), maxBlockCount(0), frameInUseCount(0) {}

	/// <summary>Constructor for a pool creation info structure.</summary>
	/// <param name="memoryTypeIndex">Vulkan memory type index to allocate this pool from</param>
	/// <param name="flags">A set of PoolCreateFlags</param>
	/// <param name="blockSize">Size of a single `pvrvk::DeviceMemory` block to be allocated as part of this pool, in bytes</param>
	/// <param name="minBlockCount">Minimum number of blocks to be always allocated in this pool, even if they stay empty.</param>
	/// <param name="maxBlockCount">Maximum number of blocks that can be allocated in this pool.</param>
	/// <param name="frameInUseCount">Maximum number of additional frames that are in use at the same time as current frame.</param>
	PoolCreateInfo(uint32_t memoryTypeIndex, PoolCreateFlags flags, pvrvk::DeviceSize blockSize = 0, size_t minBlockCount = 0, size_t maxBlockCount = 0, uint32_t frameInUseCount = 0)
		: memoryTypeIndex(memoryTypeIndex), flags(flags), blockSize(blockSize), minBlockCount(minBlockCount), maxBlockCount(maxBlockCount), frameInUseCount(frameInUseCount)
	{}
};
namespace impl {
/// <summary>An embedded ref counted Pool class</summary>
class Pool_
{
private:
	friend class pvr::utils::vma::impl::Allocator_;

	class make_shared_enabler
	{
	public:
		make_shared_enabler() {}
		friend class Pool_;
	};

	static Pool constructShared(const PoolCreateInfo& poolCreateInfo) { return std::make_shared<Pool_>(make_shared_enabler{}, poolCreateInfo); }

	Allocator _allocator;
	VmaPool _vmaPool;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Pool_)

	Pool_(make_shared_enabler, const PoolCreateInfo& createInfo);
	~Pool_();
	//!\endcond

	/// <summary>Get pool stats</summary>
	/// <returns>Returns pool stats</returns>
	PoolStats getStats() const;

	/// <summary>Mark all allocations from this pool lost as if they are not used in
	/// current frame or VmaPoolCreateInfo::frameInUseCount back from now.</summary>
	/// <returns>Returns number of allocations marked as lost.</returns>
	size_t makeAllocationsLost();
};

/// <summary>A wrapper for device memory</summary>
class DeviceMemoryWrapper_ : public pvrvk::impl::DeviceMemory_
{
private:
	friend class Allocator_;

	class make_shared_enabler : public DeviceMemory_::make_shared_enabler
	{
	protected:
		make_shared_enabler() : DeviceMemory_::make_shared_enabler() {}
		friend class DeviceMemoryWrapper_;
	};

	static DeviceMemoryWrapper constructShared(const pvrvk::DeviceWeakPtr& device, const pvrvk::MemoryAllocationInfo& allocationInfo, pvrvk::MemoryPropertyFlags memPropFlags,
		VkDeviceMemory vkMemoryHandle = VK_NULL_HANDLE)
	{
		return std::make_shared<DeviceMemoryWrapper_>(make_shared_enabler{}, device, allocationInfo, memPropFlags, vkMemoryHandle);
	}

public:
	//!\cond NO_DOXYGEN
	DeviceMemoryWrapper_(make_shared_enabler, const pvrvk::DeviceWeakPtr& device, const pvrvk::MemoryAllocationInfo& allocationInfo, pvrvk::MemoryPropertyFlags memPropFlags,
		VkDeviceMemory vkMemoryHandle)
		: pvrvk::impl::DeviceMemory_(make_shared_enabler{}, device, allocationInfo, memPropFlags, vkMemoryHandle)
	{}
	virtual ~DeviceMemoryWrapper_()
	{
		_vkHandle = VK_NULL_HANDLE; // avoid pvrvk::impl::DeviceMemory_ from calling vkFreeMemory
	}

	/// <summary>Mapping device memory directly is not supported via VMA and instead the device memory should be mapped using the
	/// corresponding Device Memory's device memory allocation.</summary>
	virtual void* map(VkDeviceSize /*offset*/, VkDeviceSize /*size*/, pvrvk::MemoryMapFlags /*memoryMapFlags*/)
	{
		throw std::runtime_error("VMA DeviceMemory cannot be mapped, Use Allocation map");
	}

	/// <summary>Unmapping device memory directly is not supported via VMA and instead the device memory should be unmapped using the
	/// corresponding Device Memory's device memory allocation.</summary>
	virtual void unmap() { throw std::runtime_error("VMA DeviceMemory cannot be unmapped, Use Allocation unmap"); }
	//!\endcond
};

/// <summary>The DeviceMemoryWrapper_. Class Just wraps the Vulkan device memory object allocated by the memory allocatpr.
/// This class doesn't manages the creation and destruction of vulkan object. It only serves as the
/// interface to device memory functions.</summary>
class Allocation_ : public pvrvk::impl::IDeviceMemory_
{
private:
	friend class pvr::utils::vma::impl::Allocator_;

	class make_shared_enabler
	{
	private:
		make_shared_enabler() {}
		friend class Allocation_;
	};

	static Allocation constructShared(Allocator& memAllocator, const AllocationCreateInfo& allocCreateInfo, VmaAllocation vmaAllocation, const VmaAllocationInfo& allocInfo)
	{
		return std::make_shared<Allocation_>(make_shared_enabler{}, memAllocator, allocCreateInfo, vmaAllocation, allocInfo);
	}

	void recalculateOffsetAndSize(VkDeviceSize& offset, VkDeviceSize& size) const;

	void updateAllocationInfo() const;
	Pool _pool;
	Allocator _memAllocator;
	VmaAllocation _vmaAllocation;
	mutable VmaAllocationInfo _allocInfo;
	AllocationCreateFlags _createFlags;
	pvrvk::MemoryPropertyFlags _flags;
	pvrvk::DeviceSize _mappedSize;
	pvrvk::DeviceSize _mappedOffset;

public:
	//!\cond NO_DOXYGEN
	~Allocation_();
	Allocation_(make_shared_enabler, Allocator& memAllocator, const AllocationCreateInfo& allocCreateInfo, VmaAllocation vmaAllocation, const VmaAllocationInfo& allocInfo);
	//!\endcond

	/// <summary>Return true if this memory block is mappable by the host (const).</summary>
	/// <returns>True is this memory block can be mapped, otherwise false.</returns>
	bool isMappable() const;

	/// <summary>Return the memory flags(const)</summary>
	/// <returns>pvrvk::MemoryPropertyFlags</returns>
	pvrvk::MemoryPropertyFlags getMemoryFlags() const;

	/// <summary>Return the memory type</summary>
	/// <returns>The memory type of the pvrvk::Allocation</returns>
	uint32_t getMemoryType() const;

	/// <summary>Map the memory allocation. Also you can directly call map on the device memory on you own if you want
	/// But using this function makes sures correct offset and size is always specified.
	/// Therefore map and unmap of this object is recommended.
	/// Do not use it on memory allocated with AllocationCreateFlags::e_PERSISTENT_MAP_BIT as multiple maps to same DeviceMemory is illegal.</summary>
	/// <param name="offset">The offset into the device memory to map</param>
	/// <param name="size">The size of the returned mapped data</param>
	/// <param name="memoryMapFlags">Memory mapping flags specifying how the mappng will take place</param>
	/// <returns>Returned mapped data</returns>
	void* map(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE, pvrvk::MemoryMapFlags memoryMapFlags = pvrvk::MemoryMapFlags::e_NONE);

	/// <summary>Function unmaps the memory previously mapped by the mapMemory function</summary>
	void unmap();

	/// <summary>Return true if this memory is being mapped by the host (const)</summary>
	/// <returns>VkDeviceSize</returns
	bool isMapped() const;

	/// <summary>Get offset into deviceMemory object to the beginning of this allocation, in bytes.
	/// (deviceMemory, offset) pair is unique to this allocation.</summary>
	/// <returns>Returns device offset</returns>
	pvrvk::DeviceSize getOffset() const;

	/// <summary>Flush ranges of non-coherent memory from the host caches.</summary>
	/// <param name="offset">The offset into the device memory to map</param>
	/// <param name="size">The size of the returned mapped data</param>
	void flushRange(pvrvk::DeviceSize offset = 0, pvrvk::DeviceSize size = VK_WHOLE_SIZE);

	/// <summary>Returns a pointer to the beginning of this allocation as mapped data.</summary>
	/// Null if this allocation is not persistently mapped.
	/// It can change after call to unmapPersistentlyMappedMemory(), mapPersistentlyMappedMemory()
	/// from the memory allocator. Also it can change after call to defragment() if this allocation
	/// is passed to the function.
	/// <returns>Returns the mapped data</returns>
	void* getMappedData();

	/// <summary>Get the user data</summary>
	/// <returns>Returns the user data</returns>
	void* getUserData();

	/// <summary>Sets userData of this allocation to new value.</summary>
	/// <param name="userData">The new data to srt</param>
	void setUserData(void* userData);

	/// <summary>Check if this allocation is lost. Allocation created with AllocationCreateFlags::e_CAN_BECOME_LOST_BIT flag
	/// can become lost as a result of another allocation with AllocationCreateFlags::e_CAN_MAKE_OTHER_LOST_BIT flag, so you must check it before use</summary>
	/// <returns>Returns true if the allocation is lost.</returns>
	bool isAllocationLost() const;

	/// <summary>Get this allocation create flags</summary>
	/// <returns>The allocation creation flags.</returns>
	AllocationCreateFlags getCreateFlags() const;

	/// <summary>Check if this allocation can become lost</summary>
	/// <returns>Returns true if this allocation can become lost</returns>
	bool canBecomeLost() const;

	/// <summary>Return this mapped memory offset (const)</summary>
	/// <returns>The offset into the device memory which has been mapped</returns>
	VkDeviceSize getMappedOffset() const;

	/// <summary>Return this mapped memory size (const)</summary>
	/// <returns>The size of the mapped memory</returns>
	VkDeviceSize getMappedSize() const;

	/// <summary>Return this memory size (const)</summary>
	/// <returns>The size of the device memory</returns>
	VkDeviceSize getSize() const;

	/// <summary>Return this memory pool</summary>
	/// <returns>The memory pool</returns>
	Pool getMemoryPool();

	/// <summary>Invalidates ranges of non-coherent memory from the host caches.</summary>
	/// <param name="offset">The offset into the device memory to map</param>
	/// <param name="size">The size of the returned mapped data</param>
	void invalidateRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
};

/// <summary>The MemoryAllocator_ class./summary>
class Allocator_ : public std::enable_shared_from_this<Allocator_>
{
private:
	friend class AllocatorCreateFactory;
	friend class Pool_;
	friend class Allocation_;
	friend class DeviceMemoryCallbackDispatcher_;

	class make_shared_enabler
	{
	public:
		make_shared_enabler() {}
		friend class Allocator_;
	};

	static Allocator constructShared(const AllocatorCreateInfo& createInfo) { return std::make_shared<Allocator_>(make_shared_enabler{}, createInfo); }

	pvrvk::DeviceWeakPtr _device;
	VmaAllocator _vmaAllocator;
	mutable std::vector<pvrvk::DeviceMemory> _deviceMemory;
	DebugReportFlags _reportFlags;

	Allocation createMemoryAllocation(const AllocationCreateInfo& allocCreateInfo, const VmaAllocationInfo& allocInfo, VmaAllocation vmaAllocation);
	void onAllocateDeviceMemoryFunction(uint32_t memoryType, VkDeviceMemory memory, pvrvk::DeviceSize size);
	void onFreeDeviceMemoryFunction(uint32_t memoryType, VkDeviceMemory memory, pvrvk::DeviceSize size);

	DeviceMemoryCallbacks _deviceMemCallbacks;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Allocator_)

	~Allocator_();
	Allocator_(make_shared_enabler, const AllocatorCreateInfo& createInfo);
	//!\endcond

	/// <summary>Adds a callback dispatcher.</summary>
	void addCallbackDispatcherContext();

	/// <summary>Allocate memory for an image.</summary>
	/// <param name="image">The image to allocate memeory for</param>
	/// <param name="allocCreateInfo">Allocation create info</param>
	/// <returns>Returns the successfull memory allocation.</returns>
	Allocation allocateMemoryForImage(pvrvk::Image& image, const AllocationCreateInfo& allocCreateInfo);

	/// <summary>Allocate memory for buffer. NOTE: It is the calle's responsibility of the allocation's lifetime.</summary>
	/// <param name="buffer">The buffer the allocation is for</param>
	/// <param name="createInfo">allocation create info</param>
	/// <returns>A successfully allocated allocation</returns>
	Allocation allocateMemoryForBuffer(pvrvk::Buffer& buffer, const AllocationCreateInfo& createInfo);

	/// <summary>Alloocate memory. Note: It is users resposibility to keep the
	/// lifetime of the allocation returned from this function call.</summary>
	/// <param name="vkMemoryRequirements">Memory requirement</param>
	/// <param name="createInfo">Allcoation create info</param>
	/// <returns>The successfull allocation.</returns>
	Allocation allocateMemory(const pvrvk::MemoryRequirements* vkMemoryRequirements, const AllocationCreateInfo& createInfo);

	/// <summary>Create memory pool object</summary>
	/// <param name="poolCreateInfo">Pool allcoation create info</param>
	/// <returns>The successfully created Pool object.</returns>
	Pool createPool(const PoolCreateInfo& poolCreateInfo);

	/// <summary>Finds the memory type index for a particular allocation</summary>
	/// <param name="memoryTypeBits">The set of bits required for a successfull allocation</param>
	/// <param name="allocationCreateInfo">Allocation creation info which will contaol the way in which the allocation will be used</param>
	/// <param name="outMemoryTypeIndex">The returned memory type index</param>
	void findMemoryTypeIndex(uint32_t memoryTypeBits, const AllocationCreateInfo& allocationCreateInfo, uint32_t& outMemoryTypeIndex);

	/// <summary>Create buffer with memory allocation</summary>
	/// <param name="createInfo">Buffer creation information</param>
	/// <param name="allocationCreateInfo">Allocation creation info which will control the way in which the allocation will be used</param>
	/// <returns>The successfully created buffer.</returns>
	pvrvk::Buffer createBuffer(const pvrvk::BufferCreateInfo& createInfo, const AllocationCreateInfo& allocationCreateInfo);

	/// <summary>Create image with memory allocation</summary>
	/// <param name="createInfo">Image creation information</param>
	/// <param name="allocationCreateInfo">Allocation creation info which will control the way in which the allocation will be used</param>
	/// <returns>The successfully created image.</returns>
	pvrvk::Image createImage(const pvrvk::ImageCreateInfo& createInfo, const AllocationCreateInfo& allocationCreateInfo);

	/// <summary>Defragment the memory allocations. This function can move allocaion to compact used memory, ensure more continuous free space and possibly also free some
	/// DeviceMemory. It can work only on allocations made from memory type that is HOST_VISIBLE. Allocations are modified to point to the new DeviceMemory and offset. Data in this
	/// memory is also memmove-ed to the new place. However, if you have images or buffers bound to these allocations, you need to destroy, recreate, and bind them to the new place
	/// in memory.</summary>
	/// <param name="memAllocations">A pointer to a set of device memory allocations to defragment</param>
	/// <param name="numAllocations">The number of device memory allocations pointed to by memAllocations</param>
	/// <param name="defragInfo">VmaDefragmentationInfo controlling the defragment operation</param>
	/// <param name="outAllocationsChanged">An array of boolean values specifying whether the corresponding memory allocation in the array pointed to by memAllocations has been
	/// defragmented</param>
	/// <param name="outDefragStatus">A set of DefragmentationStats</param>
	void defragment(
		Allocation* memAllocations, uint32_t numAllocations, const VmaDefragmentationInfo* defragInfo, pvrvk::Bool32* outAllocationsChanged, DefragmentationStats* outDefragStatus);

	/// <summary>Getter for the allocator's device</summary>
	/// <returns>The device.</returns>
	pvrvk::DeviceWeakPtr getDevice();

	/// <summary>Create and returns and memory statistics map</summary>
	/// <param name="detailedMap">Specifies whether a defailed statistics map should be created</param>
	/// <returns>The statistics map.</returns>
	std::string buildStatsString(bool detailedMap)
	{
		char* _statsString;
		vmaBuildStatsString(_vmaAllocator, &_statsString, detailedMap);
		const std::string statStr(_statsString);
		vmaFreeStatsString(_vmaAllocator, _statsString);
		return statStr;
	}

	/// <summary>Create and returns and memory statistics</summary>
	/// <returns>The statistics map.</returns>
	Stats calculateStats() const
	{
		impl::VmaStats vmaStats;
		vmaCalculateStats(_vmaAllocator, &vmaStats);
		return Stats(vmaStats);
	}
};

/// <summary>Creates a Pool</summary>
/// <param name="poolCreateInfo">Specifies how the created pool will be created</param>
/// <returns>The created Memory Pool.</returns>
inline Pool Allocator_::createPool(const PoolCreateInfo& poolCreateInfo) { return Pool_::constructShared(poolCreateInfo); }

/// <summary>Getter for the allocator's device</summary>
/// <returns>The device.</returns>
inline pvrvk::DeviceWeakPtr Allocator_::getDevice() { return _device; }

inline Allocation Allocator_::createMemoryAllocation(const AllocationCreateInfo& allocCreateInfo, const VmaAllocationInfo& allocInfo, VmaAllocation vmaAllocation)
{
	Allocator allocator = shared_from_this();
	Allocation allocation = Allocation_::constructShared(allocator, allocCreateInfo, vmaAllocation, allocInfo);
	if (uint32_t(_reportFlags & DebugReportFlags::Allocation) != 0)
	{
		Log(LogLevel::Debug, "VMA: New Allocation 0x%llx: DeviceMemory 0x%llx, MemoryType %d, Offset %lu bytes, Size %lu bytes", allocation->_vmaAllocation, allocInfo.deviceMemory,
			allocInfo.memoryType, allocInfo.offset, allocInfo.size);
	}
	return allocation;
}

inline void Allocator_::onAllocateDeviceMemoryFunction(uint32_t memoryType, VkDeviceMemory memory, pvrvk::DeviceSize size)
{
	VkMemoryPropertyFlags memProp;
	vmaGetMemoryTypeProperties(_vmaAllocator, memoryType, &memProp);
	DeviceMemoryWrapper deviceMemory =
		DeviceMemoryWrapper_::constructShared(getDevice(), pvrvk::MemoryAllocationInfo(size, memoryType), static_cast<pvrvk::MemoryPropertyFlags>(memProp), memory);
	_deviceMemory.emplace_back(deviceMemory);

	if (uint32_t(_reportFlags & DebugReportFlags::DeviceMemory) != 0)
	{ Log(LogLevel::Debug, "VMA: New DeviceMemory 0x%llx, MemoryType %d, Size %lu bytes", memory, memoryType, size); }
	if (_deviceMemCallbacks.pfnAllocate) { _deviceMemCallbacks.pfnAllocate(shared_from_this(), memoryType, _deviceMemory.back(), size); }
}

inline void Allocator_::onFreeDeviceMemoryFunction(uint32_t memoryType, VkDeviceMemory memory, pvrvk::DeviceSize size)
{
	if (uint32_t(_reportFlags & DebugReportFlags::DeviceMemory) != 0)
	{ Log(LogLevel::Debug, "VMA: Freed DeviceMemory 0x%llx: MemoryType %d, Size %lu bytes", memory, memoryType, size); }
	if (_deviceMemCallbacks.pfnFree)
	{
		auto it = std::find_if(_deviceMemory.begin(), _deviceMemory.end(), [&](const pvrvk::DeviceMemory& deviceMemory) { return deviceMemory->getVkHandle() == memory; });
		if (it != _deviceMemory.end()) { _deviceMemCallbacks.pfnAllocate(shared_from_this(), memoryType, *it, size); }
	}
}

inline uint32_t Allocation_::getMemoryType() const { return _allocInfo.memoryType; }

inline bool Allocation_::isMapped() const { return _mappedSize > 0; }

inline bool Allocation_::isMappable() const
{
	return (static_cast<uint32_t>(getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0) ||
		(static_cast<uint32_t>(getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0);
}

inline pvrvk::MemoryPropertyFlags Allocation_::getMemoryFlags() const { return _flags; }

inline pvrvk::DeviceSize Allocation_::getOffset() const { return _allocInfo.offset; }

inline void Allocation_::flushRange(pvrvk::DeviceSize offset, pvrvk::DeviceSize size)
{
	recalculateOffsetAndSize(offset, size);
	if (static_cast<uint32_t>(_flags & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0)
	{
		Log(LogLevel::Warning,
			"Flushing allocation 0x%llx from memory block 0x%llx"
			" created using HOST_COHERENT_BIT memory flags - this is unnecessary.",
			_vmaAllocation, getVkHandle());
	}

	VkMappedMemoryRange range = {};
	range.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_MAPPED_MEMORY_RANGE);
	range.memory = getVkHandle();
	range.offset = offset;
	range.size = size;
	pvrvk::impl::vkThrowIfFailed(_device.lock()->getVkBindings().vkFlushMappedMemoryRanges(_device.lock()->getVkHandle(), 1, &range), "Failed to flush range of memory block");
}

inline void Allocation_::invalidateRange(VkDeviceSize offset, VkDeviceSize size)
{
	recalculateOffsetAndSize(offset, size);
	if (static_cast<uint32_t>(_flags & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) != 0)
	{
		Log(LogLevel::Warning,
			"Invalidating range of an allocation 0x%llx from memory block 0x%llx"
			" created using HOST_COHERENT_BIT memory flags - this is unnecessary.",
			_vmaAllocation, getVkHandle());
	}

	VkMappedMemoryRange range = {};
	range.sType = static_cast<VkStructureType>(pvrvk::StructureType::e_MAPPED_MEMORY_RANGE);
	range.memory = getVkHandle();
	range.offset = offset;
	range.size = size;
	pvrvk::impl::vkThrowIfFailed(_device.lock()->getVkBindings().vkInvalidateMappedMemoryRanges(_device.lock()->getVkHandle(), 1, &range), "Failed to invalidate range of memory block");
}

inline void Allocation_::recalculateOffsetAndSize(VkDeviceSize& offset, VkDeviceSize& size) const
{
	offset += getOffset();
	if (size == VK_WHOLE_SIZE) { size = getOffset() + getSize() - offset; }
	assert(size <= _allocInfo.size);
}

inline void* Allocation_::getUserData()
{
	updateAllocationInfo();
	return _allocInfo.pUserData;
}

inline AllocationCreateFlags Allocation_::getCreateFlags() const { return _createFlags; }

inline bool Allocation_::canBecomeLost() const { return static_cast<uint32_t>(_createFlags & AllocationCreateFlags::e_CAN_BECOME_LOST_BIT) != 0; }

inline VkDeviceSize Allocation_::getMappedOffset() const { return _mappedOffset; }

inline VkDeviceSize Allocation_::getMappedSize() const { return _mappedSize; }

inline VkDeviceSize Allocation_::getSize() const { return _allocInfo.size; }

inline Pool Allocation_::getMemoryPool() { return _pool; }
} // namespace impl

inline pvrvk::DeviceSize PoolStats::getUnusedSize() const { return unusedSize; }

inline size_t PoolStats::getAllocationCount() const { return allocationCount; }

inline pvrvk::DeviceSize PoolStats::getUnusedRangeSizeMax() const { return unusedRangeSizeMax; }

inline size_t PoolStats::getUnusedRangeCount() const { return unusedRangeCount; }

inline pvrvk::DeviceSize PoolStats::getSize() const { return size; }

/// <summary>Retrieves DefragmentationStats regarding the bytes which have been moved</summary>
/// <returns>A set of DefragmentationStats corresponding to which bytes which have been moved.</returns>
inline pvrvk::DeviceSize DefragmentationStats::getBytesMoved() const { return bytesMoved; }

/// <summary>Retrieves DefragmentationStats regarding the bytes which have been freed</summary>
/// <returns>A set of DefragmentationStats corresponding to which bytes which have been freed.</returns>
inline pvrvk::DeviceSize DefragmentationStats::getBytesFreed() const { return bytesFreed; }

/// <summary>Retrieves DefragmentationStats regarding the device memory blocks which have been moved</summary>
/// <returns>A set of DefragmentationStats corresponding to which device memory blocks which have been moved.</returns>
inline uint32_t DefragmentationStats::getAllocationsMoved() const { return allocationsMoved; }

/// <summary>Retrieves DefragmentationStats regarding the device memory blocks which have been freed</summary>
/// <returns>A set of DefragmentationStats corresponding to which device memory blocks which have been freed.</returns>
inline uint32_t DefragmentationStats::getDeviceMemoryBlocksFreed() const { return deviceMemoryBlocksFreed; }

/// <summary>Creates a device memory allocator</summary>
/// <param name="createInfo">Specifies how the created device memory allocator will function</param>
/// <returns>The created device memory allocator.</returns>
Allocator createAllocator(const AllocatorCreateInfo& createInfo);
} // namespace vma
} // namespace utils
} // namespace pvr
