/*!
\brief Function implementations for the DebugUtilsMessenger class.
\file PVRVk/DebugUtilsMessengerVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DebugUtilsMessengerVk.h"
#include "PVRVk/InstanceVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
DebugUtilsMessenger_::DebugUtilsMessenger_(make_shared_enabler, Instance& instance, const DebugUtilsMessengerCreateInfo& createInfo) : PVRVkInstanceObjectBase(instance)
{
	// Setup callback creation information
	VkDebugUtilsMessengerCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = static_cast<VkDebugUtilsMessengerCreateFlagsEXT>(createInfo.getFlags());
	callbackCreateInfo.messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(createInfo.getMessageSeverity());
	callbackCreateInfo.messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(createInfo.getMessageType());
	callbackCreateInfo.pfnUserCallback = createInfo.getCallback();
	callbackCreateInfo.pUserData = createInfo.getPUserData();

	if (!instance->getVkBindings().vkCreateDebugUtilsMessengerEXT) { throw std::runtime_error("vkCreateDebugUtilsMessengerEXT NULL"); }
	// Register the DebugUtilsMessenger
	vkThrowIfFailed(instance->getVkBindings().vkCreateDebugUtilsMessengerEXT(instance->getVkHandle(), &callbackCreateInfo, nullptr, &_vkHandle), "Failed to create DebugUtilsMessenger");
}

DebugUtilsMessenger_::~DebugUtilsMessenger_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_instance.expired())
		{
			getInstance()->getVkBindings().vkDestroyDebugUtilsMessengerEXT(getInstance()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			assert(false && "Attempted to destroy object of type DebugUtilsMessenger after its corresponding instance");
		}
	}
}
//!\endcond
} // namespace impl
} // namespace pvrvk
