/*!
\brief Function definitions for Fence class
\file PVRVk/FenceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/FenceVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/ImageVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
Fence_::Fence_(make_shared_enabler, const DeviceWeakPtr& device, const FenceCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkFenceCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_FENCE_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkFenceCreateFlags>(createInfo.getFlags());
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateFence(getDevice()->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "Failed to create Fence");
}

Fence_::~Fence_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyFence(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}
//!\endcond

bool Fence_::wait(uint64_t timeoutNanos)
{
	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(getDevice()->getVkBindings().vkWaitForFences(getDevice()->getVkHandle(), 1, &getVkHandle(), true, timeoutNanos)),
		"Fence::wait returned an error");
	assert((res == Result::e_SUCCESS || res == Result::e_TIMEOUT || res == Result::e_NOT_READY) && "Fence returned invalid non-error VkResult!");
	return (res == Result::e_SUCCESS);
}

void Fence_::reset() { vkThrowIfFailed(getDevice()->getVkBindings().vkResetFences(getDevice()->getVkHandle(), 1, &getVkHandle()), "Fence::reset returned an error"); }

bool Fence_::isSignalled()
{
	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(getDevice()->getVkBindings().vkGetFenceStatus(getDevice()->getVkHandle(), getVkHandle())), "Fence::getStatus returned an error");
	assert((res == Result::e_SUCCESS || res == Result::e_TIMEOUT || res == Result::e_NOT_READY) && "Fence returned invalid non-error VkResult!");
	return (res == Result::e_SUCCESS);
}
} // namespace impl
} // namespace pvrvk
