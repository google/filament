/*!
\brief Function definitions for the Buffer class.
\file PVRVk/BufferVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRVk/BufferVk.h"
namespace pvrvk {
namespace impl {
Buffer_::~Buffer_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyBuffer(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}

Buffer_::Buffer_(make_shared_enabler, const DeviceWeakPtr& device, const BufferCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	Device deviceSharedPtr = device.lock();

	VkBufferCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_BUFFER_CREATE_INFO);
	vkCreateInfo.size = _createInfo.getSize();
	vkCreateInfo.usage = static_cast<VkBufferUsageFlags>(_createInfo.getUsageFlags());
	vkCreateInfo.flags = static_cast<VkBufferCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.sharingMode = static_cast<VkSharingMode>(_createInfo.getSharingMode());
	vkCreateInfo.pQueueFamilyIndices = _createInfo.getQueueFamilyIndices();
	vkCreateInfo.queueFamilyIndexCount = _createInfo.getNumQueueFamilyIndices();
	vkThrowIfFailed(static_cast<Result>(deviceSharedPtr->getVkBindings().vkCreateBuffer(deviceSharedPtr->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle)), "Failed to create Buffer");

	deviceSharedPtr->getVkBindings().vkGetBufferMemoryRequirements(deviceSharedPtr->getVkHandle(), _vkHandle, &_memRequirements.get());
}

BufferView_::BufferView_(make_shared_enabler, const DeviceWeakPtr& device, const BufferViewCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkBufferViewCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_BUFFER_VIEW_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkBufferViewCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.buffer = _createInfo.getBuffer()->getVkHandle();
	vkCreateInfo.format = static_cast<VkFormat>(_createInfo.getFormat());
	vkCreateInfo.offset = _createInfo.getOffset();
	vkCreateInfo.range = _createInfo.getRange();
	vkThrowIfFailed(static_cast<Result>(getDevice()->getVkBindings().vkCreateBufferView(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle)), "Failed to create BufferView");
}

BufferView_::~BufferView_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyBufferView(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}
} // namespace impl
} // namespace pvrvk
//!\endcond
