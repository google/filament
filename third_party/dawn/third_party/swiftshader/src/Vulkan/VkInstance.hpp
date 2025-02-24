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

#ifndef VK_INSTANCE_HPP_
#define VK_INSTANCE_HPP_

#include "VkObject.hpp"

namespace vk {

class DebugUtilsMessenger;

class Instance
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE; }

	Instance(const VkInstanceCreateInfo *pCreateInfo, void *mem, VkPhysicalDevice physicalDevice, DebugUtilsMessenger *messenger);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkInstanceCreateInfo *) { return 0; }

	VkResult getPhysicalDevices(uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices) const;
	VkResult getPhysicalDeviceGroups(uint32_t *pPhysicalDeviceGroupCount,
	                                 VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties) const;

	void submitDebugUtilsMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

private:
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	// This debug messenger (when available) is owned by the instance and created via passing a
	// chained struct to vkCreateInstance. This should be destroyed with the instance.
	DebugUtilsMessenger *messenger = nullptr;
};

using DispatchableInstance = DispatchableObject<Instance, VkInstance>;

static inline Instance *Cast(VkInstance object)
{
	return DispatchableInstance::Cast(object);
}

}  // namespace vk

#endif  // VK_INSTANCE_HPP_
