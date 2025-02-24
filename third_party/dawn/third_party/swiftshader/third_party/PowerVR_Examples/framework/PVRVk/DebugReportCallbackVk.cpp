/*!
\brief Function implementations for the DebugReportCallback class.
\file PVRVk/DebugReportCallbackVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DebugReportCallbackVk.h"
#include "PVRVk/InstanceVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
DebugReportCallback_::DebugReportCallback_(make_shared_enabler, Instance& instance, const DebugReportCallbackCreateInfo& createInfo) : PVRVkInstanceObjectBase(instance)
{
	// Setup callback creation information
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT);
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = static_cast<VkDebugReportFlagsEXT>(createInfo.getFlags());
	callbackCreateInfo.pfnCallback = createInfo.getCallback();
	callbackCreateInfo.pUserData = createInfo.getPUserData();

	// Register the DebugReportCallback
	vkThrowIfFailed(instance->getVkBindings().vkCreateDebugReportCallbackEXT(instance->getVkHandle(), &callbackCreateInfo, nullptr, &_vkHandle), "Failed to create DebugReportCallback");
}

DebugReportCallback_::~DebugReportCallback_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_instance.expired())
		{
			getInstance()->getVkBindings().vkDestroyDebugReportCallbackEXT(getInstance()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			assert(false && "Attempted to destroy object of type DebugReportCallback after its corresponding instance");
		}
	}
}
//!\endcond
} // namespace impl
} // namespace pvrvk
