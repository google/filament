// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkInstance.hpp"
#include "VkDebugUtilsMessenger.hpp"
#include "VkDestroy.hpp"

namespace vk {

Instance::Instance(const VkInstanceCreateInfo *pCreateInfo, void *mem, VkPhysicalDevice physicalDevice, DebugUtilsMessenger *messenger)
    : physicalDevice(physicalDevice)
    , messenger(messenger)
{
}

void Instance::destroy(const VkAllocationCallbacks *pAllocator)
{
	if(messenger)
	{
		vk::destroy(static_cast<VkDebugUtilsMessengerEXT>(*messenger), pAllocator);
	}
	vk::destroy(physicalDevice, pAllocator);
}

VkResult Instance::getPhysicalDevices(uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices) const
{
	if(!pPhysicalDevices)
	{
		*pPhysicalDeviceCount = 1;
		return VK_SUCCESS;
	}

	if(*pPhysicalDeviceCount < 1)
	{
		return VK_INCOMPLETE;
	}

	pPhysicalDevices[0] = physicalDevice;
	*pPhysicalDeviceCount = 1;

	return VK_SUCCESS;
}

VkResult Instance::getPhysicalDeviceGroups(uint32_t *pPhysicalDeviceGroupCount,
                                           VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties) const
{
	if(!pPhysicalDeviceGroupProperties)
	{
		*pPhysicalDeviceGroupCount = 1;
		return VK_SUCCESS;
	}

	if(*pPhysicalDeviceGroupCount < 1)
	{
		return VK_INCOMPLETE;
	}

	pPhysicalDeviceGroupProperties[0].physicalDeviceCount = 1;
	pPhysicalDeviceGroupProperties[0].physicalDevices[0] = physicalDevice;
	pPhysicalDeviceGroupProperties[0].subsetAllocation = VK_FALSE;
	*pPhysicalDeviceGroupCount = 1;

	return VK_SUCCESS;
}

void Instance::submitDebugUtilsMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
	if(messenger)
	{
		messenger->submitMessage(messageSeverity, messageTypes, pCallbackData);
	}
}

}  // namespace vk
