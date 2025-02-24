/*!
\brief Function definitions for Event class
\file PVRVk/EventVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/EventVk.h"
#include "PVRVk/BufferVk.h"
#include "PVRVk/ImageVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
Event_::Event_(make_shared_enabler, const DeviceWeakPtr& device, const EventCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkEventCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_EVENT_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkEventCreateFlags>(createInfo.getFlags());
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateEvent(getDevice()->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "Failed to create Event");
}

Event_::~Event_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyEvent(getDevice()->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}
//!\endcond

void Event_::set() { vkThrowIfFailed(getDevice()->getVkBindings().vkSetEvent(getDevice()->getVkHandle(), getVkHandle()), "Event::set returned an error"); }

void Event_::reset() { vkThrowIfFailed(getDevice()->getVkBindings().vkResetEvent(getDevice()->getVkHandle(), getVkHandle()), "Event::reset returned an error"); }

bool Event_::isSet()
{
	Result res;
	vkThrowIfError(res = static_cast<pvrvk::Result>(getDevice()->getVkBindings().vkGetEventStatus(getDevice()->getVkHandle(), getVkHandle())), "Event::isSet returned an error");
	assert((res == Result::e_EVENT_SET || res == Result::e_EVENT_RESET) && "Event::isSet returned a success code that was neither EVENT_SET or EVENT_RESET");
	return res == Result::e_EVENT_SET;
}
} // namespace impl
} // namespace pvrvk
