/*!
\brief Implementation of functions of the Vulkan Memory Allocator
\file PVRUtils/Vulkan/MemoryAllocator.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#define VMA_IMPLEMENTATION
#include "MemoryAllocator.h"
namespace pvr {
namespace utils {
namespace vma {
namespace impl {
//!\cond NO_DOXYGEN
Allocation Allocator_::allocateMemoryForImage(pvrvk::Image& image, const AllocationCreateInfo& createInfo)
{
	VmaAllocation vmaAllocation;
	VmaAllocationInfo allocInfo;

	VmaAllocationCreateInfo vmaAllocCreateInfo;
	vmaAllocCreateInfo.flags = (VmaAllocationCreateFlags)createInfo.flags;
	vmaAllocCreateInfo.usage = (VmaMemoryUsage)createInfo.usage;
	vmaAllocCreateInfo.requiredFlags = (VkMemoryPropertyFlags)createInfo.requiredFlags;
	vmaAllocCreateInfo.preferredFlags = (VkMemoryPropertyFlags)createInfo.preferredFlags;
	vmaAllocCreateInfo.pUserData = createInfo.pUserData;
	vmaAllocCreateInfo.pool = (createInfo.pool ? createInfo.pool->_vmaPool : VK_NULL_HANDLE);
	vmaAllocCreateInfo.memoryTypeBits = createInfo.memoryTypeBits;

	pvrvk::impl::vkThrowIfError(
		(pvrvk::Result)vmaAllocateMemoryForImage(_vmaAllocator, image->getVkHandle(), &vmaAllocCreateInfo, &vmaAllocation, &allocInfo), "Failed to allocate memory for image");
	return createMemoryAllocation(createInfo, allocInfo, vmaAllocation);
}

Allocation Allocator_::allocateMemoryForBuffer(pvrvk::Buffer& buffer, const AllocationCreateInfo& createInfo)
{
	VmaAllocationInfo vmaAllocationInfo;
	VmaAllocation vmaAllocation;

	VmaAllocationCreateInfo vmaAllocCreateInfo;
	vmaAllocCreateInfo.flags = (VmaAllocationCreateFlags)createInfo.flags;
	vmaAllocCreateInfo.usage = (VmaMemoryUsage)createInfo.usage;
	vmaAllocCreateInfo.requiredFlags = (VkMemoryPropertyFlags)createInfo.requiredFlags;
	vmaAllocCreateInfo.preferredFlags = (VkMemoryPropertyFlags)createInfo.preferredFlags;
	vmaAllocCreateInfo.pUserData = createInfo.pUserData;
	vmaAllocCreateInfo.pool = (createInfo.pool ? createInfo.pool->_vmaPool : VK_NULL_HANDLE);
	vmaAllocCreateInfo.memoryTypeBits = createInfo.memoryTypeBits;

	pvrvk::impl::vkThrowIfError((pvrvk::Result)vmaAllocateMemoryForBuffer(_vmaAllocator, buffer->getVkHandle(), &vmaAllocCreateInfo, &vmaAllocation, &vmaAllocationInfo),
		"Failed to allocate memory for buffer");
	return createMemoryAllocation(createInfo, vmaAllocationInfo, vmaAllocation);
}

Allocation Allocator_::allocateMemory(const pvrvk::MemoryRequirements* vkMemoryRequirements, const AllocationCreateInfo& createInfo)
{
	VmaAllocationInfo vmaAllocationInfo;
	VmaAllocation vmaAllocation;
	VmaAllocationCreateInfo vmaAllocCreateInfo;
	vmaAllocCreateInfo.flags = (VmaAllocationCreateFlags)createInfo.flags;
	vmaAllocCreateInfo.usage = (VmaMemoryUsage)createInfo.usage;
	vmaAllocCreateInfo.requiredFlags = (VkMemoryPropertyFlags)createInfo.requiredFlags;
	vmaAllocCreateInfo.preferredFlags = (VkMemoryPropertyFlags)createInfo.preferredFlags;
	vmaAllocCreateInfo.pUserData = createInfo.pUserData;
	vmaAllocCreateInfo.pool = (createInfo.pool ? createInfo.pool->_vmaPool : VK_NULL_HANDLE);

	pvrvk::impl::vkThrowIfError(
		(pvrvk::Result)vmaAllocateMemory(_vmaAllocator, (const VkMemoryRequirements*)vkMemoryRequirements, &vmaAllocCreateInfo, &vmaAllocation, &vmaAllocationInfo),
		"Failed to Allocate memory");
	return createMemoryAllocation(createInfo, vmaAllocationInfo, vmaAllocation);
} // namespace impl

void Allocator_::findMemoryTypeIndex(uint32_t memoryTypeBits, const AllocationCreateInfo& allocationCreateInfo, uint32_t& outMemoryTypeIndex)
{
	VmaAllocationCreateInfo vmaAllocCreateInfo;
	vmaAllocCreateInfo.flags = (VmaAllocationCreateFlags)allocationCreateInfo.flags;
	vmaAllocCreateInfo.usage = (VmaMemoryUsage)allocationCreateInfo.usage;
	vmaAllocCreateInfo.requiredFlags = (VkMemoryPropertyFlags)allocationCreateInfo.requiredFlags;
	vmaAllocCreateInfo.preferredFlags = (VkMemoryPropertyFlags)allocationCreateInfo.preferredFlags;
	vmaAllocCreateInfo.pool = allocationCreateInfo.pool->_vmaPool;
	pvrvk::impl::vkThrowIfError((pvrvk::Result)vmaFindMemoryTypeIndex(_vmaAllocator, memoryTypeBits, &vmaAllocCreateInfo, &outMemoryTypeIndex), "Failed to Find memory type index");
}

pvrvk::Buffer Allocator_::createBuffer(const pvrvk::BufferCreateInfo& createInfo, const AllocationCreateInfo& allocationCreateInfo)
{
	pvrvk::Buffer outBuffer = _device.lock()->createBuffer(createInfo);
	Allocation allocation = allocateMemoryForBuffer(outBuffer, allocationCreateInfo);
	outBuffer->bindMemory(pvrvk::DeviceMemory(allocation), allocation->getOffset());
	return outBuffer;
}

pvrvk::Image Allocator_::createImage(const pvrvk::ImageCreateInfo& createInfo, const AllocationCreateInfo& allocationCreateInfo)
{
	pvrvk::Image outImage = _device.lock()->createImage(createInfo);
	Allocation allocation = allocateMemoryForImage(outImage, allocationCreateInfo);
	outImage->bindMemoryNonSparse(pvrvk::DeviceMemory(allocation), allocation->getOffset());
	return outImage;
}

void Allocator_::defragment(
	Allocation* memAllocations, uint32_t numAllocations, const VmaDefragmentationInfo* defragInfo, pvrvk::Bool32* outAllocationsChanged, DefragmentationStats* outDefragStatus)
{
	std::vector<VmaAllocation> allocations(numAllocations);
	std::transform(memAllocations, memAllocations + numAllocations, allocations.begin(), [&](Allocation& memAlloc) { return memAlloc->_vmaAllocation; });

	std::vector<VkBool32> allocationChanged(0);
	if (outAllocationsChanged == nullptr)
	{
		allocationChanged.resize(numAllocations);
		outAllocationsChanged = allocationChanged.data();
	}

	DefragmentationStats myDefragStatus;
	if (outDefragStatus == nullptr && uint32_t(_reportFlags & DebugReportFlags::Defragments) != 0) { outDefragStatus = &myDefragStatus; }
	pvrvk::impl::vkThrowIfFailed((::VkResult)vmaDefragment(
		_vmaAllocator, allocations.data(), numAllocations, outAllocationsChanged, (const VmaDefragmentationInfo*)defragInfo, (VmaDefragmentationStats*)outDefragStatus));
	for (uint32_t i = 0; i < numAllocations; ++i)
	{
		if (outAllocationsChanged[i]) { memAllocations[i]->updateAllocationInfo(); }
	}
	if (uint32_t(_reportFlags & DebugReportFlags::Defragments) != 0)
	{
		Log(LogLevel::Debug, "VMA: Defragment Stats:");
		Log(LogLevel::Debug, "VMA: \tNumber of allocations moved %d", outDefragStatus->getAllocationsMoved());
		Log(LogLevel::Debug, "VMA: \tBytes freed %d", outDefragStatus->getBytesFreed());
		Log(LogLevel::Debug, "VMA: \tBytes moved %d", outDefragStatus->getBytesMoved());
		Log(LogLevel::Debug, "VMA: \tMemoryBlocks freed %d", outDefragStatus->getDeviceMemoryBlocksFreed());
	}
}

Allocator_::~Allocator_()
{
	_deviceMemory.clear();
	if (_vmaAllocator != VK_NULL_HANDLE) { vmaDestroyAllocator(_vmaAllocator); }
	_vmaAllocator = VK_NULL_HANDLE;
}

size_t Pool_::makeAllocationsLost()
{
	size_t numLost;
	vmaMakePoolAllocationsLost(_allocator->_vmaAllocator, _vmaPool, &numLost);
	return numLost;
}

Pool_::Pool_(make_shared_enabler, const PoolCreateInfo& createInfo) : _vmaPool(VK_NULL_HANDLE)
{
	pvrvk::impl::vkThrowIfError((pvrvk::Result)vmaCreatePool(_allocator->_vmaAllocator, &(VmaPoolCreateInfo&)createInfo, &_vmaPool), "Failed to create Memory Pool");
}

Pool_::~Pool_()
{
	if (_vmaPool != VK_NULL_HANDLE) { vmaDestroyPool(_allocator->_vmaAllocator, _vmaPool); }
}

PoolStats Pool_::getStats() const
{
	PoolStats stats;
	vmaGetPoolStats(_allocator->_vmaAllocator, _vmaPool, &(VmaPoolStats&)stats);
	return stats;
}

void Allocation_::unmap()
{
	if (!_mappedSize) { throw pvrvk::ErrorMemoryMapFailed("Cannot unmap memory block as the memory is not mapped"); }
	_mappedSize = 0;
	_mappedOffset = 0;
	vmaUnmapMemory(_memAllocator->_vmaAllocator, _vmaAllocation);
}

void* Allocation_::getMappedData()
{
	updateAllocationInfo();
	return (getVkHandle() != VK_NULL_HANDLE ? _allocInfo.pMappedData : nullptr);
}

void Allocation_::setUserData(void* userData) { vmaSetAllocationUserData(_memAllocator->_vmaAllocator, _vmaAllocation, userData); }

Allocation_::~Allocation_()
{
	if (_vmaAllocation == VK_NULL_HANDLE) { return; }
	updateAllocationInfo();
	if (uint32_t(_memAllocator->_reportFlags & DebugReportFlags::Allocation) != 0)
	{
		Log(LogLevel::Debug,
			"VMA: Freed Allocation 0x%llx: DeviceMemory 0x%llx, MemoryType %d, "
			"Offset %lu bytes, Size %lu bytes",
			_vmaAllocation, _allocInfo.deviceMemory, _allocInfo.memoryType, _allocInfo.offset, _allocInfo.size);
	}
	vmaFreeMemory(_memAllocator->_vmaAllocator, _vmaAllocation);
	_vmaAllocation = VK_NULL_HANDLE;
	_vkHandle = VK_NULL_HANDLE;
}

void* Allocation_::map(VkDeviceSize offset, VkDeviceSize size, pvrvk::MemoryMapFlags memoryMapFlags)
{
	(void)memoryMapFlags;
	size_t total_offset = static_cast<size_t>(offset + getOffset());
	if (!isMappable())
	{
		throw pvrvk::ErrorMemoryMapFailed("Cannot map memory block as the memory was created without "
										  "HOST_VISIBLE_BIT or HOST_COHERENT_BIT memory flags");
	}
	if (_mappedSize) { throw pvrvk::ErrorMemoryMapFailed("Cannot map memory block as the memory is already mapped"); }
	if (size != VK_WHOLE_SIZE)
	{
		if ((total_offset + size) > (total_offset + _allocInfo.size))
		{ throw pvrvk::ErrorMemoryMapFailed("Cannot map map memory block : offset + size range greater than the memory block size"); }
	}

	void* mappedMemory = nullptr;

	pvrvk::impl::vkThrowIfFailed((pvrvk::Result)vmaMapMemory(_memAllocator->_vmaAllocator, _vmaAllocation, &mappedMemory), "Failed to map memory block");

	if (mappedMemory == 0) { throw pvrvk::ErrorMemoryMapFailed("Failed to map memory block"); }

	// store the mapped offset and mapped size
	_mappedOffset = offset;
	_mappedSize = size;
	mappedMemory = ((char*)mappedMemory + offset);
	return mappedMemory;
}

bool Allocation_::isAllocationLost() const
{
	if (static_cast<uint32_t>(_createFlags & AllocationCreateFlags::e_CAN_BECOME_LOST_BIT) != 0) { updateAllocationInfo(); }
	return getVkHandle() == VK_NULL_HANDLE;
}

Allocation_::Allocation_(make_shared_enabler, Allocator& memAllocator, const AllocationCreateInfo& allocCreateInfo, VmaAllocation vmaAllocation, const VmaAllocationInfo& allocInfo)
	: IDeviceMemory_(memAllocator->getDevice(), allocInfo.deviceMemory), _mappedSize(0), _mappedOffset(0), _memAllocator(memAllocator)
{
	_vmaAllocation = vmaAllocation;
	_allocInfo = allocInfo;
	_createFlags = allocCreateInfo.flags;
	_pool = allocCreateInfo.pool;
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(_memAllocator->_vmaAllocator, _allocInfo.memoryType, &memFlags);
	_flags = static_cast<pvrvk::MemoryPropertyFlags>(memFlags);
	if (uint32_t(allocCreateInfo.flags & AllocationCreateFlags::e_MAPPED_BIT) != 0)
	{
		_mappedOffset = allocInfo.offset;
		_mappedSize = allocInfo.size;
	}
	_vkHandle = allocInfo.deviceMemory;
}

void Allocation_::updateAllocationInfo() const
{
	// optimize it.
	{
		vmaGetAllocationInfo(_memAllocator->_vmaAllocator, _vmaAllocation, &_allocInfo);
	}
	if (_allocInfo.deviceMemory != getVkHandle())
	{
		VkDeviceMemory& deviceMemory = const_cast<VkDeviceMemory&>(_vkHandle);
		deviceMemory = _allocInfo.deviceMemory;
	}
}

class AllocatorCreateFactory
{
public:
	static Allocator createAllocator(const AllocatorCreateInfo& createInfo) { return Allocator_::constructShared(createInfo); }
};

class DeviceMemoryCallbackDispatcher_
{
public:
	static std::shared_ptr<DeviceMemoryCallbackDispatcher_> getCallbackDispatcher();

private:
	friend class Allocator_;
	static std::vector<AllocatorWeakPtr> _context;

	static void VKAPI_PTR allocateDeviceMemoryFunction(VmaAllocator allocator, uint32_t memoryType, VkDeviceMemory memory, VkDeviceSize size)
	{
		AllocatorWeakPtr context = getDispatchContext(allocator);
		if (!context.expired()) { context.lock()->onAllocateDeviceMemoryFunction(memoryType, memory, size); }
	}

	static void VKAPI_PTR freeDeviceMemoryFunction(VmaAllocator allocator, uint32_t memoryType, VkDeviceMemory memory, VkDeviceSize size)
	{
		AllocatorWeakPtr context = getDispatchContext(allocator);
		if (!context.expired()) { context.lock()->onFreeDeviceMemoryFunction(memoryType, memory, size); }
	}

	static AllocatorWeakPtr getDispatchContext(VmaAllocator allocator)
	{
		auto it = std::find_if(_context.begin(), _context.end(), [&](const AllocatorWeakPtr& context) {
			if (!context.expired()) { return context.lock()->_vmaAllocator == allocator; }
			return false;
		});
		if (it != _context.end()) { return (*it); }
		return AllocatorWeakPtr();
	}

	void addContext(AllocatorWeakPtr memAllocator) { _context.emplace_back(memAllocator); }

	DeviceMemoryCallbacks _callBacks;
};
std::vector<AllocatorWeakPtr> DeviceMemoryCallbackDispatcher_::_context;

std::shared_ptr<DeviceMemoryCallbackDispatcher_> DeviceMemoryCallbackDispatcher_::getCallbackDispatcher()
{
	static std::shared_ptr<DeviceMemoryCallbackDispatcher_> dispatcher = std::make_shared<DeviceMemoryCallbackDispatcher_>();
	return dispatcher;
}

pvr::utils::vma::impl::Allocator_::Allocator_(make_shared_enabler, const AllocatorCreateInfo& createInfo) : _vmaAllocator(VK_NULL_HANDLE)
{
	_device = createInfo.device;
	const VkInstanceBindings& instanceBindings = _device.lock()->getPhysicalDevice()->getInstance()->getVkBindings();

	const VkDeviceBindings& deviceBindings = _device.lock()->getVkBindings();
	const VmaVulkanFunctions vmaFunctions = {
		instanceBindings.vkGetPhysicalDeviceProperties,
		instanceBindings.vkGetPhysicalDeviceMemoryProperties,
		deviceBindings.vkAllocateMemory,
		deviceBindings.vkFreeMemory,
		deviceBindings.vkMapMemory,
		deviceBindings.vkUnmapMemory,
		deviceBindings.vkFlushMappedMemoryRanges,
		deviceBindings.vkInvalidateMappedMemoryRanges,
		deviceBindings.vkBindBufferMemory,
		deviceBindings.vkBindImageMemory,
		deviceBindings.vkGetBufferMemoryRequirements,
		deviceBindings.vkGetImageMemoryRequirements,
		deviceBindings.vkCreateBuffer,
		deviceBindings.vkDestroyBuffer,
		deviceBindings.vkCreateImage,
		deviceBindings.vkDestroyImage,
		deviceBindings.vkCmdCopyBuffer,
		deviceBindings.vkGetBufferMemoryRequirements2KHR,
		deviceBindings.vkGetImageMemoryRequirements2KHR,
	};

	VmaDeviceMemoryCallbacks vmaDeviceMemCallbacks{ DeviceMemoryCallbackDispatcher_::allocateDeviceMemoryFunction, DeviceMemoryCallbackDispatcher_::freeDeviceMemoryFunction };

	if (createInfo.pDeviceMemoryCallbacks) { _deviceMemCallbacks = *createInfo.pDeviceMemoryCallbacks; }
	const VmaAllocatorCreateInfo vmaCreateInfo{ static_cast<VmaAllocatorCreateFlags>(createInfo.flags), _device.lock()->getPhysicalDevice()->getVkHandle(),
		_device.lock()->getVkHandle(), createInfo.preferredLargeHeapBlockSize, (const VkAllocationCallbacks*)createInfo.pAllocationCallbacks, &vmaDeviceMemCallbacks,
		createInfo.frameInUseCount, createInfo.pHeapSizeLimit, &vmaFunctions };
	_reportFlags = createInfo.reportFlags;
	pvrvk::impl::vkThrowIfFailed(::VkResult(vmaCreateAllocator(&vmaCreateInfo, &_vmaAllocator)), "Failed to create memory allocator");
}

void pvr::utils::vma::impl::Allocator_::addCallbackDispatcherContext() { DeviceMemoryCallbackDispatcher_::getCallbackDispatcher()->addContext(shared_from_this()); }
} // namespace impl
Allocator createAllocator(const AllocatorCreateInfo& createInfo)
{
	Allocator outAllocator = impl::AllocatorCreateFactory::createAllocator(createInfo);
	outAllocator->addCallbackDispatcherContext();
	return outAllocator;
}
//!\endcond
} // namespace vma
} // namespace utils
} // namespace pvr
