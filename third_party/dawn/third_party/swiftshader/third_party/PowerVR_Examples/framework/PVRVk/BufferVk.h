/*!
\brief The PVRVk class, an object wrapping memory that is directly(non-image) accessible to the shaders.
\file PVRVk/BufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DeviceMemoryVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Buffer creation descriptor.</summary>
struct BufferCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	BufferCreateInfo()
		: _flags(BufferCreateFlags::e_NONE), _size(0), _sharingMode(SharingMode::e_EXCLUSIVE), _usageFlags(BufferUsageFlags::e_NONE), _numQueueFamilyIndices(0),
		  _queueFamilyIndices(nullptr)
	{}

	/// <summary>Constructor</summary>
	/// <param name="size">The buffer creation size</param>
	/// <param name="usageFlags">The buffer creation usage flags</param>
	/// <param name="flags">The buffer creation flags</param>
	/// <param name="sharingMode">The buffer creation sharing mode</param>
	/// <param name="queueFamilyIndices">A pointer to a list of supported queue families</param>
	/// <param name="numQueueFamilyIndices">The number of supported queue family indices</param>
	BufferCreateInfo(pvrvk::DeviceSize size, pvrvk::BufferUsageFlags usageFlags, pvrvk::BufferCreateFlags flags = pvrvk::BufferCreateFlags::e_NONE,
		SharingMode sharingMode = SharingMode::e_EXCLUSIVE, const uint32_t* queueFamilyIndices = nullptr, uint32_t numQueueFamilyIndices = 0)
		: _flags(flags), _size(size), _sharingMode(sharingMode), _usageFlags(usageFlags), _numQueueFamilyIndices(numQueueFamilyIndices), _queueFamilyIndices(queueFamilyIndices)
	{}

	/// <summary>Get the buffer creation flags</summary>
	/// <returns>The set of buffer creation flags</returns>
	inline BufferCreateFlags getFlags() const { return _flags; }
	/// <summary>Set the buffer creation flags</summary>
	/// <param name="flags">The buffer creation flags</param>
	inline void setFlags(BufferCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get the buffer creation size</summary>
	/// <returns>The set of buffer creation size</returns>
	inline DeviceSize getSize() const { return _size; }
	/// <summary>Set the buffer creation size</summary>
	/// <param name="size">The buffer creation size</param>
	inline void setSize(DeviceSize size) { this->_size = size; }
	/// <summary>Get the buffer creation sharing mode</summary>
	/// <returns>The set of buffer creation sharing mode</returns>
	inline SharingMode getSharingMode() const { return _sharingMode; }
	/// <summary>Set the buffer creation sharing mode</summary>
	/// <param name="sharingMode">The buffer creation sharing mode</param>
	inline void setSharingMode(SharingMode sharingMode) { this->_sharingMode = sharingMode; }
	/// <summary>Get the buffer creation usage flags</summary>
	/// <returns>The set of buffer creation usage flags</returns>
	inline BufferUsageFlags getUsageFlags() const { return _usageFlags; }
	/// <summary>Set the buffer creation usage flags</summary>
	/// <param name="usageFlags">The buffer creation usage flags</param>
	inline void setUsageFlags(BufferUsageFlags usageFlags) { this->_usageFlags = usageFlags; }
	/// <summary>Get the number of queue family indices pointed to</summary>
	/// <returns>The number of queue family indices pointed to</returns>
	inline uint32_t getNumQueueFamilyIndices() const { return _numQueueFamilyIndices; }
	/// <summary>Set the number of queue family indices pointed to</summary>
	/// <param name="numQueueFamilyIndices">The number of queue family indices pointed to</param>
	inline void setNumQueueFamilyIndices(uint32_t numQueueFamilyIndices) { this->_numQueueFamilyIndices = numQueueFamilyIndices; }
	/// <summary>Get this buffer creation infos pointer to a list of supported queue family indices</summary>
	/// <returns>A pointer to a list of supported queue family indices</returns>
	inline const uint32_t* getQueueFamilyIndices() const { return _queueFamilyIndices; }

private:
	/// <summary>Flags to use for creating the buffer</summary>
	BufferCreateFlags _flags;
	/// <summary>The size of the buffer in bytes</summary>
	DeviceSize _size;
	/// <summary>Specifies the buffer sharing mode specifying how the buffer can be used by multiple queue families</summary>
	SharingMode _sharingMode;
	/// <summary>describes the buffer's intended usage</summary>
	BufferUsageFlags _usageFlags;
	/// <summary>The number of queue families in the _queueFamilyIndices array</summary>
	uint32_t _numQueueFamilyIndices;
	/// <summary>The list of queue families that will access this buffer</summary>
	const uint32_t* _queueFamilyIndices;
};

/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Vulkan implementation of the Buffer.</summary>
class Buffer_ : public PVRVkDeviceObjectBase<VkBuffer, ObjectType::e_BUFFER>, public DeviceObjectDebugUtils<Buffer_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Buffer_;
	};

	static Buffer constructShared(const DeviceWeakPtr& device, const BufferCreateInfo& createInfo) { return std::make_shared<Buffer_>(make_shared_enabler{}, device, createInfo); }

	BufferCreateInfo _createInfo;
	MemoryRequirements _memRequirements;
	VkDeviceSize _memoryOffset;
	DeviceMemory _deviceMemory;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Buffer_)
	Buffer_(make_shared_enabler, const DeviceWeakPtr& device, const BufferCreateInfo& createInfo);

	/// <summary>Destructor. Checks if the device is valid</summary>
	~Buffer_();
	//!\endcond

	/// <summary>Return the DeviceMemory bound to this buffer. Note</summary>: only nonsparse buffer can have a bound memory block
	/// <returns>MemoryBlock</returns>
	DeviceMemory& getDeviceMemory() { return _deviceMemory; }

	/// <summary>Get this buffer's creation flags</summary>
	/// <returns>The set of buffer creation flags</returns>
	inline BufferCreateFlags getFlags() const { return _createInfo.getFlags(); }

	/// <summary>Indicates whether the buffers creation flags includes the given flag</summary>
	/// <param name="flags">A buffer creation flag</param>
	/// <returns>True if the buffers creation flags includes the given flag</returns>
	inline bool hasCreateFlag(pvrvk::BufferCreateFlags flags) const { return (_createInfo.getFlags() & flags) == flags; }

	/// <summary>Indicates whether the buffers usage flags includes the given flag</summary>
	/// <param name="flags">A buffer usage flag</param>
	/// <returns>True if the buffers usage flags includes the given flag</returns>
	inline bool hasUsageFlag(pvrvk::BufferUsageFlags flags) const { return (_createInfo.getUsageFlags() & flags) == flags; }

	/// <summary>Get this buffer's size</summary>
	/// <returns>The size of this buffer</returns>
	inline DeviceSize getSize() const { return _createInfo.getSize(); }

	/// <summary>Get this buffer's supported sharing mode</summary>
	/// <returns>A SharingMode structure specifying this buffer's supported sharing mode</returns>
	inline SharingMode getSharingMode() const { return _createInfo.getSharingMode(); }

	/// <summary>Get this buffer's supported usage flags</summary>
	/// <returns>A BufferUsageFlags structure specifying this buffer's supported usage flags</returns>
	inline BufferUsageFlags getUsageFlags() const { return _createInfo.getUsageFlags(); }

	/// <summary>Get the number of queue families supported by this buffer</summary>
	/// <returns>The size of the list of supported queue family indices</returns>
	inline uint32_t getNumQueueFamilyIndices() const { return _createInfo.getNumQueueFamilyIndices(); }

	/// <summary>Get this buffer's pointer to supported queue families</summary>
	/// <returns>A pointer to a list of supported queue family indices</returns>
	inline const uint32_t* getQueueFamilyIndices() const { return _createInfo.getQueueFamilyIndices(); }

	/// <summary>Call only on Non-sparse buffer.
	/// Binds a non-sparse memory block. This function must be called once
	/// after this buffer creation. Calling second time don't do anything</summary>
	/// <param name="deviceMemory">Device memory block to bind</param>
	/// <param name="offset"> begin offset in the memory block</param>
	void bindMemory(DeviceMemory deviceMemory, VkDeviceSize offset)
	{
		if (isSparseBuffer()) { throw ErrorValidationFailedEXT("Cannot call bindMemory on a sparse buffer"); }

		if (_deviceMemory) { throw ErrorValidationFailedEXT("Cannot bind a memory block as Buffer already has a memory block bound"); }
		_memoryOffset = offset;
		_deviceMemory = deviceMemory;

		vkThrowIfFailed(static_cast<Result>(getDevice()->getVkBindings().vkBindBufferMemory(getDevice()->getVkHandle(), getVkHandle(), _deviceMemory->getVkHandle(), offset)),
			"Failed to bind memory to buffer");
	}

	/// <summary>Get this buffer's create flags</summary>
	/// <returns>BufferCreateInfo</returns>
	BufferCreateInfo getCreateInfo() const { return _createInfo; }

	/// <summary>Return true if this is a sparse buffer</summary>
	/// <returns>bool</returns>
	bool isSparseBuffer() const
	{
		return (_createInfo.getFlags() & (BufferCreateFlags::e_SPARSE_ALIASED_BIT | BufferCreateFlags::e_SPARSE_BINDING_BIT | BufferCreateFlags::e_SPARSE_RESIDENCY_BIT)) != 0;
	}

	/// <summary>Get thus buffer memory requirements</summary>
	/// <returns>VkMemoryRequirements</returns>
	const MemoryRequirements& getMemoryRequirement() const { return _memRequirements; }
};
} // namespace impl

/// <summary>Buffer view creation descriptor.</summary>
struct BufferViewCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	BufferViewCreateInfo() : _format(Format::e_UNDEFINED), _offset(0), _range(VK_WHOLE_SIZE), _flags(BufferViewCreateFlags::e_NONE) {}

	/// <summary>Constructor</summary>
	/// <param name="buffer">The buffer to be used in the buffer view</param>
	/// <param name="format">The format of the data in the buffer</param>
	/// <param name="offset">The buffer offset</param>
	/// <param name="range">The range of the buffer view</param>
	/// <param name="flags">A set of flags used for creating the buffer view</param>
	BufferViewCreateInfo(const Buffer& buffer, Format format, DeviceSize offset = 0, DeviceSize range = VK_WHOLE_SIZE, BufferViewCreateFlags flags = BufferViewCreateFlags::e_NONE)
		: _buffer(buffer), _format(format), _offset(offset), _range(range), _flags(flags)
	{
		assert(range <= buffer->getSize() - offset);
	}

	/// <summary>Get the buffer view creation flags</summary>
	/// <returns>The set of buffer view creation flags</returns>
	inline BufferViewCreateFlags getFlags() const { return _flags; }
	/// <summary>Set the buffer view creation flags</summary>
	/// <param name="flags">The buffer view creation flags</param>
	inline void setFlags(BufferViewCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get Buffer</summary>
	/// <returns>The Buffer used in the buffer view</returns>
	inline const Buffer& getBuffer() const { return _buffer; }
	/// <summary>Set PVRVk Buffer view creation image</summary>
	/// <param name="buffer">Buffer to use for creating a PVRVk buffer view</param>
	inline void setBuffer(const Buffer& buffer) { this->_buffer = buffer; }
	/// <summary>Get Buffer view format</summary>
	/// <returns>Buffer view format (Format)</returns>
	inline Format getFormat() const { return _format; }
	/// <summary>Set the Buffer view format for PVRVk Buffer creation</summary>
	/// <param name="format">The buffer view format to use for creating a PVRVk buffer</param>
	inline void setFormat(Format format) { this->_format = format; }
	/// <summary>Get the buffer view creation offset</summary>
	/// <returns>The set of buffer view creation offset</returns>
	inline DeviceSize getOffset() const { return _offset; }
	/// <summary>Set the buffer view creation offset</summary>
	/// <param name="offset">The buffer view creation offset</param>
	inline void setOffset(DeviceSize offset) { this->_offset = offset; }
	/// <summary>Get the buffer view creation range</summary>
	/// <returns>The set of buffer view creation range</returns>
	inline DeviceSize getRange() const { return _range; }
	/// <summary>Set the buffer view creation range</summary>
	/// <param name="range">The buffer view creation range</param>
	inline void setRange(DeviceSize range) { this->_range = range; }

private:
	/// <summary>The buffer on which the view will be created</summary>
	Buffer _buffer;
	/// <summary>Describes the format of the data elements in the buffer</summary>
	Format _format;
	/// <summary>The offset in bytes from the base address of the buffer</summary>
	DeviceSize _offset;
	/// <summary>The size in bytes of the buffer view</summary>
	DeviceSize _range;
	/// <summary>Flags to use for creating the buffer view</summary>
	BufferViewCreateFlags _flags;
};

namespace impl {
/// <summary>pvrvk implementation of a BufferView.</summary>
class BufferView_ : public PVRVkDeviceObjectBase<VkBufferView, ObjectType::e_BUFFER_VIEW>, public DeviceObjectDebugUtils<BufferView_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class BufferView_;
	};

	static BufferView constructShared(const DeviceWeakPtr& device, const BufferViewCreateInfo& createInfo)
	{
		return std::make_shared<BufferView_>(make_shared_enabler{}, device, createInfo);
	}

	/// <summary>Creation information used when creating the buffer view.</summary>
	BufferViewCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(BufferView_)

	BufferView_(make_shared_enabler, const DeviceWeakPtr& device, const BufferViewCreateInfo& createInfo);

	/// <summary>Destructor. Will properly release all resources held by this object.</summary>
	~BufferView_();
	//!\endcond

	/// <summary>Get the buffer view creation flags</summary>
	/// <returns>The set of buffer view creation flags</returns>
	inline BufferViewCreateFlags getFlags() const { return _createInfo.getFlags(); }
	/// <summary>Get Buffer</summary>
	/// <returns>The Buffer used in the buffer view</returns>
	inline const Buffer& getBuffer() const { return _createInfo.getBuffer(); }
	/// <summary>Get Buffer view format</summary>
	/// <returns>Buffer view format (Format)</returns>
	inline Format getFormat() const { return _createInfo.getFormat(); }
	/// <summary>Get the buffer view creation offset</summary>
	/// <returns>The set of buffer view creation offset</returns>
	inline DeviceSize getOffset() const { return _createInfo.getOffset(); }
	/// <summary>Get the buffer view creation range</summary>
	/// <returns>The set of buffer view creation range</returns>
	inline DeviceSize getRange() const { return _createInfo.getRange(); }
	/// <summary>Get this buffer view's create flags</summary>
	/// <returns>BufferViewCreateInfo</returns>
	const BufferViewCreateInfo& getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
