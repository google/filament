/*!
\brief Function implementations of the Vulkan Device class.
\file PVRVk/DeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/DisplayVk.h"
#include "PVRVk/DisplayModeVk.h"

namespace pvrvk {
namespace impl {
Display_::Display_(make_shared_enabler, const PhysicalDeviceWeakPtr& physicalDevice, const DisplayPropertiesKHR& displayProperties)
	: PVRVkPhysicalDeviceObjectBase(physicalDevice, displayProperties.getDisplay()), _properties(displayProperties)
{
	const Instance& instanceSharedPtr = getPhysicalDevice()->getInstance();
	if (instanceSharedPtr->getEnabledExtensionTable().khrDisplayEnabled)
	{
		uint32_t numModes = 0;
		instanceSharedPtr->getVkBindings().vkGetDisplayModePropertiesKHR(getPhysicalDevice()->getVkHandle(), getVkHandle(), &numModes, NULL);
		pvrvk::ArrayOrVector<VkDisplayModePropertiesKHR, 4> displayModePropertiesVk(numModes);
		instanceSharedPtr->getVkBindings().vkGetDisplayModePropertiesKHR(getPhysicalDevice()->getVkHandle(), getVkHandle(), &numModes, displayModePropertiesVk.get());

		for (uint32_t i = 0; i < numModes; ++i)
		{
			DisplayModePropertiesKHR displayModeProperties = displayModePropertiesVk[i];
			_displayModes.emplace_back(DisplayMode_::constructShared(physicalDevice, displayModeProperties));
		}
	}
	else
	{
		throw ErrorUnknown("Display Extension must be enabled when creating the VkInstance.");
	}
}
} // namespace impl
} // namespace pvrvk

//!\endcond
