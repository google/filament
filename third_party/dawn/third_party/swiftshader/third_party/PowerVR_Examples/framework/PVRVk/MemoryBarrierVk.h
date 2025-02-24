/*!
\brief PVRVk Semaphore class.
\file PVRVk/SemaphoreVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
#if defined(_WIN32)
#undef MemoryBarrier
#endif
/// <summary>A Global memory barrier used for memory accesses for all memory objects.</summary>
struct MemoryBarrier
{
private:
	pvrvk::AccessFlags srcAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a source access mask.
	pvrvk::AccessFlags dstAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.

public:
	/// <summary>Constructor, zero initialization</summary>
	MemoryBarrier() : srcAccessMask(pvrvk::AccessFlags(0)), dstAccessMask(pvrvk::AccessFlags(0)) {}

	/// <summary>Constructor, setting all members</summary>
	/// <param name="srcAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	MemoryBarrier(pvrvk::AccessFlags srcAccessMask, pvrvk::AccessFlags dstAccessMask) : srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask) {}

	/// <summary>Get srcAccessMask</summary>
	/// <returns>An AccessFlags structure specifying the source memory barrier access flags</returns>
	inline const AccessFlags& getSrcAccessMask() const { return srcAccessMask; }

	/// <summary>Set srcAccessMask</summary>
	/// <param name="inSrcAccessMask">An AccessFlags structure specifying the source memory barrier access flags</param>
	inline void setSrcAccessMask(const AccessFlags& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

	/// <summary>Get dstAccessMask</summary>
	/// <returns>An AccessFlags structure specifying the destination memory barrier access flags</returns>
	inline const AccessFlags& getDstAccessMask() const { return dstAccessMask; }

	/// <summary>Set dstAccessMask</summary>
	/// <param name="inDstAccessMask">An AccessFlags structure specifying the destination memory barrier access flags</param>
	inline void setDstAccessMask(const AccessFlags& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }
};

/// <summary>A Buffer memory barrier used only for memory accesses involving a specific range of the specified
/// buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.</summary>
struct BufferMemoryBarrier
{
private:
	pvrvk::AccessFlags srcAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a source access mask.
	pvrvk::AccessFlags dstAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.
	Buffer buffer; //!< Handle to the buffer whose backing memory is affected by the barrier.
	uint32_t offset; //!< Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer
	uint32_t size; //!< Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.

public:
	/// <summary>Constructor, zero initialization</summary>
	BufferMemoryBarrier() : srcAccessMask(pvrvk::AccessFlags(0)), dstAccessMask(pvrvk::AccessFlags(0)) {}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="buffer">Handle to the buffer whose backing memory is affected by the barrier.</param>
	/// <param name="offset">Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer</param>
	/// <param name="size">Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.</param>
	BufferMemoryBarrier(pvrvk::AccessFlags srcAccessMask, pvrvk::AccessFlags dstAccessMask, Buffer buffer, uint32_t offset, uint32_t size)
		: srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask), buffer(buffer), offset(offset), size(size)
	{}

	/// <summary>Get srcAccessMask</summary>
	/// <returns>An AccessFlags structure specifying the source memory barrier access flags</returns>
	inline const AccessFlags& getSrcAccessMask() const { return srcAccessMask; }

	/// <summary>Set srcAccessMask</summary>
	/// <param name="inSrcAccessMask">An AccessFlags structure specifying the source memory barrier access flags</param>
	inline void setSrcAccessMask(const AccessFlags& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

	/// <summary>Get dstAccessMask</summary>
	/// <returns>An AccessFlags structure specifying the destination memory barrier access flags</returns>
	inline const AccessFlags& getDstAccessMask() const { return dstAccessMask; }

	/// <summary>Set dstAccessMask</summary>
	/// <param name="inDstAccessMask">An AccessFlags structure specifying the destination memory barrier access flags</param>
	inline void setDstAccessMask(const AccessFlags& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }

	/// <summary>Get Buffer associated with the memory barrier</summary>
	/// <returns>The PVRVk::Buffer associated with the memory barrier</returns>
	inline const Buffer& getBuffer() const { return buffer; }

	/// <summary>Set buffer associated with the memory barrier</summary>
	/// <param name="inBuffer">The PVRVk::Buffer associated with the memory barrier</param>
	inline void setBuffer(const Buffer& inBuffer) { this->buffer = inBuffer; }

	/// <summary>Get size corresponding to the slice of the Buffer associated with the memory barrier</summary>
	/// <returns>The size of the range of the PVRVk::Buffer associated with the memory barrier</returns>
	inline uint32_t getSize() const { return size; }

	/// <summary>Set the size of the slice of the buffer associated with the memory barrier</summary>
	/// <param name="inSize">The size of the slice of the PVRVk::Buffer associated with the memory barrier</param>
	inline void setSize(uint32_t inSize) { this->size = inSize; }

	/// <summary>Get the offset into the Buffer associated with the memory barrier</summary>
	/// <returns>The offset into PVRVk::Buffer associated with the memory barrier</returns>
	inline uint32_t getOffset() const { return offset; }

	/// <summary>Set the offset into the buffer associated with the memory barrier</summary>
	/// <param name="inOffset">The offset into the PVRVk::Buffer associated with the memory barrier</param>
	inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
};

/// <summary>A Image memory barrier used only for memory accesses involving a specific subresource range of the
/// specified image object. It is also used to perform a layout transition for an image subresource range, or to
/// transfer ownership of an image subresource range from one queue family to another.</summary>
struct ImageMemoryBarrier
{
private:
	pvrvk::AccessFlags srcAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a source access mask.
	pvrvk::AccessFlags dstAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.
	pvrvk::ImageLayout oldLayout; //!< Old layout in an image layout transition.
	pvrvk::ImageLayout newLayout; //!< New layout in an image layout transition.
	uint32_t srcQueueFamilyIndex; //!< Source queue family for a queue family ownership transfer.
	uint32_t dstQueueFamilyIndex; //!< Destination queue family for a queue family ownership transfer
	Image image; //!< Handle to the image affected by this barrier
	ImageSubresourceRange subresourceRange; //!< Describes the image subresource range within image that is affected by this barrier

public:
	/// <summary>Constructor. All flags are zero initialized, and family indexes set to -1.</summary>
	ImageMemoryBarrier()
		: srcAccessMask(pvrvk::AccessFlags(0)), dstAccessMask(pvrvk::AccessFlags(0)), oldLayout(pvrvk::ImageLayout::e_UNDEFINED), newLayout(pvrvk::ImageLayout::e_UNDEFINED),
		  srcQueueFamilyIndex(static_cast<uint32_t>(-1)), dstQueueFamilyIndex(static_cast<uint32_t>(-1))
	{}

	/// <summary>Constructor. Set all individual elements.</summary>
	/// <param name="srcMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="image">Handle to the image affected by this barrier</param>
	/// <param name="subresourceRange">Describes the image subresource range within image that is affected by this barrier</param>
	/// <param name="oldLayout">Old layout in an image layout transition.</param>
	/// <param name="newLayout">New layout in an image layout transition.</param>
	/// <param name="srcQueueFamilyIndex">Source queue family for a queue family ownership transfer.</param>
	/// <param name="dstQueueFamilyIndex">Destination queue family for a queue family ownership transfer</param>
	ImageMemoryBarrier(pvrvk::AccessFlags srcMask, pvrvk::AccessFlags dstMask, const Image& image, const ImageSubresourceRange& subresourceRange, pvrvk::ImageLayout oldLayout,
		pvrvk::ImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
		: srcAccessMask(srcMask), dstAccessMask(dstMask), oldLayout(oldLayout), newLayout(newLayout), srcQueueFamilyIndex(srcQueueFamilyIndex),
		  dstQueueFamilyIndex(dstQueueFamilyIndex), image(image), subresourceRange(subresourceRange)
	{}

	/// <summary>Get srcAccessMask</summary>
	/// <returns>AcceAn AccessFlags structure specifying the source memory barrier access flagsssFlags</returns>
	inline const AccessFlags& getSrcAccessMask() const { return srcAccessMask; }

	/// <summary>Set srcAccessMask</summary>
	/// <param name="inSrcAccessMask">An AccessFlags structure specifying the source memory barrier access flags</param>
	inline void setSrcAccessMask(const AccessFlags& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

	/// <summary>Get dstAccessMask</summary>
	/// <returns>An AccessFlags structure specifying the destination memory barrier access flags</returns>
	inline const AccessFlags& getDstAccessMask() const { return dstAccessMask; }

	/// <summary>Set dstAccessMask</summary>
	/// <param name="inDstAccessMask">An AccessFlags structure specifying the destination memory barrier access flags</param>
	inline void setDstAccessMask(const AccessFlags& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }

	/// <summary>Get the old image layout of the image associated with the memory barrier</summary>
	/// <returns>The old image layout of the image associated with the memory barrier</returns>
	inline const ImageLayout& getOldLayout() const { return oldLayout; }
	/// <summary>Set old image layout</summary>
	/// <param name="inOldLayout">The old image layout of the image associated memory barrier</param>
	inline void setOldLayout(const ImageLayout& inOldLayout) { this->oldLayout = inOldLayout; }

	/// <summary>Get the new image layout of the image associated with the memory barrier</summary>
	/// <returns>The new image layout of the image associated with the memory barrier</returns>
	inline const ImageLayout& getNewLayout() const { return newLayout; }

	/// <summary>Set new image layout</summary>
	/// <param name="inNewLayout">The new image layout of the image associated memory barrier</param>
	inline void setNewLayout(const ImageLayout& inNewLayout) { this->newLayout = inNewLayout; }

	/// <summary>Get the source queue family index for the image associated with the memory barrier</summary>
	/// <returns>The source queue family index of the image associated with the memory barrier</returns>
	inline uint32_t getSrcQueueFamilyIndex() const { return srcQueueFamilyIndex; }

	/// <summary>Set the source queue family index</summary>
	/// <param name="inSrcQueueFamilyIndex">The source queue family index of the image associated with the memory barrier</param>
	inline void setSrcQueueFamilyIndex(uint32_t inSrcQueueFamilyIndex) { this->srcQueueFamilyIndex = inSrcQueueFamilyIndex; }

	/// <summary>Get the destination queue family index for the image associated with the memory barrier</summary>
	/// <returns>The destination queue family index of the image associated with the memory barrier</returns>
	inline uint32_t getDstQueueFamilyIndex() const { return dstQueueFamilyIndex; }

	/// <summary>Set the destination queue family index</summary>
	/// <param name="inDstQueueFamilyIndex">The destination queue family index of the image associated with the memory barrier</param>
	inline void setDstQueueFamilyIndex(uint32_t inDstQueueFamilyIndex) { this->dstQueueFamilyIndex = inDstQueueFamilyIndex; }

	/// <summary>Get Image</summary>
	/// <returns>The PVRVk::Image associated with the memory barrier</returns>
	inline const Image& getImage() const { return image; }

	/// <summary>Set Image</summary>
	/// <param name="inImage">The PVRVk::Image associated with the memory barrier</param>
	inline void setImage(const Image& inImage) { this->image = inImage; }

	/// <summary>Get the subresource range of the image associated with the memory barrier</summary>
	/// <returns>The subresource range of the image associated with the memory barrier</returns>
	inline const ImageSubresourceRange& getSubresourceRange() const { return subresourceRange; }

	/// <summary>Set the subresource range of the image associated with the memory barrier</summary>
	/// <param name="inSubresourceRange">The subresource range of the image associated with the memory barrier</param>
	inline void setSubresourceRange(const ImageSubresourceRange& inSubresourceRange) { this->subresourceRange = inSubresourceRange; }
};

/// <summary>A memory barrier into the command stream. Used to signify that some types of pending operations
/// from before the barrier must have finished before the commands after the barrier start executing.</summary>
struct MemoryBarrierSet
{
private:
	MemoryBarrierSet(const MemoryBarrierSet&) = delete; // deleted
	MemoryBarrierSet& operator=(const MemoryBarrierSet&) = delete; // deleted

	std::vector<MemoryBarrier> memBarriers;
	std::vector<ImageMemoryBarrier> imageBarriers;
	std::vector<BufferMemoryBarrier> bufferBarriers;

public:
	/// <summary>Constructor. Empty barrier</summary>
	MemoryBarrierSet() = default;

	/// <summary>Clear this object of all barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllBarriers()
	{
		memBarriers.clear();
		imageBarriers.clear();
		bufferBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Memory barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllMemoryBarriers()
	{
		memBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Buffer barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllBufferRangeBarriers()
	{
		bufferBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Image barriers</summary>
	/// <returns>MemoryBarrierSet&</returns>
	MemoryBarrierSet& clearAllImageAreaBarriers()
	{
		imageBarriers.clear();
		return *this;
	}

	/// <summary>Add a generic Memory barrier.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(MemoryBarrier barrier)
	{
		memBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of a buffer
	/// must complete before other operations on that part of the buffer execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const BufferMemoryBarrier& barrier)
	{
		bufferBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
	/// must complete before other operations on that part of the Image execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSet& addBarrier(const ImageMemoryBarrier& barrier)
	{
		imageBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Get an array of the MemoryBarrier object of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const std::vector<MemoryBarrier>& getMemoryBarriers() const { return this->memBarriers; }

	/// <summary>Get an array of the Image Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const std::vector<ImageMemoryBarrier>& getImageBarriers() const { return this->imageBarriers; }

	/// <summary>Get an array of the Buffer Barriers of this set.</summary>
	/// <returns>All MemoryBarrier objects that this object contains</returns>
	const std::vector<BufferMemoryBarrier>& getBufferBarriers() const { return this->bufferBarriers; }
};
} // namespace pvrvk
